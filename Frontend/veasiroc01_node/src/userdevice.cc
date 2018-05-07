#include "userdevice.h"

#include<iostream>
#include<fstream>
#include<sstream>
#include<bitset>
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<string.h>

static const unsigned int udp_port = 4660;
static const unsigned int tcp_port = 24;

#include "rbcp.h"
#include "endian.h"

DaqMode g_daq_mode  = DM_NORMAL;
std::string nick_name;
#define DEBUG 0

namespace
{
  // RBCP registers --------------------------------------
  static const uint32_t DIRECT_CTRL_ADDRESS = 0x00000000;
  static const uint32_t STATUS_REG_ADDRESS  = 0x00000077;

  static const uint8_t daqModeBit    = 0x01;
  static const uint8_t sendAdcBit    = 0x02;
  static const uint8_t sendTdcBit    = 0x04;
  static const uint8_t sendScalerBit = 0x08;

  bool isDaqMode  = false;
  bool sendAdc    = true;
  bool sendTdc    = true;
  bool sendScaler = false;

  // Direct control register
  enum indexRegEasiroc{
    i_razChn,   i_valEvt, i_resetPA, i_pwrOn,
    i_selectSc, i_loadSc, i_rstbSr,  i_rstbRead
  };
  enum indexRegModule{
    i_startCycle1, i_startCycle2, i_userOutput, i_ledUser,
    i_ledReady,    i_ledBusy,     i_selectHg,   i_selectProbe
  };
  std::bitset<8> reg_easiroc1;
  std::bitset<8> reg_easiroc2;
  std::bitset<8> reg_module;

  //maximum datasize by byte unit
  static const int n_header      = 3;
  static const int max_n_word    = n_header+2*16*64 + 64*2;
  static const int max_data_size = sizeof(unsigned int)*max_n_word;

  char ip[100];
  int  sock=0;

  //_________________________________________________________________________
  // local function
  //_________________________________________________________________________
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

  //_________________________________________________________________________
  void
  readAndThrowPreviousData(double timeout = 0.1)
  {
    int thrownBytes = 0;
    while(1) {
      fd_set rfds;
      FD_ZERO(&rfds);
      FD_SET(sock, &rfds);

      struct timeval tv;
      tv.tv_sec = static_cast<unsigned int>(timeout);
      tv.tv_usec = static_cast<unsigned int>(timeout * 1000 * 1000);

      int select_ret = select(sock + 1, &rfds, NULL, NULL, &tv);
      if(select_ret < 0) {
	perror("readAndThrowPreviousData");
	throw std::runtime_error("select failed");
	return;
      }else if(select_ret == 0) {
	break;
      }else{
	char buf[256];
	thrownBytes += recv(sock, buf, sizeof(buf), 0);
      }
    }
    std::cout << thrownBytes << " bytes are thrown" << std::endl;
  }

  //_________________________________________________________________________
  void
  writeStatusRegister(uint8_t data = 0)
  {
    if(isDaqMode) {
      data |= daqModeBit;
    }
    if(sendAdc) {
      data |= sendAdcBit;
    }
    if(sendTdc) {
      data |= sendTdcBit;
    }
    if(sendScaler) {
      data |= sendScalerBit;
    }

    RBCP rbcp(std::string(ip), udp_port);
    rbcp.write(&data, STATUS_REG_ADDRESS, 1);
    //printf("status register %02X\n", data);
  }

  //_________________________________________________________________________
  void
  sendDirectControl()
  {
    const size_t n_reg = 3;
    uint8_t reg_direct[n_reg] = {
      static_cast<uint8_t>(reg_easiroc1.to_ulong()),
      static_cast<uint8_t>(reg_easiroc2.to_ulong()),
      static_cast<uint8_t>(reg_module.to_ulong())
    };

    RBCP rbcp(std::string(ip), udp_port);
    rbcp.write(reg_direct, DIRECT_CTRL_ADDRESS, n_reg);

  }

  //_________________________________________________________________________
  void
  enterDaqMode()
  {
    isDaqMode = true;
    writeStatusRegister();
  }

  //_________________________________________________________________________
  void
  exitDaqMode()
  {
    isDaqMode = false;
    writeStatusRegister();
  }
  
  //_________________________________________________________________________
  int
  receiveNByte(uint8_t* buf, size_t bytes)
  {
    size_t receivedBytes = 0;
    while(receivedBytes < bytes) {
      ssize_t receivedDataLength = recv(sock, buf + receivedBytes,
					bytes - receivedBytes, 0);
      if(receivedDataLength < 0) {
	perror("recv");
	throw std::runtime_error("recv failed");
      }
      receivedBytes += receivedDataLength;
      //std::cout << "receiveNbyte_: " << receivedDataLength << " bytes are read" << std::endl;
    }
    return receivedBytes;
  }

  //_________________________________________________________________________
  uint32_t
  decodeWord(uint32_t word)
  {
    return ((word & 0x7f000000) >> 3) | ((word & 0x007f0000) >> 2) |
           ((word & 0x00007f00) >> 1) | ((word & 0x0000007f) >> 0);
  }

  //_________________________________________________________________________
  uint32_t
  receiveHeader()
  {
    uint8_t headerBuf[4];
    if(receiveNByte(headerBuf, 4) < 0) {
      std::cout << "Receive header failed" << std::endl;
      throw std::runtime_error("Receive header failed");
    }
    uint32_t header        = Endian::getBigEndian32(headerBuf);
    uint32_t decodedHeader = decodeWord(header);
    bool isHeader = decodedHeader & 0x08000000;
    if(!isHeader) {
      std::cout << "Frame Error" << std::endl;
      printf("    %08X\n", header);
      throw std::runtime_error("Frame Error");
    }
    return decodedHeader;
  }

  //_________________________________________________________________________
  size_t
  receiveData(uint8_t* buf, size_t length)
  {
    return receiveNByte(buf, length * 4);
  }
  
}

//___________________________________________________________________________
int
get_maxdatasize( void )
{
  return max_data_size;
}

//___________________________________________________________________________
void
open_device( NodeProp& nodeprop )
{
  nick_name = nodeprop.getNickName();
  static const std::string& func_name(nick_name+" [::"+__func__+"()]");

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

  // reset direct control registers
  reg_easiroc1.set(   i_rstbRead );
  reg_easiroc1.set(   i_rstbSr   );
  reg_easiroc1.reset( i_loadSc   );
  reg_easiroc1.set(   i_selectSc );
  reg_easiroc1.set(   i_pwrOn    );
  reg_easiroc1.set(   i_resetPA  );
  reg_easiroc1.set(   i_valEvt   );
  reg_easiroc1.reset( i_razChn   );

  reg_easiroc2.set(   i_rstbRead );
  reg_easiroc2.set(   i_rstbSr   );
  reg_easiroc2.reset( i_loadSc   );
  reg_easiroc2.set(   i_selectSc );
  reg_easiroc2.set(   i_pwrOn    );
  reg_easiroc2.set(   i_resetPA  );
  reg_easiroc2.set(   i_valEvt   );
  reg_easiroc2.reset( i_razChn   );

  reg_module.reset( i_selectProbe );
  reg_module.reset( i_selectHg    );
  reg_module.reset( i_ledBusy     );
  reg_module.reset( i_ledReady    );
  reg_module.reset( i_ledUser     );
  reg_module.reset( i_userOutput  );
  reg_module.reset( i_startCycle2 );
  reg_module.reset( i_startCycle1 );

  sendDirectControl();

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

      readAndThrowPreviousData();
      enterDaqMode();
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

  int ret = -10;
  exitDaqMode();
  if(sock >= 0) {
    ret = close(sock);
  }
  std::cout << "finalize_device() " << ret << std::endl;

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
      try {
        uint32_t header = receiveHeader();
        uint8_t spillNumber = (header & 0x8000) >> 15;
        uint8_t eventNumber = (header & 0x7000) >> 12;
        uint32_t dataSize = header & 0x0fff;

        Endian::setLittleEndian32(data, 0x4157494d);
        data++;
        Endian::setLittleEndian32(data, dataSize);
        data++;
        Endian::setLittleEndian32(data, spillNumber << 3 | eventNumber);
        data++;

	//        if((*eventNum & 0x00000007) != eventNumber) {
	//std::cerr << "Event slip detected!!!!" << std::endl;
            //exit(1);
	//        }

        size_t receivedDataLength = receiveData(reinterpret_cast<uint8_t*>(data),
                                                 static_cast<size_t>(dataSize));
        if(receivedDataLength != dataSize * 4) {
            std::cerr << "receiveData failed" << std::endl;
            return -1;
        }
        len = dataSize + 3;
        return 1;
      }catch(std::exception &e) {
        std::cout << e.what() << std::endl;
        return -1;
      }

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
