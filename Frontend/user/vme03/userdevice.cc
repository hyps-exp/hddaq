/*
 *   vme03: userdevice.cc
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "vme_v7807.h"

#define MAX_DATASIZE 4*1024*1024;

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

  ////////// v830 setting
  for(int i=0;i<V830_NUM;i++){
    *(v830[i].reset) = 0x0;
    *(v830[i].ch_enable) = 0x0; // for MEB, v830 only
    // acq mode 00: disabled (default), 01: external or from VME, 10: periodical
    *(v830[i].clr) = 0x01;
  }

  sleep(10);

  return 0;
}

//========================================================================================
int init_device()
{
  //called when START command comes

  //Busy off
  *(rpv130[0].csr1) = 0x1;
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
  //never called (exit when fe_exit command comes)
  vme_close();
  return 0;
}


//========================================================================================
int wait_device()
{
  //printf("vme03: wait_device() \n");
  trig_flag = 0;

  ////////// Polling
  int reg = 0;
  for(int i=0;i<2000000;i++){
    reg = *(rpv130[0].rsff);
    if( (reg>>0)&0x1 ){
      trig_flag = 1;
      return 0;
    }
    else if( (reg>>1)&0x1 ){
      trig_flag = 2;
      return 0;
    }
  }

  // TimeOut
  printf("vme03: time out \n");
  return -1;
}

//========================================================================================
int read_device(unsigned int *data, int *len, int *event_num, int run_num)
{
  
  //for spill end trigger
  if(trig_flag==2){
    for(int i=0;i<V830_NUM;i++){
      *(v830[i].trig) = 0x0;
      int ret = vme_dma_read( bus_hdl, dma_hdl, 0, (uint64_t)v830[i].addr+0x1000,
      			      V830_AM, 4*32, 0 );
      if(ret!=0){
      	perror("v830: vme_dma_read() failed\n");
      }else{
	for(int k=0;k<32;k++) printf("v830: data[%d][%d]=%d\n", i,k,dma_buf[k]);
	printf("\n\n");
      }
    }
    
    //Busy off
    *(rpv130[0].csr1) = 0x1;
    *(rpv130[0].pulse) = 0x1;
    return 2;
  }

  //for normal trigger
  int ndata   = 0;
  ndata += VME_MASTER_HSIZE;
  
  ////////// vme-rm
  {
    for(int i=0;i<VME_RM_NUM;i++){
      int vme_module_header_start = ndata;
      ndata += VME_MODULE_HSIZE;
      data[ndata++] = *(vme_rm[i].event);
      data[ndata++] = *(vme_rm[i].spill);
      data[ndata++] = *(vme_rm[i].serial);
      data[ndata++] = 0x0; // spill_end_flag
      VME_MODULE_HEADER vme_module_header;
      vme_module_header.m_magic       = VME_MODULE_MAGIC;
      vme_module_header.m_vme_address = vme_rm[i].addr;
      vme_module_header.m_data_size   = ndata - vme_module_header_start;
      memcpy( &data[vme_module_header_start],
	      &vme_module_header, VME_MODULE_HSIZE*4 );
    }
  }

  ////////// v830
  {
    for(int i=0;i<V830_NUM;i++){
      int vme_module_header_start = ndata;
      ndata += VME_MODULE_HSIZE;
      
      int ret = vme_dma_read( bus_hdl, dma_hdl, 0, v830[i].addr+0x1000,
      			      V830_AM, 4*32, 0 );
      if(ret!=0){
      	printf("v830[%d]: vme_dma_read() failed\n",i);
      }else{
      	memcpy( &data[ndata], dma_buf, 4*32 );
      	ndata += 32;
      }
      
      VME_MODULE_HEADER vme_module_header;
      vme_module_header.m_magic       = VME_MODULE_MAGIC;
      vme_module_header.m_vme_address = v830[i].addr;
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

  //Busy off
  *(rpv130[0].csr1) = 0x1;
  *(rpv130[0].pulse) = 0x1;
  return 1;
}
