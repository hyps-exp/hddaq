#include<fstream>
#include<cstdlib>
#include<iostream>
#include<cstdio>
#include<csignal>
#include<list>
#include<unistd.h>

#include"UDPRBCP.hh"
#include"FPGAModule.hh"

#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<string.h>
#include<errno.h>

#include"DaqFuncs.hh"

namespace HUL::DAQ{

  int user_stop = 0;

  // signal -----------------------------------------------------------------
  void
  UserStop_FromCtrlC(int signal)
  {
    std::cout << "#D: Stop request" << std::endl;
    user_stop = 1;
  }

  // Function for trigger-type DAQ ------------------------------------------
  namespace TRG{
    static const int32_t kNumHead      {3};
    static const int32_t kNumBodyMax   {8192+65536}; // to read ADC data (32 ch * 2048 max windowsize)
    static const int32_t kNumData      {kNumHead + kNumBodyMax};
    static const int32_t kNumByte      {4};
  };
  
  void
  DoTrgDaq(std::string ip, int32_t runno, int32_t event_num, uint32_t daq_gate_address)
  {
    static const int32_t kPrintStep    {1000};

    (void) signal(SIGINT, UserStop_FromCtrlC);

    // TCP socket
    int sock;
    if(-1 == (sock = ConnectSocket(ip))) return;
    std::cout << "#D: Socket connected" << std::endl;

    // Start DAQ
    std::string filename = "data/run" + std::to_string(runno) + ".dat";
    std::ofstream ofs(filename.c_str(), std::ios::binary);
    if(!ofs.is_open()){
      std::cerr << "#E: Data file cannot be created.\n"
		<< "    Does a 'data' directory exist in the current directory?"
		<< std::endl;
      close(sock);
      std::exit(-1);
    }

    // DAQ gate ON
    RBCP::UDPRBCP udp_rbcp(ip, RBCP::gUdpPort, RBCP::DebugMode::kNoDisp);
    HUL::FPGAModule fpga_module(udp_rbcp);
    fpga_module.WriteModule(daq_gate_address, 1);
    
    std::cout << "#D: Open the DAQ gate and starts data aquisition." << std::endl;  

    // DAQ Cycle
    uint32_t buf[TRG::kNumData];
    int32_t n = 0;
    for( ; n<event_num; ++n){
      int32_t ret_receive;
      while( kRecvTimeOut == ( ret_receive = DoEventCycle(sock, buf)) && !user_stop) continue;
      if(user_stop) break;

      if(ret_receive == kRecvZero){
	std::cout << "#E: Recv() returns 0" << std::endl;
	break;
      }

      if(ret_receive == kRecvError){
	std::cout << "#E: Recv() returns -1" << std::endl;
	break;
      }
      
      int32_t n_word = ret_receive;
      ofs.write((char*)buf, n_word*TRG::kNumByte);

      if(n%kPrintStep==0){
	printf("\033[2J");
	printf("Event %d\n", n);
	for(int32_t i = 0; i<n_word; ++i){
	  printf("%8x ", buf[i]);
	  if((i+1)%8 == 0) printf("\n");
	}// for(i)

	printf("\n");
      }
    }// For(n)

    std::cout << "#D: " << n << " events were accumulated." << std::endl;
    std::cout << "#D: Close the DAQ gate." << std::endl;  

    fpga_module.WriteModule(daq_gate_address,  0);

    std::cout << "#D: Read remaining data in FPGA. Wait time out." << std::endl;    
    sleep(1);
    while( kRecvTimeOut != DoEventCycle(sock, buf));
    std::cout << "#D: Time out is detected. End of DAQ." << std::endl;  

    //  shutdown(sock, SHUT_RDWR);
    close(sock);
    ofs.close();

    return;
  }

  
  // Function for streaming DAQ ---------------------------------------------
  namespace STR{
    static const int32_t kNumData   {10000};
    static const int32_t kNumByte   {8};
  };
  
  void
  DoStrDaq(std::string ip, int32_t runno)
  {

    struct DataCont{
      uint8_t data[STR::kNumData*STR::kNumByte];
      uint32_t recv_bytes;
    };
    
    (void) signal(SIGINT, UserStop_FromCtrlC);

    // TCP socket
    int sock;
    if(-1 == (sock = ConnectSocket(ip))) return;
    std::cout << "#D: Socket connected" << std::endl;

    // Start DAQ
    std::string filename = "data/run" + std::to_string(runno) + ".dat";
    std::ofstream ofs(filename.c_str(), std::ios::binary);
    if(!ofs.is_open()){
      std::cerr << "#E: Data file cannot be created.\n"
		<< "    Does a 'data' directory exist in the current directory?"
		<< std::endl;
      close(sock);
      return;
    }

    std::cout << "#D: Start DAQ" << std::endl;
    // DAQ Cycle
    std::list<DataCont> data_list;
    static const uint32_t kReadSize = sizeof(uint8_t)*STR::kNumByte*STR::kNumData;
    for(;;){
      DataCont dcont = {{0},0};
      int32_t recv_status = 0;

      while( kRecvTimeOut == ( recv_status = Receive(sock, dcont.data, kReadSize, dcont.recv_bytes)) && !user_stop) continue;

      if(recv_status == kRecvZero){
	std::cout << "#E: Recv() returns 0" << std::endl;
	break;
      }

      if(recv_status == kRecvError){
	std::cout << "#E: Recv() returns -1" << std::endl;
	break;
      }

      printf("Recording...");
      data_list.push_back(dcont);
    
      if(user_stop == 1) break;
    }// For()

    std::list<DataCont>::iterator itr     = data_list.begin();
    std::list<DataCont>::iterator itr_end = data_list.end();
    for(; itr != itr_end; ++itr){
      //    std::cout << "N:" << (*itr).n_word << std::endl;
      ofs.write((char*)(*itr).data, (*itr).recv_bytes);
    }

    std::cout << "#D: Read remaining data in FPGA. Wait time out." << std::endl;
  
    sleep(1);
    DataCont dcont = {{0},0};
    while(kRecvTimeOut != Receive(sock, dcont.data, kReadSize,dcont.recv_bytes));
  
    std::cout << "#D: Time out is detected. End of DAQ." << std::endl;

    close(sock);
    ofs.close();
  
    return;
  }

  // EventCycle -------------------------------------------------------------
  int32_t
  DoEventCycle(int sock, uint32_t* buffer)
  {
    // data read ---------------------------------------------------------
    static const uint32_t kSizeHeader = TRG::kNumHead*TRG::kNumByte;
  
    uint32_t received_bytes = 0;
    int recv_status = Receive(sock, (uint8_t*)buffer, kSizeHeader, received_bytes);
    if(recv_status <= 0) return recv_status;

    uint32_t n_word_data  = buffer[1] & 0x3ffff;
    uint32_t size_data    = n_word_data*TRG::kNumByte;
  
    if(n_word_data == 0) return TRG::kNumHead;

    recv_status = Receive(sock, (uint8_t*)(buffer + TRG::kNumHead), size_data, received_bytes);
    if(recv_status <= 0) return recv_status;
  
    return TRG::kNumHead + n_word_data;
  }
  
  // ConnectSocket ----------------------------------------------------------
  int
  ConnectSocket(std::string ip)
  {
    struct sockaddr_in SiTCP_ADDR;
    unsigned int port = 24;

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SiTCP_ADDR.sin_family      = AF_INET;
    SiTCP_ADDR.sin_port        = htons((unsigned short int) port);
    SiTCP_ADDR.sin_addr.s_addr = inet_addr(ip.c_str());

    struct timeval tv;
    tv.tv_sec  = 3;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    if(0 > connect(sock, (struct sockaddr*)&SiTCP_ADDR, sizeof(SiTCP_ADDR))){
      std::cerr << "#E : TCP connection error" << std::endl;
      close(sock);
      return -1;
    }
  
    return sock;
  }

  // Socket read ----------------------------------------------------------------
  int32_t
  Receive(int sock, uint8_t* data_buf, uint32_t length, uint32_t& num_received_bytes)
  {
    uint32_t revd_size = 0;
    int      tmp_ret   = 0;
    int      ret_val   = HUL::DAQ::kRecvNormal;

    while(revd_size < length){
      tmp_ret = recv(sock, (char*)data_buf + revd_size, length -revd_size, 0);

      if(tmp_ret == 0){
	num_received_bytes = 0;
	ret_val            = HUL::DAQ::kRecvZero;
	return ret_val;
      }
    
      if(tmp_ret < 0){
	int errbuf = errno;
	perror("#D: TCP receive");
	if(errbuf == EAGAIN){
	  // socket read time out
	  std::cout << " recv() time out, received byte size: " << revd_size << std::endl;
	  num_received_bytes = static_cast<uint32_t>(revd_size);
	  ret_val            = HUL::DAQ::kRecvTimeOut;
	}else{
	  // something wrong
	  std::cerr << "#E: TCP error : " << errbuf << std::endl;

	  num_received_bytes = 0;
	  ret_val            = HUL::DAQ::kRecvError;
	}

	return ret_val;
      }

      revd_size += tmp_ret;
    }

    num_received_bytes = static_cast<uint32_t>(revd_size);
    return ret_val;
  }

};
