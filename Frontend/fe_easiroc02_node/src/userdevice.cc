#include"userdevice.h"
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<fstream>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<string.h>
#include<errno.h>

#include"easiroc_config.hh"

//maximum datasize by byte unit
static const int max_data_size = 4*1024*1024;
DaqMode g_daq_mode = DM_NORMAL;

#define EVSLIP_PREVENTION 0

static const int SiTCP_PORT = 24;
std::string ip;
int   sock = 0;

//static int event_num = 0;
extern unsigned int slowdata[sizeByte_SC];
extern unsigned int Pindata[sizeByte_PIN];
extern unsigned int ReadSCdata[sizeByte_RSC];

int  ConnectSocket(const char *ip);
int  init_easiroc();
int  receive(int sock, char* data_buf, unsigned int *ReadLength);
int  WriteData(int socket, unsigned int data);
int  ReadData(int socket, unsigned int signal, unsigned int data);

int  PowerOn_ASIC(int sock);
int  TransmitSC(int sock);
int  TransmitReadSC(int socket);
void Start_DAQCycle(int socket);
void Stop_DAQCycle(int socket);
int  Event_Cycle(int socket, unsigned int* event_buffer);

int get_maxdatasize()
{
  return max_data_size;
}

void open_device(NodeProp& nodeprop)
{
  fprintf(stdout, "open_device\n");

  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  //Connection check -------------------------------------------------------------
  std::ifstream ifs("daq_info");
  ifs >> ip;
  ifs.close();

  char message[400];
  while(0 > (sock = ConnectSocket(ip.c_str() ) )){
    sprintf(message,"EASIROC(%s)::open_device : Connection fail", ip.c_str());
    send_error_message(message);
    fprintf(stderr,"%s\n", message);
  }

  close(sock);

  return;
}

void init_device(NodeProp& nodeprop)
{
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  char message[400];
  //  event_num = 0;

  switch(g_daq_mode){
  case DM_NORMAL:
    {

      while(0 > (sock = ConnectSocket(ip.c_str() ) )){
	sprintf(message,"EASIROC(%s)::init_device : Connection fail", ip.c_str());
	send_error_message(message);
	fprintf(stderr,"%s\n", message);
      }

      sprintf(message,"EASIROC(%s)::init_device : Connection done", ip.c_str());
      send_normal_message(message);
      fprintf(stderr,"%s\n", message);

      init_easiroc();

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

void finalize_device(NodeProp& nodeprop)
{
  fprintf(stdout, "finalize_device\n");
  Stop_DAQCycle(sock);
  unsigned int data[1000];
  while(Event_Cycle(sock, data) > 0){
    fprintf(stderr, "dummy data\n");
  };

  close(sock);

  return;
}

void close_device(NodeProp& nodeprop)
{
  fprintf(stdout, "close_device\n");
  return;
}


int wait_device(NodeProp& nodeprop)
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{

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


int read_device(NodeProp& nodeprop, unsigned int* data, int& len)
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  fprintf(stdout, "read_device\n");
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int ret_event_cycle = Event_Cycle(sock, data);
      len = ret_event_cycle == -1 ? -1 : ret_event_cycle/sizeof(unsigned int);

#if  EVSLIP_PREVENTION
      //      fprintf(stderr, "%d %d %d\n", event_tag, event_counter, event_num);

      if(len > 0){
	int event_num     = nodeprop.getEventNumber();
	int event_tag     = (int)*(data +2) & 0x1;
	int event_counter = (int)((*(data +2)>>16) & 0xffff);
	char message[400];

	if((event_num & 0xfff) != event_counter){
	  sprintf(message, "EASIROC(%s)::read_device : Event slip (Local)", ip.c_str());
	  send_fatal_message(message);
	  std::ofstream ofs("coredump.dat", std::ios::binary);
	  int errernum = 0xe1;
	  ofs.write((char*)&errernum, sizeof(int));
	  ofs.write((char*)&event_num, sizeof(int));
	  ofs.write((char*)&event_counter, sizeof(int));
	  ofs.write((char*)&event_tag, sizeof(int));
	  ofs.write((char*)data, len);
	  ofs.close();
	  exit(-1);
	}

	if(((event_tag) & 0x1 ) != (event_counter+1 & 0x1)){
	  sprintf(message, "EASIROC(%s)::read_device : Event slip (Global)", ip.c_str());
	  send_fatal_message(message);
	  std::ofstream ofs("coredump.dat", std::ios::binary);
	  int errernum = 0xe2;
	  ofs.write((char*)&errernum, sizeof(int));
	  ofs.write((char*)&event_num, sizeof(int));
	  ofs.write((char*)&event_counter, sizeof(int));
	  ofs.write((char*)&event_tag, sizeof(int));
	  ofs.write((char*)data, len);
	  ofs.close();
	  exit(-1);
	}
      }
#endif
      //      event_num++;

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


int ConnectSocket(const char *ip)
{
  struct sockaddr_in SiTCP_ADDR;
  unsigned int port;

  int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  port                       = SiTCP_PORT;
  SiTCP_ADDR.sin_family      = AF_INET;
  SiTCP_ADDR.sin_port        = htons((unsigned short int)port);
  SiTCP_ADDR.sin_addr.s_addr = inet_addr(ip);

  struct timeval tv;
  tv.tv_sec  = 3;
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

  int flag = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

  //Connection -------------------------------------------------------------
  if(0 > connect(sock, (struct sockaddr*)&SiTCP_ADDR, sizeof(SiTCP_ADDR))){
    close(sock);
    return -1;
  }

  return sock;
}

// Methods for EASIROC ------------------------------------------------------

int init_easiroc()
{
  char message[400];

  fprintf(stdout, "init_easiroc\n");

  // Power On ASIC --------------------------------------------------------
  PrepareFPGA();
  PowerOn_ASIC(sock);
  // slow controll EASIROC ------------------------------------------------
  PrepareSC();
  TransmitSC(sock);
  PrepareReadSC();
  TransmitReadSC(sock);

  // select DAQ mode ------------------------------------------------------
  {
    unsigned int daq_mode = 1; // 1: MHTDC, 2: ADC, 3: MHTDC & ADC
    unsigned int signal   = 31;
    signal = signal << 16;
    unsigned int data = daq_mode;
    data = data << 8;
    signal += data;
    if(-1 == WriteData(sock, signal)){
      exit(-1);
    }
    sprintf(message, "EASIROC(%s)::init_easiroc : DAQ mode is %d", ip.c_str(), daq_mode);
    send_normal_message(message);
    fprintf(stderr, "%s\n", message);
  }

  // DAQ Start ------------------------------------------------------------
  Start_DAQCycle(sock);

  return 0;
}

int receive(int sock, char* data_buf, unsigned int *ReadLength){
  unsigned int revd_size = 0;
  int tmp_returnVal      = 0;

  char message[400];

  while(revd_size < *ReadLength){
    tmp_returnVal = recv(sock, data_buf +revd_size, *ReadLength -revd_size, 0);
    if(tmp_returnVal == 0){break;}
    if(tmp_returnVal < 0){
      int errbuf = errno;
      perror("TCP receive");
      if(errbuf == EAGAIN){
	// time out
      }else{
	// something wrong
	sprintf(message, "EASIROC(%s)::receive : TCP receive error No. is %d", ip.c_str(), errbuf);
	send_error_message(message);
      }

      revd_size = tmp_returnVal;
      break;
    }
    revd_size += tmp_returnVal;
  }

  return revd_size;
}


int WriteData(int sock, unsigned int data){
  char message[400];

  data += 128 << 24;
//   for(int i = 31; i>-1; --i){
//     unsigned int hoge = (data >> i) & 1;
//     if(hoge){
//       fprintf(stdout, "!");
//     }else{
//       fprintf(stdout, ".");
//     }
//     if(i%8 == 0){
//       fprintf(stdout, "\n");
//     }
//   }
//   fprintf(stdout, "\n");

//  fprintf(stderr, "%x\n", data);
  send(sock, (char*)&data, sizeof(int), 0);
  unsigned int buf    = 0;
  unsigned int length = 4;
  receive(sock, (char*)&buf, &length);
  if(data - buf){
    fprintf(stderr, "#E : Data Transmit Error (%s)\n", ip.c_str());
    fprintf(stderr, "Transmited data is %x\n", data);
    fprintf(stderr, "Returned data is   %x\n", buf);

    sprintf(message, "EASIROC(%s)::WriteData : Data Transmit Error\n \
                      Transmited data is %x\n \
                      Returned data is   %x", ip.c_str(), data, buf);
    send_fatal_message(message);

    std::exit(-1);
  }

  return 0;
}

int ReadData(int sock, unsigned int signal, unsigned int *data){
  signal += 64 << 24;
  send(sock, (char*)&signal, sizeof(int), 0);
  unsigned int length = 4;
  receive(sock, (char*)data, &length);

//    for(int i = 31; i>-1; --i){
//      unsigned int hoge = (*data >> i) & 1;
//      if(hoge){
//        fprintf(stdout, "!");
//      }else{
//        fprintf(stdout, ".");
//      }
//      if(i%8 == 0){
//        fprintf(stdout, "\n");
//      }
//    }
//    fprintf(stdout, "\n");

  return 0;
}

int PowerOn_ASIC(int socket){
  unsigned int buffer = 0;
  buffer += 0 << 16;
  buffer += (Pindata[0] & 255) << 8;
  if(-1 == WriteData(socket, buffer)){
    exit(-1);
  }

  for(int i = 1 ; i<5; ++i){
    buffer = 0;
    if(i == 4){
      buffer += 5 << 16;
    }else{
      buffer += i << 16;
    }
    buffer += ((Pindata[1] >> (i-1)*8) & 255) << 8;

    if(-1 == WriteData(socket, buffer)){
      exit(-1);
    }
    usleep(1);
  }

  return 0;
}


int TransmitSC(int socket){
  unsigned int data = 0;
  //Set SC mode -----------------------------------------------------------
  data = 0;
  data += 1 << 16;
  data += 240 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  //SC start -------------------------------------------------------------
  data = 0;
  data += 10 << 16;
  data += (slowdata[0] & 255) << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  for(int i = 1; i<15; ++i){
    for(int shift = 0; shift<4; ++shift){
      data = 0;
      data += 10 << 16;
      data += ((slowdata[i] >> 8*shift) & 255) << 8;
      if(-1 == WriteData(socket, data)){
	exit(-1);
      }
      usleep(1);
    }
  }

  usleep(50000);

  //StartCycle -----------------------------------------------------------
  data = 0;
  data += 1 << 16;
  data += 242 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  data = 0;
  data += 1 << 16;
  data += 240 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  usleep(50000);

  //Load SC --------------------------------------------------------------
  data = 0;
  data += 1 << 16;
  data += 241 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  data = 0;
  data += 1 << 16;
  data += 240 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  return 0;
}

int TransmitReadSC(int socket){
  //SCA read ---------------------------------------------------------------
  unsigned int data = 0;

  for(int i = 0; i<4; ++i){
    data = 0;
    data += 12 << 16;
    data += ((ReadSCdata[0] >> i*8) & 255) << 8;
    if(-1 == WriteData(socket, data)){
      exit(-1);
    }
    usleep(1);
  }

  //StartCycle ------------------------------------------------------------
  data = 0;
  data += 1 << 16;
  data += 242 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  data = 0;
  data += 1 << 16;
  data += 240 << 8;
  if(-1 == WriteData(socket, data)){
    exit(-1);
  }

  return 0;
}

// DAQ cycle ---------------------------------------------------------------
void Start_DAQCycle(int socket){
  unsigned int signal = 0;
  signal += 32 << 24;
  signal += 100 << 16;
  send(socket, (char*)&signal, sizeof(int), 0);

  return;
}

int Event_Cycle(int socket, unsigned int* event_buffer){
  static unsigned int sizeHeader = sizeof(unsigned int)*3;
  int ret = receive(socket, (char*)event_buffer, &sizeHeader);
  // Header read
  if( 0 > ret){return -1;}

  char message[400];

  if(0xFFFFEA0C != *(event_buffer)){
    // fatal DAQ errer
    // core dump ------------------------------------------------------------
    fprintf(stderr, "Fatal DAQ errer\n");
    sprintf(message, "EASIROC(%s)::Event_Cycle : Header is broken", ip.c_str());
    send_fatal_message(message);

    std::ofstream ofs("coredump.dat", std::ios::binary);
    int errernum = 0xe0;
    ofs.write((char*)&errernum, sizeof(int));
    ofs.write((char*)event_buffer, 3*sizeof(int));
    ofs.close();
    std::exit(-1);
  }

  unsigned int sizeData = sizeof(unsigned int)*(*(event_buffer + 1) & 0xffff);
  ret = receive(socket, (char*)(event_buffer +3), &sizeData);
  if( 0 > ret){return -1;}

  return sizeHeader + sizeData;
}

void Stop_DAQCycle(int socket){
  unsigned int signal = 0;
  fprintf(stderr, "ADC exit process\n");
  signal += 16 << 24;
  signal += 100 << 16;
  send(socket, (char*)&signal, sizeof(int), 0);
  sleep(1);

  signal = 0;
  signal += 100 << 16;
  send(socket, (char*)&signal, sizeof(int), 0);

  usleep(10000);

  return;
}
