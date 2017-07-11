
#include "userdevice.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "FPGAModule.hh"
#include "RegisterMap.hh"
#include "UDPRBCP.hh"
#include "errno.h"
#include "network.hh"
#include "rbcp.h"

namespace
{
  using namespace HUL_MST;
  //maximum datasize by byte unit
  static const int n_word = 41; // header 3 + body 1(RM) + 32(TOF) + 4(CH) + 1(TAG)
  static const int max_data_size = 4*n_word;
  DaqMode g_daq_mode = DM_NORMAL;

  static const int NofHead = 3;

  char ip[100];
  int  sock=0;
  rbcp_header rbcpHeader;

  //______________________________________________________________________________
  // local function
  //______________________________________________________________________________
  int
  ConnectSocket( char *ip )
  {
    struct sockaddr_in SiTCP_ADDR;
    unsigned int port = tcp_port;

    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    SiTCP_ADDR.sin_family      = AF_INET;
    SiTCP_ADDR.sin_port        = htons((unsigned short int)port);
    SiTCP_ADDR.sin_addr.s_addr = inet_addr(ip);

    struct timeval tv;
    tv.tv_sec  = 3;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    // Connection
    if(0 > connect(sock, (struct sockaddr*)&SiTCP_ADDR, sizeof(SiTCP_ADDR))){
      close(sock);
      return -1;
    }

    std::cout << "socket connected" << std::endl;

    return sock;
  }

  //______________________________________________________________________________
  int
  receive( int sock, char* data_buf, unsigned int ReadLength )
  {
    unsigned int revd_size = 0;
    int tmp_returnVal      = 0;

    while( revd_size < ReadLength ){
      tmp_returnVal = recv(sock, data_buf +revd_size, ReadLength -revd_size, 0);
      if(tmp_returnVal == 0){break;}
      if(tmp_returnVal < 0){
	int errbuf = errno;
	std::cerr << "TCP receive" << std::endl;
	if(errbuf == EAGAIN){
	  // time out
	}else{
	  // something wrong
	  std::ostringstream oss;
	  oss << "::" << __func__ << "() " << ip
	      << " TCP receive error " << errbuf;
	  send_error_message( oss.str() );
	}

	revd_size = tmp_returnVal;
	break;
      }
      revd_size += tmp_returnVal;
    }

    return revd_size;
  }

  //______________________________________________________________________________
  int
  EventCycle( int socket, unsigned int* event_buffer )
  {
    // data read ---------------------------------------------------------
    static const unsigned int sizeHeader = NofHead*sizeof(unsigned int);
    int ret = receive(sock, (char*)event_buffer, sizeHeader);
    if(ret < 0) return -1;

    unsigned int n_word_data  = event_buffer[1] & 0x3ff;
    unsigned int sizeData     = n_word_data*sizeof(unsigned int);
    
  
    //if(n_word_data == 0) return NofHead;
    if(n_word_data == 0) return sizeHeader;

    ret = receive(sock, (char*)(event_buffer + NofHead), sizeData);
    if(ret < 0) return -1;

    for (int i=0; i<NofHead+n_word_data; i++)
      printf("%d : %x\n", i, event_buffer[i]);

    //return NofHead+ n_word_data;
    return sizeHeader + sizeData;

  }

}

//______________________________________________________________________________
int
get_maxdatasize( void )
{
  return max_data_size;
}

//______________________________________________________________________________
void
open_device( NodeProp& nodeprop )
{
  const std::string& nick_name(nodeprop.getNickName());
  const std::string& func_name(nick_name+" [::"+__func__+"()]");

  // RBCP
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  // load parameters
  const int argc = nodeprop.getArgc();
  for( int i=0; i<argc; ++i ){
    std::string arg = nodeprop.getArgv(i);
    std::istringstream iss;
    if( arg.substr(0,11) == "--sitcp-ip=" ){
      iss.str( arg.substr(11) );
      iss >> ip;
    }
  }

  FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
  fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_FERAMode);
  fModule.WriteModule(IOM::mid, IOM::laddr_nimout2, IOM::reg_o_FERAMode);
  fModule.WriteModule(IOM::mid, IOM::laddr_nimout3, IOM::reg_o_FERAMode);
  fModule.WriteModule(IOM::mid, IOM::laddr_nimout4, IOM::reg_o_ModuleBusy);


  //Connection check -----------------------------------------------
  while(0 > (sock = ConnectSocket(ip) )){
    std::ostringstream oss;
    oss << func_name << " Connection fail : " << ip;
    send_error_message( oss.str() );
    std::cerr << oss.str() << std::endl;
  }

  close(sock);

  return;
}

//______________________________________________________________________________
void
init_device( NodeProp& nodeprop )
{
  const std::string& nick_name(nodeprop.getNickName());
  const std::string& func_name(nick_name+" [::"+__func__+"()]");

  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  //  event_num = 0;

  switch(g_daq_mode){
  case DM_NORMAL:
    {
      while(0 > (sock = ConnectSocket(ip) )){
	std::ostringstream oss;
	oss << func_name << " Connection fail : " << ip;
	send_error_message( oss.str() );
	std::cerr << oss.str() << std::endl;
      }

      {
	std::ostringstream oss;
	oss << func_name << " Connection done : " << ip;
	send_normal_message( oss.str() );
      }

      // Start DAQ
      FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
      {
	std::ostringstream oss;
	oss << func_name << " Firmware : " << std::hex << std::showbase
	    << fModule.ReadModule( BCT::mid, BCT::laddr_Version, 4 );
	send_normal_message( oss.str() );
      }

      //fModule.WriteModule(BCT::mid, BCT::laddr_Reset,  1);
      fModule.WriteModule(DCT::mid, DCT::laddr_evb_reset, 1);
      ::sleep(1);
      unsigned int sel_trig = TRM::reg_EnRM | TRM::reg_ClrRM | 
	                      TRM::reg_L2RM | TRM::reg_EnL2 | 
	                      TRM::reg_L1RM;

      fModule.WriteModule(TRM::mid, TRM::laddr_sel_trig,  sel_trig);

      fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_FERAMode);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout2, IOM::reg_o_FERAMode);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout3, IOM::reg_o_FERAMode);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout4, IOM::reg_o_ModuleBusy);

      // start DAQ
      fModule.WriteModule(DCT::mid, DCT::laddr_gate, 1);

      std::cout << "init_device" << std::endl;
      return;
    }
  case DM_DUMMY:
    {
      return;
    }
  default:
    return;
  }

}

//______________________________________________________________________________
void
finalize_device( NodeProp& nodeprop )
{
  // const std::string& nick_name(nodeprop.getNickName());
  // const std::string& func_name(nick_name+" [::"+__func__+"()]");

  FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
  fModule.WriteModule(DCT::mid, DCT::laddr_gate, 0);
  ::sleep(1);
  unsigned int data[n_word];
  while(-1 != EventCycle(sock, data));

  shutdown(sock, SHUT_RDWR);
  close(sock);

  return;
}

//______________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{
  // const std::string& nick_name(nodeprop.getNickName());
  // const std::string& func_name(nick_name+" [::"+__func__+"()]");
  return;
}

//______________________________________________________________________________
int
wait_device( NodeProp& nodeprop )
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  // const std::string& nick_name(nodeprop.getNickName());
  // const std::string& func_name(nick_name+" [::"+__func__+"()]");
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      return 0;
    }
  case DM_DUMMY:
    {
      usleep(200000);
      return 0;
    }
  default:
    return 0;
  }

}

//______________________________________________________________________________
int
read_device( NodeProp& nodeprop, unsigned int* data, int& len )
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  const std::string& nick_name(nodeprop.getNickName());
  const std::string& func_name(nick_name+" [::"+__func__+"()]");
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int val = EventCycle(sock, data);
      if (val == -1)
	len = -1;
      else
	len = val/sizeof(unsigned int);

      // if( len > 0 ){
      // 	for(int i = 0; i<n_word; ++i){
      // 	  printf("%x ", data[i]);
      // 	  if(i%8==0) printf("\n");
      // 	}
      // }
      return len;
    }
  case DM_DUMMY:
    {
      len = 0;
      return 0;
    }
  default:
    len = 0;
    return 0;
  }

}
