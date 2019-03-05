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
  using namespace HRTDC_BASE;
  //maximum datasize by byte unit
  static const int n_header      = 3;
  static const int max_n_word    = n_header + 2 + 16*64 + 16*64;
  static const int max_data_size = sizeof(unsigned int)*max_n_word;

  char ip[100];
  unsigned int min_time_window;
  unsigned int max_time_window;
  
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

    unsigned int n_word_data  = event_buffer[1] & 0x3ff;
    unsigned int sizeData     = n_word_data*sizeof(unsigned int);

    if(event_buffer[0] != 0xffff800b){
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
#endif

    if(n_word_data == 0) return sizeHeader;

    ret = receive(sock, (char*)(event_buffer + n_header), sizeData);
#if DEBUG
    for(unsigned int i = 0; i<n_word_data; ++i){
      printf("D%d : %x\n", i, event_buffer[n_header+i]);
    }
    std::cout << "\n" << std::dec << std::endl;
#endif

    if(ret < 0) return -1;  
    return sizeHeader + sizeData;
  }


  // set_tdc_window ------------------------------------------------------------
  void
  set_tdc_window(unsigned int wmax, unsigned int wmin, FPGAModule& fModule, unsigned int mif_id)
  {
    static const unsigned int c_max       = 2047;
    static const unsigned int ptr_diff_wr = 2;

    unsigned int ptr_ofs            = c_max - wmax + ptr_diff_wr;

    WriteMIFModule(fModule, mif_id,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_ptrofs, ptr_ofs, 2);
    WriteMIFModule(fModule, mif_id,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_win_max, wmax, 2);
    WriteMIFModule(fModule, mif_id,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_win_min, wmin, 2);

  }

  // ddr_initialize --------------------------------------------------------
  void
  ddr_initialize(FPGAModule& fModule)
  {
    unsigned int reg_enable_up   = en_up   ? HRTDC_BASE::DCT::reg_enable_u : 0;
    unsigned int reg_enable_down = en_down ? HRTDC_BASE::DCT::reg_enable_d : 0;

    std::cout << "#D : Do DDR initialize" << std::endl;
    // MZN
    if(en_up){
      WriteMIFModule(fModule, MIFU::mid,
		     HRTDC_MZN::DCT::mid,
		     HRTDC_MZN::DCT::laddr_test_mode, 1, 1 );
    }

    if(en_down){
      WriteMIFModule(fModule, MIFD::mid,
		     HRTDC_MZN::DCT::mid,
		     HRTDC_MZN::DCT::laddr_test_mode, 1, 1 );
    }

    unsigned int reg =
      reg_enable_up   |
      reg_enable_down |
      HRTDC_BASE::DCT::reg_test_mode_u |
      HRTDC_BASE::DCT::reg_test_mode_d;

    // Base
    fModule.WriteModule(HRTDC_BASE::DCT::mid, 
			HRTDC_BASE::DCT::laddr_ctrl_reg, reg);

    fModule.WriteModule(HRTDC_BASE::DCT::mid, 
			HRTDC_BASE::DCT::laddr_init_ddr, 0);

    unsigned int ret = fModule.ReadModule(HRTDC_BASE::DCT::mid, HRTDC_BASE::DCT::laddr_rcv_status, 1);

    if(en_up){
      if( ret & HRTDC_BASE::DCT::reg_bit_aligned_u){
	std::cout << "#D : DDR initialize succeeded (MZN-U)" << std::endl;
      }else{
	std::cout << "#E : Failed (MZN-U)" << std::endl;
	exit(-1);
      }
    }// bit aligned ?

    if(en_down){
      if( ret & HRTDC_BASE::DCT::reg_bit_aligned_d){
	std::cout << "#D : DDR initialize succeeded (MZN-D)" << std::endl;
      }else{
	std::cout << "#E : Failed (MZN-D)" << std::endl;
	exit(-1);
      }
    }// bit aligned ?

    // Set DAQ mode
   
    if(en_up){
      WriteMIFModule(fModule, MIFU::mid,
		     HRTDC_MZN::DCT::mid,
		     HRTDC_MZN::DCT::laddr_test_mode, 0, 1 );

    }
 
    if(en_down){
      WriteMIFModule(fModule, MIFD::mid,
		     HRTDC_MZN::DCT::mid,
		     HRTDC_MZN::DCT::laddr_test_mode, 0, 1 );
    }

    reg = reg_enable_up | reg_enable_down;
    fModule.WriteModule(HRTDC_BASE::DCT::mid, 
			HRTDC_BASE::DCT::laddr_ctrl_reg, reg);
  
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

    if((int)mif_id == MIFU::mid){
      std::cout << "#D LUT is ready! (MIF-U)" << std::endl;
    }else{
      std::cout << "#D LUT is ready! (MIF-D)" << std::endl;
    }

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
  if(en_up)  fModule.WriteModule(HRTDC_BASE::MIFU::mid, HRTDC_BASE::MIF::laddr_frst, 1);
  if(en_down)fModule.WriteModule(HRTDC_BASE::MIFD::mid, HRTDC_BASE::MIF::laddr_frst, 1);
  ::sleep(1);
  if(en_up)  fModule.WriteModule(HRTDC_BASE::MIFU::mid, HRTDC_BASE::MIF::laddr_frst, 0);
  if(en_down)fModule.WriteModule(HRTDC_BASE::MIFD::mid, HRTDC_BASE::MIF::laddr_frst, 0);

  ddr_initialize(fModule);
  if(en_up)   CalibLUT(fModule, MIFU::mid);
  if(en_down) CalibLUT(fModule, MIFD::mid);

  unsigned int tdc_ctrl = HRTDC_MZN::TDC::reg_autosw;
  if(en_up){
    WriteMIFModule(fModule, MIFU::mid,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_controll, tdc_ctrl, 1);    

  }

  if(en_down ){
    WriteMIFModule(fModule, MIFD::mid,
		   HRTDC_MZN::TDC::mid, HRTDC_MZN::TDC::laddr_controll, tdc_ctrl, 1);
  } 
  
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

      // Start DAQ
      FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
      {
	std::ostringstream oss;
	oss << func_name << " Firmware (BASE): " << std::hex << std::showbase
	    << fModule.ReadModule( BCT::mid, BCT::laddr_Version, 4 );
	send_normal_message( oss.str() );

	std::ostringstream oss_mznu;
	oss_mznu << func_name << " Firmware (MZNU): " << std::hex << std::showbase
	    << ReadMIFModule(fModule, MIFU::mid,
			     HRTDC_MZN::BCT::mid, HRTDC_MZN::BCT::laddr_Version, 4 );
	send_normal_message( oss_mznu.str() );

	std::ostringstream oss_mznd;
	oss_mznd << func_name << " Firmware (MZND): " << std::hex << std::showbase
	    << ReadMIFModule(fModule, MIFD::mid,
			     HRTDC_MZN::BCT::mid, HRTDC_MZN::BCT::laddr_Version, 4 );
	send_normal_message( oss_mznd.str() );
      }

      fModule.WriteModule(TRM::mid, TRM::laddr_sel_trig,
			  TRM::reg_L1Ext | TRM::reg_L2J0 | TRM::reg_ClrJ0
			  | TRM::reg_EnL2 | TRM::reg_EnJ0 );

      fModule.WriteModule(DCT::mid, DCT::laddr_evb_reset, 0x1);
      if(en_up)   set_tdc_window(max_time_window, min_time_window, fModule, MIFU::mid);
      if(en_down) set_tdc_window(max_time_window, min_time_window, fModule, MIFD::mid);

      fModule.WriteModule(IOM::mid, IOM::laddr_extL1,  IOM::reg_i_nimin1);
      //fModule.WriteModule(IOM::mid, IOM::laddr_extL2,  IOM::reg_i_nimin2);
      //fModule.WriteModule(IOM::mid, IOM::laddr_extClr, IOM::reg_i_nimin3);

      // start DAQ
      if(en_up){
	WriteMIFModule(fModule, MIFU::mid,
		       HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_gate, 1, 1);    
      }

      if(en_down ){
	WriteMIFModule(fModule, MIFD::mid,
		       HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_gate, 1, 1);
      } 
      
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
  if(en_up){
    WriteMIFModule(fModule, MIFU::mid,
		   HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_gate, 0, 1);
  }

  if(en_down){
    WriteMIFModule(fModule, MIFD::mid,
		   HRTDC_MZN::DCT::mid, HRTDC_MZN::DCT::laddr_gate, 0, 1);  
  }

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
