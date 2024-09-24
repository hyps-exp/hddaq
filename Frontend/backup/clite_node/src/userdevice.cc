#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "userdevice.h"
#include "nodeprop.h"
#include "cludp.h"
#include "cltcp.h"
#include "copperlite.h"
#include "copperliteparam.h"

#define SET_REGISTER 1

#define TCP_PORT 24
#define UDP_PORT 4660

static int socktcp;
static int sockudp;

static char udphost[256];
static char tcphost[256];
static int udpport = UDP_PORT;
static int tcpport = TCP_PORT;

static char paramfile[256];

static const int max_data_size = 4*1024*1024;

DaqMode g_daq_mode = DM_NORMAL;

int get_maxdatasize()
{
  return max_data_size;
}

//-------------------------------------------------------------
// open_device
//-------------------------------------------------------------
void open_device(NodeProp& nodeprop)
{
  printf("open_device\n");

  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

      static char sval[256];
      int val = 0;

      strcpy(udphost, "192.168.0.16");
      strcpy(tcphost, "192.168.0.17");
      paramfile[0] = 0;

      int g_argc = nodeprop.getArgc();

      // set parameters from main arguments -----------------------
      for (int i = 1 ; i < g_argc ; i++) {
      	const char* str = (nodeprop.getArgv(i)).c_str();
      	printf("hoge %s\n",str);
      	if (str[0] != '-'){
      	  strncpy(paramfile, str, 255);
      	}
      	if (sscanf(str, "--udp-host=%s", sval) == 1) {
      	  strncpy(udphost, sval, 256);
      	}
      	if (sscanf(str, "--tcp-host=%s", sval) == 1) {
      	  strncpy(tcphost, sval, 256);
      	}
      	if (sscanf(str, "--udp-port=%d", &val) == 1) {
      	  udpport = val;
      	}
      	if (sscanf(str, "--tcp-port=%d", &val) == 1) {
      	  tcpport = val;
      	}
      	if (sscanf(str, "--finesse=%x", &val) == 1) {
      	  cltcp_set_finesse_act(val);
      	}
      }

      // load parameters ------------------------------------------
      if (paramfile[0]) {
      	printf("Parameter file: %s\n", paramfile);
      	get_finesse_param(paramfile);
      	cltcp_set_finesse_act(get_finesse_act());
      	if (get_udphost()[0] != 0) {
      	  strncpy(udphost, get_udphost(), 256);
      	}
      	if (get_tcphost()[0] != 0) {
      	  strncpy(tcphost, get_tcphost(), 256);
      	}
      	if (get_udpport() != 0) {
      	  udpport = get_udpport();
      	}
      	if (get_tcpport() != 0) {
      	  tcpport = get_tcpport();
      	}
      }

      printf("UDP host: %s, port: %d\n", udphost, udpport);
      printf("TCP host: %s, port: %d\n", tcphost, tcpport);

      // Connect UDP & transmit registers --------------------------
      sockudp = udpreg_open(udphost, udpport);
      if (sockudp <= 0) {
      	perror("udpreg_open");
      	char message[400];
      	sprintf(message,
      		"clite::open_device udpreg_open error (%s)",
      		udphost
      		);
      	send_error_message(message);
      	std::exit(-1);
      }

      ///// set register from here
#if SET_REGISTER
      int status;
      static char clreg[5];
      int slot, addr;
      int claddr;

      /* clreg[0] = 0x8f ; clreg[1] = 0x00; */
      clreg[0] = 0x80 | cltcp_get_finesse_act();
      clreg[1] = 0x00;
      status = udpreg_write(sockudp, clreg, CL_FCR, 1);
      clreg[0] = 0x00;
      clreg[1] = 0x01;
      clreg[2] = 0x00;
      clreg[3] = 0x00;
      clreg[4] = 0x00;
      status = udpreg_write(sockudp, clreg, CL_FFTH, 4);

      clreg[0] = 0x80 ; clreg[1] = 0x00;
      status = udpreg_write(sockudp, clreg, CL_RCR, 1);
      clreg[0] = 0x00 ; clreg[1] = 0x00;
      status = udpreg_write(sockudp, clreg, CL_RCR, 1);
      usleep(100);

      /*
      	clreg[0] = 0x30 ; clreg[1] = 0x00;
      	status = udpreg_write(sockudp, clreg, 0x60b, 1);
      	clreg[0] = 0x30 ; clreg[1] = 0x00;
      	status = udpreg_write(sockudp, clreg, 0x60f, 1);
      */

      clreg[1] = 0x00;
      while(fregque_get(&slot, &addr, &val)) {
      	if (slot == 0xf) {
      	  claddr   = CL_FINESSE_ALL + addr * 4 + 3;
      	  clreg[0] = val;
      	  status   = udpreg_write(sockudp, clreg, claddr, 1);
      	  printf("#D all write %x %x\n", claddr, clreg[0]);
      	} else {
      	  for ( int i=0; i<4 ;++i ) {
      	    if (((slot >> i) & 0x01) == 0x1) {
      	      claddr   = CL_FINESSE + addr * 4 + i;
      	      clreg[0] = val;
      	      status   = udpreg_write(sockudp, clreg, claddr, 1);
      	      /* printf("#D write %d %x %x\n", i, claddr, clreg[0]); */
      	    }
      	  } // for(i)
      	} // if
      }// while
#endif

  // Connect TCP ---------------------------------------------
#ifdef TCPOPENATOPEN
  socktcp = cltcp_open(tcphost, tcpport);
  if (socktcp <= 0) {
    perror("cltcp_open");
    char message[400];
    sprintf(message,
	    "clite::open_device cltcp_open error (%s)",
	    tcphost
	    );
    send_error_message(message);
    std::exit(-1);
  }
#endif

  return;
}

//-------------------------------------------------------------
// init_device
//-------------------------------------------------------------
void init_device(NodeProp& nodeprop)
{
  printf("init_device\n");
  
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  switch(g_daq_mode){
  case DM_NORMAL:
    {

      // Connect TCP ------------------------------------------
#ifndef TCPOPENATOPEN
      socktcp = cltcp_open(tcphost, tcpport);
      if (socktcp <= 0) {
	perror("cltcp_open");
	char message[400];
	sprintf(message,
		"clite::open_device cltcp_open error (%s)",
		tcphost
		);
	send_error_message(message);
	std::exit(-1);
      }

#endif
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

//-------------------------------------------------------------
// finalize_device
//-------------------------------------------------------------
void finalize_device(NodeProp& nodeprop)
{
  printf("finalize_device\n");

  // flush SiTCP buffer -----------------------------------
  cltcp_drop_event(socktcp);

  udpreg_close(sockudp);
  cltcp_close(socktcp);

  printf("finalize_device end\n");
  return;
}

//-------------------------------------------------------------
// close_device
//-------------------------------------------------------------
void close_device(NodeProp& nodeprop)
{
  return;
}

//-------------------------------------------------------------
// wait_device
//-------------------------------------------------------------
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

//-------------------------------------------------------------
// read_device
//-------------------------------------------------------------
int read_device(NodeProp& nodeprop, unsigned int* data, int& len)
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      char *buf;
      int ndata;

      buf = (char *)data;
      ndata = cltcp_read(socktcp, buf, max_data_size);
      len = ndata/sizeof(unsigned int);
      if(ndata <= 0){
	// char message[400];
	// sprintf(message,
	// 	"clite::read_device(%s) (time out ?)",
	// 	tcphost
	// 	);
	// send_warning_message(message);	
	return -1;
      }

      return 0;
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
