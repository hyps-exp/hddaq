// example_node: userdevice.cc

#include "userdevice.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "camac.h"
#include "pcc.h"

#define FERA1  3
#define FERA2  6
#define SCALER  9
#define COINREG1  11
#define COINREG2  12
#define COINREG3  13
#define COINREG4  14

#define INTREG  20
#define OUTREG  21
#define CAMAC_RM 23

enum e_2nd_flag { k_init, k_accept, k_clear };
int  g_2nd_flag = k_init;

//maximum datasize by byte unit
static const int max_data_size = 4*1024*1024; 

DaqMode g_daq_mode = DM_NORMAL;

typedef struct VmeMasterHeader {
    unsigned int magic;
    unsigned int data_size;
    unsigned int nblock;
} VmeMasterHeader;

typedef struct VmeModuleHeader {
    unsigned long long magic;
    unsigned long long vme_address;
    unsigned long long data_size;
    unsigned long long n_times_read_device;
    unsigned long long module_type[2];
    unsigned long long tv_sec;
    unsigned long long tv_nsec;
} VmeModuleHeader;

static void init_vme_master_header(VmeMasterHeader *header, unsigned long long data_size, unsigned long long nblock);
static void init_vme_module_header(VmeModuleHeader *header, unsigned int address, unsigned int data_size);
static int write_vme_module_data(unsigned int *dst, const unsigned int *src, unsigned int address, unsigned int data_size);
static int read_fera(unsigned int *data, int address);
static int read_coinreg(unsigned int *data, int address);
static int read_camac_rm(unsigned int *data, int address);

int fd;

int get_maxdatasize()
{
  return max_data_size;
}

void open_device(NodeProp& nodeprop)
{
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  char message[200];

  //called when Front-end program launches
  printf("open device\n");

  int data, q, x;
  int status, daq_status;
  int pedestal;
  int status_reg;

  if((fd = cam_open( )) == -1) {
    printf("cam_open error\n");

    sprintf(message, "ccnet01 : cam_open error");
    send_fatal_message(message);

    return;
  }

  //printf("fd = %d\n", fd);

  // Z
  if ( (status = cam_single_cc(fd, 25, 0, 17, &data, &q, &x)) == -1 ) {
    printf("Initialization error\n");

    sprintf(message, "ccnet01 : CAMAC Initialization error");
    send_error_message(message);

    return;
  }

  // C
  if ( (status = cam_single_cc(fd, 25, 0, 16, &data, &q, &x)) == -1 ) {
    printf("Clear error\n");

    sprintf(message, "ccnet01 : CAMAC Clear error");
    send_error_message(message);

    return;
  }

  // remove Inhibit
  if ( (status = cam_single_cc(fd, 25, 0, 24, &data, &q, &x)) == -1 ) {
    printf("Remove Inhibit error\n");

    sprintf(message, "ccnet01 : CAMAC Inhibit remove error");
    send_error_message(message);
    return;
  }

  if ( (status = cam_single_cc(fd, INTREG, 0, 24, &data, &q, &x)) < 0 ) { 
    printf("cam_single_cc(fd, INTREG, 0, 24...) error...\n");

    return;
  }

  // clear event counter
  status = cam_single_daq( fd, DAQEXE_CTRL_CLRCNT, &data, &daq_status);
  if( status < 0 ) {
    printf("cam_single_daq: DAQEXE_CTRL_CLRCNT error...\n");
    return;
  }

  // clear busy out
  status = cam_single_daq( fd, DAQEXE_CTRL_CLRBSY, &data, &daq_status);
  if( status < 0 ) {
    printf("cam_single_daq: DAQEXE_CTRL_CLRBSY error...\n");

    sprintf(message, "ccnet01 : DAQEXE_CTRL_CLRBSY error...");
    send_error_message(message);
    return;
  }

  // clear scaler
  status = cam_single_cc(fd, SCALER, 0, 9, &data, &q, &x);
  status = cam_single_cc(fd, SCALER, 1, 9, &data, &q, &x);
  status = cam_single_cc(fd, SCALER, 2, 9, &data, &q, &x);
  status = cam_single_cc(fd, SCALER, 3, 9, &data, &q, &x);


  // Write Pedestal memory
  pedestal=40;
  for (int i=0; i<16; i++) {
    status = cam_single_cc(fd, FERA1, i, 17, &pedestal, &q, &x);
    status = cam_single_cc(fd, FERA2, i, 17, &pedestal, &q, &x);
  }
  //pedestal=0;
  //status = cam_single_cc(fd, FERA1, 0, 17, &pedestal, &q, &x);

  /* FERA1 Status register */
  //status_reg = 0x0000;
  status_reg = 0x3f00; // ECL port ENable
  // set status register
  status = cam_single_cc(fd, FERA1, 0, 16, &status_reg, &q, &x);
  // read status register
  status = cam_single_cc(fd, FERA1, 0, 0, &data, &q, &x);
  printf("FERA1 Status register: %x\n", data);
  if (data != status_reg) {
    sprintf(message, "ccnet01 : FERA1 Status register setting error : %x", data);
    send_fatal_message(message);
  }

  /* FERA2 Status register */
  //status_reg = 0x0000;
  status_reg = 0x3f01; // ECL port ENable // vsn 1
  // set status register
  status = cam_single_cc(fd, FERA2, 0, 16, &status_reg, &q, &x);
  // read status register
  status = cam_single_cc(fd, FERA2, 0, 0, &data, &q, &x);
  printf("FERA2 Status register: %x\n", data);
  if (data != status_reg) {
    sprintf(message, "ccnet01 : FERA2 Status register setting error : %x", data);
    send_fatal_message(message);
  }
  
  sleep(10);

  return;
}

void init_device(NodeProp& nodeprop)
{
  
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  //called when START command comes
  int data, q, x;
  int status;

  status = cam_enable_trig(fd);
  if ( status < 0 ) {
    printf("cam_enable_trig : error...\n");
    return;
  }

  status = cam_single_cc(fd, FERA1, 0, 9, &data, &q, &x);    
  status = cam_single_cc(fd, FERA2, 0, 9, &data, &q, &x);    
  status = cam_single_cc(fd, COINREG1, 0, 9, &data, &q, &x);    
  status = cam_single_cc(fd, COINREG2, 0, 9, &data, &q, &x);    
  status = cam_single_cc(fd, COINREG3, 0, 9, &data, &q, &x);    
  status = cam_single_cc(fd, COINREG4, 0, 9, &data, &q, &x);    
  status = cam_single_cc(fd, OUTREG, 0, 9, &data, &q, &x);
  
  // enable interrupt at the controller
  if( (status = cam_enable_lam(fd, 0xFFFFFF)) < 0 ) {
    printf("cam_enable_lam error...\n");
    return;
  }

  // enable interrupt at Interrupt register
  if( (status = cam_single_cc(fd, INTREG, 0, 26, &data, &q, &x)) < 0 ) {
    printf("cam_single_cc(fd, INTREG, 0, 26...) error...\n");
    return;
  }

  switch(g_daq_mode){
  case DM_NORMAL:
    {
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
  return;
}

void close_device(NodeProp& nodeprop)
{
  cam_close( fd );

  return;
}


int wait_device(NodeProp& nodeprop)
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{

  //printf("ccnet01: wait_device() \n");
  int data, q, x;  
  int status;
  int n_try = 0;


  // clear interrupt at Interrupt register
  status = cam_single_cc(fd, INTREG, 0, 9, &data, &q, &x);
  if( status < 0) {
    printf("cam_single_cc(fd, INTREG, 0, 9...) error...\n");
    return(0);
  }
  
  // release Latch signal
  data = 0xffff;
  status = cam_single_cc(fd, OUTREG, 0, 17, &data, &q, &x);    

  //status = cam_wait_lam(fd, &lam_pattern, TIMEOUT);
  data=0;

  while (1) {
    cam_single_cc(fd, INTREG, 0, 0, &data, &q, &x);
    //printf("waiting trigger\n");
    if (data != 0) {
      //printf("%d \n", n_try);
      return 0;
    }
    n_try++;
    //usleep(1);	

    if(n_try >= 500000) {
      // TimeOut
      printf("ccnet01: time out \n");
      return -1;
    }
      
  }  
  
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
  
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int event_number = nodeprop.getEventNumber();

      VmeMasterHeader header;
      unsigned int *pheader;
      int module_data_size;
      int total_module_data_size = 0;

      //printf("read_device\n");

      len = 0;
      pheader = data;
      data += sizeof(header) / sizeof(unsigned int);
      len  += sizeof(header) / sizeof(unsigned int);

      module_data_size = read_camac_rm(data, CAMAC_RM);
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      //if( (event_number%10) != 0 && g_2nd_flag == k_accept ){
      /*
      if( (event_number%10) != 0 ){
      	init_vme_master_header(&header, total_module_data_size, 0);
      	memcpy(pheader, &header, sizeof(header));
	return 0;
      }
      */

      module_data_size = read_fera(data, FERA1);
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      module_data_size = read_fera(data, FERA2);  
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      module_data_size = read_coinreg(data, COINREG1);  
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      module_data_size = read_coinreg(data, COINREG2);  
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      module_data_size = read_coinreg(data, COINREG3);
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      module_data_size = read_coinreg(data, COINREG4);
      len  += module_data_size;
      data += module_data_size;
      total_module_data_size += module_data_size;

      init_vme_master_header(&header, total_module_data_size, 0);
      memcpy(pheader, &header, sizeof(header));

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


static void init_vme_master_header(VmeMasterHeader *header, unsigned long long data_size, unsigned long long nblock)
{
    header->magic = 0x00564d45;
    header->data_size = data_size + sizeof(VmeMasterHeader) / sizeof(unsigned int);
    header->nblock = nblock;
}


static void init_vme_module_header(VmeModuleHeader *header, unsigned int address, unsigned int data_size)
{
    header->magic = 0x766d65626f617264ULL;
    header->vme_address = address;
    header->data_size = data_size + sizeof(VmeModuleHeader) / sizeof(unsigned int);
    header->n_times_read_device = 0;
    header->module_type[0] = 0;
    header->module_type[1] = 0;
    header->tv_sec = 0;
    header->tv_nsec = 0;
}

static int write_vme_module_data(unsigned int *dst, const unsigned int *src, unsigned int address, unsigned int data_size)
{
    VmeModuleHeader header;

    init_vme_module_header(&header, address, data_size);
    memcpy(dst, &header, sizeof(VmeModuleHeader));
    dst += sizeof(VmeModuleHeader) / 4;

    memcpy(dst, src, data_size * 4);
    dst += data_size;

    return sizeof(VmeModuleHeader) / 4 + data_size;
}



static int read_fera(unsigned int *data, int address)
{
    unsigned long Address = address;
    unsigned int Buffer[17];
    //int BufferLen = sizeof(Buffer) / sizeof(Buffer[0]);
    int data_size = 0;
    int value, q, x;
    int status;

    for (int i=0; i<17; i++) {
      status = cam_single_cc(fd, address, i, 2, &value, &q, &x);
      if (q==0) break;

      Buffer[i]=value;
      data_size++;

      /*
      if (q==1)
	printf("FERA1 %d, Q=%d, X=%d, data = %x\n", i, q, x, value);
      */
      //printf("%d ", value);
    }
    
    if (data_size == 0) {
      Buffer[0] = 0x8000;
      data_size = 1;
    }

    return write_vme_module_data(data, Buffer, Address, data_size);
}

static int read_coinreg(unsigned int *data, int address)
{
    unsigned long Address = address;
    unsigned int Buffer[17];
    //int BufferLen = sizeof(Buffer) / sizeof(Buffer[0]);
    int data_size = 0;
    int value, q, x;
    int status;

    status = cam_single_cc(fd, address, 0, 2, &value, &q, &x);
    Buffer[0] = value;
    data_size = 1;
    //    printf("COINREG %d, Q=%d, X=%d, data = %x\n", address, q, x, value);
    //    if(status < 0 || q!=1) printf("Read error coinreg : %d, q=%d\n", address, q);
    return write_vme_module_data(data, Buffer, Address, data_size);
}

static int read_camac_rm(unsigned int *data, int address)
{
    unsigned long Address = address;
    unsigned int Buffer[17];
    //int BufferLen = sizeof(Buffer) / sizeof(Buffer[0]);
    int data_size = 0;
    int value, q, x;
    int status;

    // Event Number
    status = cam_single_cc(fd, address, 0, 0, &value, &q, &x);
    Buffer[0] = value;

    // Spill Number
    status = cam_single_cc(fd, address, 1, 0, &value, &q, &x);
    Buffer[1] = value;

    // input register
    status = cam_single_cc(fd, address, 4, 0, &value, &q, &x);
    Buffer[2] = value;
    g_2nd_flag = value;

    data_size=3;
    //printf("RM : Event %x, Spill %x, Input register %x\n", Buffer[0], Buffer[1], Buffer[2]);

    return write_vme_module_data(data, Buffer, Address, data_size);
}
