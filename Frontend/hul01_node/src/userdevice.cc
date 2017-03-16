
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
  using namespace HUL;
  //maximum datasize by byte unit
  static const int n_word = 100; // header 3 + body 1+32*3
  static const int max_data_size = 4*n_word;
  DaqMode g_daq_mode = DM_NORMAL;

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
    static unsigned int sizeData = n_word*sizeof(unsigned int);
    int ret = receive(socket, (char*)event_buffer, sizeData);
    if( 0 > ret) return -1;

    return sizeData;
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

      fModule.WriteModule(BCT::mid, BCT::laddr_Reset,  1);
      ::sleep(1);
      fModule.WriteModule(MTM::mid, MTM::laddr_sel_trig,
			  MTM::reg_L1RM | MTM::reg_L2RM |
			  MTM::reg_EnL2 | MTM::reg_EnRM );
      fModule.WriteModule(DCT::mid, DCT::laddr_evb_reset, 0x1);
      fModule.WriteModule(SCR::mid, SCR::laddr_enable_block, 0xb);
      fModule.WriteModule(SCR::mid, SCR::laddr_counter_reset, 0x0);
      fModule.WriteModule(IOM::mid, IOM::laddr_extSpillGate, IOM::reg_i_nimin1);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_ModuleBusy);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout2, IOM::reg_o_clk1MHz);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout3, IOM::reg_o_clk10kHz);
      fModule.WriteModule(IOM::mid, IOM::laddr_nimout4, IOM::reg_o_RML1 );
      // start DAQ
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
  // const std::string& nick_name(nodeprop.getNickName());
  // const std::string& func_name(nick_name+" [::"+__func__+"()]");
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      len = EventCycle(sock, data)/sizeof(unsigned int);
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
