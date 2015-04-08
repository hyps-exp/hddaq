// vme01_node: vme_v7807.cc

#include <stdio.h>
#include <stdlib.h>

#include "userdevice.h"
#include "vme_v7807.h"

//vme module list
const int RPV130_NUM = 1;
const int SMP_NUM    = 6;
RPV130_REG rpv130[RPV130_NUM] = { { 0x00008f00 } };
SMP_REG    smp[SMP_NUM]       = { { 0x10000000 }, { 0x10200000 }, { 0x10400000 },
				  { 0x10600000 }, { 0x10800000 }, { 0x10a00000 } };

//global variables
vme_bus_handle_t  bus_hdl;
vme_dma_handle_t  dma_hdl;
uint32_t         *dma_buf;

//local variables
static vme_master_handle_t mst_hdl[max_hdl_num];
static int hdl_num = 0;

void vme_open()
{
  char message[256];
  //open device
  if( vme_init(&bus_hdl) ){
    send_fatal_message("vme01: vme_init() failed");
    std::exit(-1);
  }
  if( vme_dma_buffer_create( bus_hdl, &dma_hdl, 4*DMA_BUF_LEN, 0, NULL) ){
    send_fatal_message("vme01: vme_dma_buffer_create() failed");
    std::exit(-1);
  }
  dma_buf = (uint32_t*)vme_dma_buffer_map( bus_hdl, dma_hdl, 0);
  if(!dma_buf){
    send_fatal_message("vme01: vme_dma_buffer_map() failed");
    std::exit(-1);
  }
  //### RPV130 #################################################
  {
    check_handle_number(hdl_num);
    size_t w_size = RPV130_MAP_SIZE*RPV130_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  rpv130[0].addr, RPV130_AM, w_size,
				  VME_CTL_PWEN, NULL ) ){
      send_fatal_message("vme01: RPV130: vme_master_window_create() failed");
      std::exit(-1);
    }
    void *ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num++], 0 );
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
      sprintf(message, "vme01: RPV130  [%08llx] joined", rpv130[i].addr);
      send_normal_message(message);
    }
  }
  //### SMP ###################################################
  {
    check_handle_number(hdl_num);
    size_t w_size = SMP_MAP_SIZE*SMP_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  smp[0].addr, SMP_AM, w_size,
				  VME_CTL_PWEN, NULL )){
      send_fatal_message("vme01: SMP: vme_master_window_create() failed");
      std::exit(-1);
    }
    void *ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num++], 0 );
    for(int i=0;i<SMP_NUM;i++){
      int d32    = 0x4;
      int offset32 = SMP_MAP_SIZE/d32*i;
      smp[i].cmr   = (uint32_t*)ptr +offset32 +0x180000/d32;
      smp[i].snccr = (uint32_t*)ptr +offset32 +0x180004/d32;
      smp[i].sncr  = (uint32_t*)ptr +offset32 +0x180008/d32;
      smp[i].dsr   = (uint32_t*)ptr +offset32 +0x18000c/d32;
      smp[i].enr   = (uint32_t*)ptr +offset32 +0x180010/d32;
      smp[i].bcr   = (uint32_t*)ptr +offset32 +0x180014/d32;
      sprintf(message, "vme01: SMP     [%08llx] joined", smp[i].addr);
      send_normal_message(message);
    }
  }
  return;
}

void vme_close()
{
  //unmap and release
  for(int i=0;i<hdl_num;i++){
    if( vme_master_window_unmap( bus_hdl, mst_hdl[i] ) ){
      send_fatal_message("vme01: vme_master_window_unmap() failed");
      std::exit(-1);
    }
    if( vme_master_window_release( bus_hdl, mst_hdl[i] ) ){
      send_fatal_message("vme01: vme_master_window_release() failed");      
      std::exit(-1);
    }
  }
  if ( vme_dma_buffer_unmap( bus_hdl, dma_hdl ) ){
    send_fatal_message("vme01: vme_dma_buffer_unmap() failed");
    std::exit(-1);
  }
  if ( vme_dma_buffer_release( bus_hdl, dma_hdl ) ){
    send_fatal_message("vme01: vme_dma_buffer_release() failed");
    std::exit(-1);
  }
  //close device
  if( vme_term(bus_hdl) ){
    send_fatal_message("vme01: vme_term() failed");
    std::exit(-1);
  }
  return;
}
void busy_off()
{

}
void check_handle_number(int handle_number)
{
  if(handle_number>=max_hdl_num){
    char message[256];
    sprintf(message, "vme01: too many Master Windows: %d/%d",
            handle_number, max_hdl_num);
    send_fatal_message(message);
    std::exit(-1);
  }
}

void init_vme_master_header( VME_MASTER_HEADER *vme_master_header,
			     uint64_t data_size, uint64_t nblock )
{
  vme_master_header->m_magic     = VME_MASTER_MAGIC;
  vme_master_header->m_data_size = data_size;
  vme_master_header->m_nblock    = nblock;
}

void init_vme_module_header( VME_MODULE_HEADER *vme_module_header,
			     uint64_t vme_address, uint64_t data_size )
{
  vme_module_header->m_magic       = VME_MODULE_MAGIC;
  vme_module_header->m_vme_address = vme_address;
  vme_module_header->m_data_size   = data_size;
  vme_module_header->m_n_times_read_device = 0;
  vme_module_header->m_module_type[0]      = 0;
  vme_module_header->m_module_type[1]      = 0;
  vme_module_header->m_tv_sec              = 0;
  vme_module_header->m_tv_nsec             = 0;
}
