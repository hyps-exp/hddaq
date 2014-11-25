#include <stdio.h>
#include <stdlib.h>

#include "vme_v7807.h"

//vme module list
vme_bus_handle_t bus_hdl;
vme_dma_handle_t dma_hdl;
uint32_t *dma_buf;

const int VME_RM_NUM=1;
VME_RM_REG vme_rm[VME_RM_NUM] = { { 0xfc000000 } };

const int RPV130_NUM=1;
RPV130_REG rpv130[RPV130_NUM] = { { 0x00008ff0 } };

const int V830_NUM=2;
V830_REG v830[V830_NUM] = { { 0x20000000 }, { 0x20010000 } };

//local variables
static vme_master_handle_t mst_hdl[8];
static int hdl_num = 0;

void vme_open()
{
  //open device
  if( vme_init(&bus_hdl) ){
    perror("#E vme_init() failed \n");
    exit(-1);
  }
  if( vme_dma_buffer_create( bus_hdl, &dma_hdl, 4*100, 0, NULL) ){
    perror("#E vme_dma_buffer_create() failed \n");
    exit(-1);
  }

  dma_buf = (uint32_t*)vme_dma_buffer_map( bus_hdl, dma_hdl, 0);
  if(!dma_buf){
    perror("#E vme_dma_buffer_map() failed \n");
    exit(-1);
  }

  //### VME_RM #################################################
  {
    if(hdl_num>7){
      perror("#E Too many Master Windows!! \n");
      exit(-1);
    }
    void *ptr;
    size_t w_size = VME_RM_MAP_SIZE*VME_RM_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  vme_rm[0].addr, VME_RM_AM, w_size,
				  VME_CTL_PWEN, NULL ) ){
      printf("#E vme_master_window_create() failed %d \n", hdl_num);      
    }
    ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num], 0 );
    hdl_num++;
    for(int i=0;i<VME_RM_NUM;i++){
      int d32    = 0x4;
      int offset = VME_RM_MAP_SIZE/d32*i;
      vme_rm[i].event  = (uint32_t*)ptr +offset +0x0/d32;
      vme_rm[i].spill  = (uint32_t*)ptr +offset +0x4/d32;
      vme_rm[i].serial = (uint32_t*)ptr +offset +0x8/d32;
    }
  }

  //### RPV130 #################################################
  {
    if(hdl_num>7){
      perror("#E Too many Master Windows!! \n");
      exit(-1);
    }
    void *ptr;
    size_t w_size = RPV130_MAP_SIZE*RPV130_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  rpv130[0].addr, RPV130_AM, w_size,
				  VME_CTL_PWEN, NULL ) ){
      perror("#E RPV130: vme_master_window_create() failed \n");
    }
    ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num], 0 );
    hdl_num++;
    for(int i=0;i<RPV130_NUM;i++){
      int d16    = 0x2;
      int offset = RPV130_MAP_SIZE/d16*i;
      rpv130[i].latch1  = (uint16_t*)ptr +offset +0x0/d16;
      rpv130[i].latch2  = (uint16_t*)ptr +offset +0x2/d16;
      rpv130[i].rsff    = (uint16_t*)ptr +offset +0x4/d16;
      rpv130[i].through = (uint16_t*)ptr +offset +0x6/d16;
      rpv130[i].pulse   = (uint16_t*)ptr +offset +0x8/d16;
      rpv130[i].level   = (uint16_t*)ptr +offset +0xA/d16;
      rpv130[i].csr1    = (uint16_t*)ptr +offset +0xC/d16;
      rpv130[i].csr2    = (uint16_t*)ptr +offset +0xE/d16;
    }
  }

  //### V830 ###################################################
  {
    if(hdl_num>7){
      perror("Too many Master Windows!!\n");
      exit(-1);
    }
    void *ptr;
    size_t w_size = V830_MAP_SIZE*V830_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  v830[0].addr, V830_AM, w_size,
				  VME_CTL_PWEN, NULL )){
      perror("#E V830: vme_master_window_create() failed %d \n");
    }
    ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num], 0 );

    hdl_num++;
    for(int i=0;i<V830_NUM;i++){
      int d16    = 0x2;
      int d32    = 0x4;
      int offset16 = V830_MAP_SIZE/d16*i;
      int offset32 = V830_MAP_SIZE/d32*i;
      for(int ch=0;ch<32;ch++){
	v830[i].counter[ch] = (uint32_t*)ptr +offset32 +0x1000/d32 +0x4/d32*ch;
      }
      v830[i].ch_enable    = (uint32_t*)ptr +offset32 +0x1100/d32;
      v830[i].clr          = (uint16_t*)ptr +offset16 +0x1108/d16;
      v830[i].str          = (uint16_t*)ptr +offset16 +0x110E/d16;
      v830[i].reset        = (uint16_t*)ptr +offset16 +0x1120/d16;
      v830[i].clear        = (uint16_t*)ptr +offset16 +0x1122/d16;
      v830[i].trig         = (uint16_t*)ptr +offset16 +0x1124/d16;
    }
  }
  
  return;
}

void vme_close()
{
  //unmap and release
  for(int i=0;i<hdl_num;i++){
    vme_master_window_unmap( bus_hdl, mst_hdl[i] );
    vme_master_window_release( bus_hdl, mst_hdl[i] );
  }
  vme_dma_buffer_unmap( bus_hdl, dma_hdl );
  vme_dma_buffer_release( bus_hdl, dma_hdl );
  //close device
  if(vme_term(bus_hdl)){
    perror("vme_term() failed\n");
    exit(-1);
  }
  return;
}
