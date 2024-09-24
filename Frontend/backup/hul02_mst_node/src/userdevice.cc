#include "userdevice.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "FPGAModule.hh"
#include "RegisterMap.hh"
#include "UDPRBCP.hh"
#include "mif_func.hh"
#include "errno.h"
#include "network.hh"
#include "rbcp.h"

bool    stand_alone = false;
DaqMode g_daq_mode  = DM_NORMAL;
std::string nick_name;
#define DEBUG 0

namespace
{
  using namespace HUL_MsT;
  //maximum datasize by byte unit
  static const int n_header      = 3;
  static const int max_n_word    = n_header + 16*64 + 16*32 + 3 + 1;
  static const int max_data_size = sizeof(unsigned int)*max_n_word;

  char ip[100];
  unsigned int min_time_window;
  unsigned int max_time_window;
  unsigned int prescale_value;
  unsigned int timer_value;
  unsigned int bypass;
  bool flag_master = false;;
  
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
    static const std::string& func_name(nick_name+" [::"+__func__+"()]");

    // data read ---------------------------------------------------------
    static const unsigned int sizeHeader = n_header*sizeof(unsigned int);
    int ret = receive(sock, (char*)event_buffer, sizeHeader);
    if(ret < 0) return -1;

    unsigned int n_word_data  = event_buffer[1] & 0xfff;
    unsigned int sizeData     = n_word_data*sizeof(unsigned int);

    if(event_buffer[0] != 0xffff20d1){
      std::ostringstream oss;
      oss << func_name << " Data broken : " << ip;
      send_fatal_message( oss.str() );
      std::cerr << oss.str() << std::endl;
    }

#if DEBUG
    std::cout << ip << std::hex <<std::endl;
    std::cout << "H1 " << event_buffer[0] << std::endl;
    std::cout << "H2 " << event_buffer[1] << std::endl;
    std::cout << "H3 " << event_buffer[2] << std::endl;
    std::cout << "\n" << std::dec << std::endl;
#endif

    if(n_word_data == 0) return sizeHeader;

    ret = receive(sock, (char*)(event_buffer + n_header), sizeData);
#if DEBUG
    for(unsigned int i = 0; i<n_word_data; ++i){
      printf("D%d : %x\n", i, event_buffer[n_header+i]);
    }
#endif

    if(ret < 0) return -1;  
    return sizeHeader + sizeData;
  }


  // set_hrtdc_window ---------------------------------------------------------
  void
  set_hrtdc_window(unsigned int wmax, unsigned int wmin, FPGAModule& fModule)
  {
    static const unsigned int c_max       = 2047;
    static const unsigned int ptr_diff_wr = 2;

    unsigned int ptr_ofs = c_max - wmax + ptr_diff_wr;

    WriteMIFModule(fModule, MIFD::mid,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_ptrofs, ptr_ofs, 2);
    WriteMIFModule(fModule, MIFD::mid,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_win_max, wmax, 2);
    WriteMIFModule(fModule, HUL_MsT::MIFD::mid,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_win_min, wmin, 2);
  }

  // set_lrtdc_window ---------------------------------------------------------
  void
  set_lrtdc_window(unsigned int wmax, unsigned int wmin, FPGAModule& fModule)
  {
    static const unsigned int c_max       = 2047;
    static const unsigned int ptr_diff_wr = 2;

    unsigned int ptr_ofs = c_max - wmax + ptr_diff_wr;

    fModule.WriteModule(LRTDC::mid, LRTDC::laddr_ptrofs,  ptr_ofs);
    fModule.WriteModule(LRTDC::mid, LRTDC::laddr_win_max, wmax);
    fModule.WriteModule(LRTDC::mid, LRTDC::laddr_win_min, wmin);
  }

  // ddr_initialize --------------------------------------------------------
  void
  ddr_initialize(FPGAModule& fModule)
  {
    std::cout << "#D : Do DDR initialize" << std::endl;
    // MZN
    WriteMIFModule(fModule, MIFD::mid,
		   HRTDC_MZN::DCT::mid,
		   HRTDC_MZN::DCT::laddr_test_mode, 1, 1 );

    unsigned int reg = HUL_MsT::DCT::reg_test_mode;

    // Base
    fModule.WriteModule(HUL_MsT::DCT::mid, 
			HUL_MsT::DCT::laddr_ctrl_reg, reg);

    fModule.WriteModule(HUL_MsT::DCT::mid, 
			HUL_MsT::DCT::laddr_init_ddr, 0);

    unsigned int ret = fModule.ReadModule(HUL_MsT::DCT::mid, HUL_MsT::DCT::laddr_rcv_status, 1);

    if( ret & HUL_MsT::DCT::reg_bit_aligned){
      std::cout << "#D : DDR initialize succeeded (MZN-D)" << std::endl;
    }else{
      std::cout << "#E : Failed (MZN-D)" << std::endl;
      exit(-1);
    }// bit aligned ?

    // Set DAQ mode
   
    WriteMIFModule(fModule, MIFD::mid,
		   HRTDC_MZN::DCT::mid,
		   HRTDC_MZN::DCT::laddr_test_mode, 0, 1 );

    reg = 0;
    fModule.WriteModule(HUL_MsT::DCT::mid, 
			HUL_MsT::DCT::laddr_ctrl_reg, reg);
  
  }// ddr_initialize

  // CalibLUT ---------------------------------------------------------------
  void
  CalibLUT(FPGAModule& fModule, unsigned int mif_id)
  {
    WriteMIFModule(fModule, mif_id,
    		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_controll, 0, 1);


    WriteMIFModule(fModule, mif_id, 
		   HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_extra_path, 1, 1);

    while(!(ReadMIFModule(fModule, mif_id, HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_status, 1) & HRTDC_MZN::TDC::reg_ready_lut)){
      sleep(1);
      std::cout << "#D waiting LUT ready" << std::endl;
    }// while

    std::cout << "#D LUT is ready! (MIF-D)" << std::endl;

    WriteMIFModule(fModule, mif_id,
		   HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_extra_path, 0, 1);
    WriteMIFModule(fModule, mif_id,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_req_switch, 1, 1);
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
  nick_name = nodeprop.getNickName();
  static const std::string& func_name(nick_name+" [::"+__func__+"()]");

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

    if( arg.substr(0,11) == "--min-twin=" ){
      iss.str( arg.substr(11) );
      iss >> min_time_window;
    }

    if( arg.substr(0,11) == "--max-twin=" ){
      iss.str( arg.substr(11) );
      iss >> max_time_window;
    }

    if( arg.substr(0,11) == "--prescale=" ){
      iss.str( arg.substr(11) );
      iss >> prescale_value;
    }

    if( arg.substr(0,8) == "--timer=" ){
      iss.str( arg.substr(8) );
      iss >> timer_value;
    }

    if( arg.substr(0,9) == "--bypass=" ){
      iss.str( arg.substr(9) );
      iss >> bypass;
    }

    // J0 bus master flag
    if( arg.substr(0,8) == "--master" ){
      flag_master = true;
   }
  }

  //Connection check -----------------------------------------------
  while(0 > (sock = ConnectSocket(ip) )){
    std::ostringstream oss;
    oss << func_name << " Connection fail : " << ip;
    send_error_message( oss.str() );
    std::cerr << oss.str() << std::endl;
  }

  close(sock);

  FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
  fModule.WriteModule(BCT::mid, BCT::laddr_Reset, 0);
  ::sleep(1);
  fModule.WriteModule(MIFD::mid, MIF::laddr_frst, 1);
  ::sleep(1);
  fModule.WriteModule(MIFD::mid, MIF::laddr_frst, 0);

  ddr_initialize(fModule);
  CalibLUT(fModule, MIFD::mid);

  WriteMIFModule(fModule, MIFD::mid,
		 HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_controll, HRTDC_MZN::TDC::reg_autosw, 1);
  
  return;
}

//______________________________________________________________________________
void
init_device( NodeProp& nodeprop )
{
  static const std::string& func_name(nick_name+" [::"+__func__+"()]");

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

      // Load matrix
      {
	system("cd /home/axis/HUL_E40_MsT && bash load_mst.sh");
      }

      // Start DAQ
      FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
      {
	std::ostringstream oss;
	oss << func_name << " Firmware (BASE): " << std::hex << std::showbase
	    << fModule.ReadModule( BCT::mid, BCT::laddr_Version, 4 );
	send_normal_message( oss.str() );

	std::ostringstream oss_mzn;
	oss_mzn << func_name << " Firmware (MZN): " << std::hex << std::showbase
	    << ReadMIFModule(fModule, MIFD::mid,
			     HRTDC_MZN::BCT::mid, HRTDC_MZN::BCT::laddr_Version, 4 );
	send_normal_message( oss_mzn.str() );
      }



      if(flag_master){
	fModule.WriteModule(TRM::mid, TRM::laddr_sel_trig,
			    TRM::reg_L1RM | TRM::reg_L2RM | TRM::reg_ClrRM |
			    TRM::reg_EnL2 | TRM::reg_EnRM );
      }else{
	fModule.WriteModule(TRM::mid, TRM::laddr_sel_trig,
			    TRM::reg_L1Ext | TRM::reg_L2J0 | TRM::reg_ClrJ0 |
			    TRM::reg_EnL2  | TRM::reg_EnJ0 );
      }

      fModule.WriteModule(DCT::mid, DCT::laddr_evb_reset, 0x1);
      set_hrtdc_window(max_time_window, min_time_window, fModule);
      set_lrtdc_window(max_time_window, min_time_window, fModule);

      fModule.WriteModule(IOM::mid, IOM::laddr_pikTrig, IOM::reg_i_nimin4);

      fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_accept);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout2, IOM::reg_o_clear);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout3, IOM::reg_o_level2);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout4, IOM::reg_o_fast_clear);

      fModule.WriteModule(MsT::mid, MsT::laddr_clear_preset, prescale_value);
      fModule.WriteModule(MsT::mid, MsT::laddr_timer_preset, timer_value);
      fModule.WriteModule(MsT::mid, MsT::laddr_bypass,       bypass);

      // start DAQ
      WriteMIFModule(fModule, MIFD::mid,
		     HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_gate, 1, 1);
      fModule.WriteModule(DCT::mid, DCT::laddr_gate, 1);
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
  WriteMIFModule(fModule, MIFD::mid,
		 HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_gate, 0, 1);  

  ::sleep(1);
  unsigned int data[max_n_word];
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
  // const std::string& nick_name(nodeprop.getNickName());
  // const std::string& func_name(nick_name+" [::"+__func__+"()]");
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int ret_event_cycle = EventCycle(sock, data);
      len = ret_event_cycle == -1 ? -1 : ret_event_cycle/sizeof(unsigned int);
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
