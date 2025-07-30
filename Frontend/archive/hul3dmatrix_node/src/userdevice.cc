#include"userdevice.h"
#include"daq_funcs.hh"
#include<cstdio>
#include<cstdlib>

#define DEBUG_MODE 0

//maximum datasize by byte unit
static const int n_word = 121;
static const int max_data_size = 4*n_word;
DaqMode g_daq_mode = DM_NORMAL;

char ip[100];
int  window_low;
int  window_high;
int  sock=0;
rbcp_header rbcpHeader;

int  ConnectSocket(char *ip);
int  Event_Cycle(int socket, unsigned int* event_buffer);

int get_maxdatasize()
{
  return max_data_size;
}

void
open_device(NodeProp& nodeprop)
{
  fprintf(stdout, "open_device\n");

  // RBCP
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  // load parameters
  FILE* fp = fopen("daq_info", "r");
  fscanf(fp, "ip          %s\n", ip);
  fscanf(fp, "window_low  %d\n", &window_low);
  fscanf(fp, "window_high %d\n", &window_high);
  fclose(fp);
  printf("#D : ip %s\n", ip);
  printf("#D : %d < window < %d\n", window_low, window_high);

  //Connection check -----------------------------------------------
  char message[400];
  while(0 > (sock = ConnectSocket(ip) )){
    sprintf(message,"HUL(%s)::open_device : Connection fail", ip);
    send_error_message(message);
    fprintf(stderr,"%s\n", message);
  }

  close(sock);

  return;
}

void
init_device(NodeProp& nodeprop)
{
  fprintf(stdout, "init_device\n");

  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  char message[400];
  //  event_num = 0;

  switch(g_daq_mode){
  case DM_NORMAL:
    {

      while(0 > (sock = ConnectSocket(ip) )){
	sprintf(message,"HUL(%s)::init_device : Connection fail", ip);
	send_error_message(message);
	fprintf(stderr,"%s\n", message);
      }

      sprintf(message,"HUL(%s)::init_device : Connection done", ip);
      send_normal_message(message);
      fprintf(stderr,"%s\n", message);

      // Start DAQ
      FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
      fModule.WriteModule(BCT::mid, BCT::laddr_Reset,  1);
      sleep(1);
      fModule.WriteModule(TDC::mid, TDC::laddr_start,  1);
      fModule.WriteModule(TDC::mid, TDC::laddr_lowth,  window_low);
      fModule.WriteModule(TDC::mid, TDC::laddr_highth, window_high);

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

void
finalize_device(NodeProp& nodeprop)
{
  fprintf(stdout, "finalize_device\n");
  FPGAModule fModule(ip, udp_port, &rbcpHeader, 0);
  fModule.WriteModule(TDC::mid, TDC::laddr_start,  0);

  unsigned int data[n_word];
  while(-1 != Event_Cycle(sock, data));

  shutdown(sock, SHUT_RDWR);
  close(sock);

  return;
}

void
close_device(NodeProp& nodeprop)
{
  fprintf(stdout, "close_device\n");
  return;
}


int
wait_device(NodeProp& nodeprop)
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


int
read_device(NodeProp& nodeprop, unsigned int* data, int& len)
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
#if DEBUG_MODE
      if(len > 0){
	std::cout << "#D : I received Event" << std::endl;
	for(int i = 0; i<n_word; ++i){
	  printf("%9x ", data[i]);
	  if((i+1)%8==0) printf("\n");
	}
	printf("\n");
      }
#endif

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

int
ConnectSocket(char *ip)
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

  //Connection -------------------------------------------------------------
  if(0 > connect(sock, (struct sockaddr*)&SiTCP_ADDR, sizeof(SiTCP_ADDR))){
    close(sock);
    return -1;
  }

  return sock;
}

int
receive(int sock, char* data_buf, unsigned int ReadLength)
{
  unsigned int revd_size = 0;
  int tmp_returnVal      = 0;

  char message[400];
  while(revd_size < ReadLength){
    tmp_returnVal = recv(sock, data_buf +revd_size, ReadLength -revd_size, 0);
    if(tmp_returnVal == 0){break;}
    if(tmp_returnVal < 0){
      int errbuf = errno;
      perror("TCP receive");
      if(errbuf == EAGAIN){
	// time out
      }else{
	// something wrong
	sprintf(message, "HUL(%s)::receive : TCP receive error No. is %d", ip, errbuf);
	send_error_message(message);
      }

      revd_size = tmp_returnVal;
      break;
    }
    revd_size += tmp_returnVal;
  }

  return revd_size;
}

int Event_Cycle(int socket, unsigned int* event_buffer)
{
  static unsigned int sizeData = n_word*sizeof(unsigned int);
  int ret = receive(socket, (char*)event_buffer, sizeData);
  if( 0 > ret) return -1;

  return sizeData;
}
