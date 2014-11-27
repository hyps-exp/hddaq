/*
 *   vme01: userdevice.cc
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>

#include "vme_v7807.h"

#define MAX_DATASIZE 4*1024*1024;

uint32_t t;
int trig_flag = 0;

//========================================================================================
int get_maxdatasize()
{
  /* return maximum datasize by byte unit*/
  return MAX_DATASIZE;
}

//========================================================================================
int open_device()
{
  //called when Front-end program launches
  
  vme_open();

  ////////// smp setting
  for(int i=0;i<SMP_NUM;i++){
    *(smp[i].cmr)   = 0x40;// SMP reset
    usleep(1000);
    *(smp[i].snccr) = 0x02;// cnt clr
    *(smp[i].bcr)   = 0x23;// hardware switch eable
    *(smp[i].cmr)   = 0x00;// set cmr
  }
  
  return 0;
}

//========================================================================================
int init_device()
{
  //called when START command comes
  
  //Busy off
  *(rpv130[0].csr1)  = 0x1;
  *(rpv130[0].pulse) = 0x1;
  return 0;
}

//========================================================================================
int finalize_device()
{
  //called when STOP command comes
  return 0;
}

//========================================================================================
int close_device()
{
  //called when END command comes
  
  vme_close();
  return 0;
}


//========================================================================================
int wait_device()
{
  //printf("vme01: wait_device() \n");
  trig_flag = 0;

  ////////// Polling
  int reg = 0;
  for(int i=0;i<2000000;i++){
    reg = *(rpv130[0].rsff);
    if( (reg>>0)&0x1 ){
      trig_flag = 1;
      t = (uint32_t)time(0);

      //SMP switch
      *(rpv130[0].pulse) = 0x80;

      for(int i=0;i<SMP_NUM;i++){
	int status = *(smp[i].cmr);
	printf("smp[%d]: status %x\n",i,status);
      }
      
      //Busy off
      *(rpv130[0].csr1)  = 0x1;
      *(rpv130[0].pulse) = 0x1;
      
      return 0;
    }
  }
  // TimeOut
  printf("vme01: time out \n");
  return -1;
}

//========================================================================================
int read_device(unsigned int *data, int *len, int *event_num, int run_num)
{
  printf("vme01: read_device() RUN#%d\n",run_num);
  sleep(1);
  *len = 10;
  return 1;

  int ndata   = 0;
  ndata += VME_MASTER_HSIZE;
  
  ////////// Time Stamp
  {
    int vme_module_header_start = ndata;
    ndata += VME_MODULE_HSIZE;
    data[ndata++] = t;
    VME_MODULE_HEADER vme_module_header;
    vme_module_header.m_magic       = VME_MODULE_MAGIC;
    vme_module_header.m_vme_address = 0x12345678;
    vme_module_header.m_data_size   = ndata - vme_module_header_start;
    memcpy( &data[vme_module_header_start],
	    &vme_module_header, VME_MODULE_HSIZE*4 );
  } 
 
  ////////// smp
  {
    for(int i=0;i<SMP_NUM;i++){
      int vme_module_header_start = ndata;
      ndata += VME_MODULE_HSIZE;
      
      int evnum = (*(smp[i].enr))&0xffff;
      int dsize = (*(smp[i].dsr))&0x7fffffff;

      if(evnum!=1){
	printf("smp[%d]: invalid evnum: %d \n", i, evnum);
	continue;
      }

      data[ndata++] = 0x0;
      int ret = vme_dma_read( bus_hdl, dma_hdl, 0, smp[i].addr,
			      SMP_AM, dsize, 0 );
      if(ret!=0){
      	printf("smp[%d]: vme_dma_read() failed\n",i);
      }else{
      	memcpy( &data[ndata], dma_buf, dsize );
      	ndata += dsize/4;
      }
      
      VME_MODULE_HEADER vme_module_header;
      vme_module_header.m_magic       = VME_MODULE_MAGIC;
      vme_module_header.m_vme_address = smp[i].addr;
      vme_module_header.m_data_size   = ndata - vme_module_header_start;
      memcpy( &data[vme_module_header_start],
	      &vme_module_header, VME_MODULE_HSIZE*4 );
    }
  }

  VME_MASTER_HEADER vme_master_header;
  vme_master_header.m_magic     = VME_MASTER_MAGIC;
  vme_master_header.m_data_size = ndata;
  vme_master_header.m_nblock    = 0;
  memcpy( &data[0], &vme_master_header, VME_MASTER_HSIZE*4 );

  *len = ndata;
  return 1;
}

