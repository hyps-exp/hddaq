// vme03_node: vme_v7807.cc

#include <stdio.h>
#include <stdlib.h>

#include "userdevice.h"
#include "vme_v7807.h"

//vme module list
const int VME_RM_NUM = 1;
const int RPV130_NUM = 1;
const int V830_NUM   = 3;
const int V775_NUM   = 1;
VME_RM_REG vme_rm[VME_RM_NUM] = { { 0xfc000000 } };
RPV130_REG rpv130[RPV130_NUM] = { { 0x00008ff0 } };
V830_REG   v830[V830_NUM]     = { { 0x20000000 }, { 0x20010000 }, { 0x20020000 } };
V775_REG   v775[V775_NUM]     = { { 0xBD020000 } };

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
    send_fatal_message("vme03: vme_init() failed");
    std::exit(-1);
  }
  if( vme_dma_buffer_create( bus_hdl, &dma_hdl, 4*DMA_BUF_LEN, 0, NULL) ){
    send_fatal_message("vme03: vme_dma_buffer_create() failed");
    std::exit(-1);
  }

  dma_buf = (uint32_t*)vme_dma_buffer_map( bus_hdl, dma_hdl, 0);
  if(!dma_buf){
    send_fatal_message("vme03: vme_dma_buffer_map() failed");
    std::exit(-1);
  }
  //### VME_RM #################################################
  {
    check_handle_number(hdl_num);
    size_t w_size = VME_RM_MAP_SIZE*VME_RM_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  vme_rm[0].addr, VME_RM_AM, w_size,
				  VME_CTL_PWEN, NULL ) ){
      send_fatal_message("vme03: VME_RM: vme_master_window_create() failed");
      std::exit(-1);
    }
    void *ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num++], 0 );
    for(int i=0;i<VME_RM_NUM;i++){
      int d32    = 0x4;
      int offset = VME_RM_MAP_SIZE/d32*i;
      vme_rm[i].event  = (uint32_t*)ptr +offset +0x0/d32;
      vme_rm[i].spill  = (uint32_t*)ptr +offset +0x4/d32;
      vme_rm[i].serial = (uint32_t*)ptr +offset +0x8/d32;
      sprintf(message, "vme03: VME_RM  [%08llx] joined", vme_rm[i].addr);
      send_normal_message(message);
    }
  }
  //### RPV130 #################################################
  {
    check_handle_number(hdl_num);
    size_t w_size = RPV130_MAP_SIZE*RPV130_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  rpv130[0].addr, RPV130_AM, w_size,
				  VME_CTL_PWEN, NULL ) ){
      send_fatal_message("vme03: RPV130: vme_master_window_create() failed");
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
      sprintf(message, "vme03: RPV130  [%08llx] joined", rpv130[i].addr);
      send_normal_message(message);
    }
  }
  //### V830 ###################################################
  {
    check_handle_number(hdl_num);
    size_t w_size = V830_MAP_SIZE*V830_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  v830[0].addr, V830_AM, w_size,
				  VME_CTL_PWEN, NULL )){
      send_fatal_message("vme03: V830: vme_master_window_create() failed");
      std::exit(-1);
    }
    void *ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num++], 0 );
    for(int i=0;i<V830_NUM;i++){
      int d16    = 0x2;
      int d32    = 0x4;
      int offset16 = V830_MAP_SIZE/d16*i;
      int offset32 = V830_MAP_SIZE/d32*i;
      for(int ch=0;ch<32;ch++){
	v830[i].counter[ch] = (uint32_t*)ptr +offset32 +0x1000/d32 +0x4/d32*ch;
      }
      v830[i].enable = (uint32_t*)ptr +offset32 +0x1100/d32;
      v830[i].clr    = (uint16_t*)ptr +offset16 +0x1108/d16;
      v830[i].str    = (uint16_t*)ptr +offset16 +0x110E/d16;
      v830[i].reset  = (uint16_t*)ptr +offset16 +0x1120/d16;
      v830[i].clear  = (uint16_t*)ptr +offset16 +0x1122/d16;
      v830[i].trig   = (uint16_t*)ptr +offset16 +0x1124/d16;
      sprintf(message, "vme03: V830    [%08llx] joined", v830[i].addr);
      send_normal_message(message);
    }
  }
  //### V775 ###################################################
  {
    check_handle_number(hdl_num);
    size_t w_size = V775_MAP_SIZE*V775_NUM;
    if( vme_master_window_create( bus_hdl, &(mst_hdl[hdl_num]),
				  v775[0].addr, V775_AM, w_size,
				  VME_CTL_PWEN, NULL )){
      send_fatal_message("vme03: V775: vme_master_window_create() failed");
      std::exit(-1);
    }
    void *ptr = (void*)vme_master_window_map( bus_hdl, mst_hdl[hdl_num++], 0 );
    for(int i=0;i<V775_NUM;i++){
      int d16    = 0x2;
      int d32    = 0x4;
      int offset16 = V775_MAP_SIZE/d16*i;
      int offset32 = V775_MAP_SIZE/d32*i;
      v775[i].data_buf     = (uint32_t*)ptr +offset32 +0x0000/d32;
      v775[i].bitset1      = (uint16_t*)ptr +offset16 +0x1006/d16;
      v775[i].bitclr1      = (uint16_t*)ptr +offset16 +0x1008/d16;
      v775[i].str1         = (uint16_t*)ptr +offset16 +0x100E/d16;
      v775[i].bitset2      = (uint16_t*)ptr +offset16 +0x1032/d16;
      v775[i].bitclr2      = (uint16_t*)ptr +offset16 +0x1034/d16;
      v775[i].range        = (uint16_t*)ptr +offset16 +0x1060/d16;
      sprintf(message, "vme03: V775    [%08llx] joined", v775[i].addr);
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
      send_fatal_message("vme03: vme_master_window_unmap() failed");
      std::exit(-1);
    }
    if( vme_master_window_release( bus_hdl, mst_hdl[i] ) ){
      send_fatal_message("vme03: vme_master_window_release() failed");
      std::exit(-1);
    }
  }
  if( vme_dma_buffer_unmap( bus_hdl, dma_hdl ) ){
    send_fatal_message("vme03: vme_dma_buffer_unmap() failed");
    std::exit(-1);    
  }
  if( vme_dma_buffer_release( bus_hdl, dma_hdl ) ){
    send_fatal_message("vme03: vme_dma_buffer_release() failed");
    std::exit(-1);
  }
  //close device
  if(vme_term(bus_hdl)){
    send_fatal_message("vme03: vme_term() failed");
    std::exit(-1);
  }
  return;
}

void check_handle_number(int handle_number)
{
  if(handle_number>=max_hdl_num){
    char message[256];
    sprintf(message, "vme03: too many Master Windows: %d/%d",
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
