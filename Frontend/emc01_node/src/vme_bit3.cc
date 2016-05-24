// emc01_node: vme_bit3.cc

#include <cstdio>
#include <cstdlib>

#include "vme_bit3.h"

//vme module list
const int VME_RM_NUM = 1;
const int EMC_NUM    = 1;
VME_RM_REG vme_rm[VME_RM_NUM] = { { 0xff000000 } };
EMC_REG    emc[EMC_NUM]       = { { 0xe3c00000 } };

//global variables

//local variables

void vme_open()
{
  char message[256];
  
  //### VME_RM ########################################################
  for(int i=0; i<VME_RM_NUM; i++){
    vme_rm[i].map_hdl = vme_mapopen( VME_RM_AM, vme_rm[i].addr, VME_RM_MAP_SIZE );
    if( vme_rm[i].map_hdl == NULL ){
      sprintf(message, "emc01: VME_RM[%08llx] vme_mapopen() failed", vme_rm[i].addr);
      send_fatal_message(message);
      std::exit(EXIT_FAILURE);
    }
    vme_rm[i].event   = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x0);
    vme_rm[i].spill   = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x4);
    vme_rm[i].serial  = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x8);
    vme_rm[i].input   = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x10);
    vme_rm[i].reset   = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x14);
    vme_rm[i].level   = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x18);
    vme_rm[i].pulse   = (uint32_t*) vme_word32(vme_rm[i].map_hdl, 0x1c);
    sprintf(message, "emc01: VME_RM  [%08llx] joined", vme_rm[i].addr);
    send_normal_message(message);
  }
  //### EMC ########################################################
  for(int i=0; i<EMC_NUM; i++){
    sprintf(message, "emc01: EMC     [%08llx] joined", emc[i].addr);
    send_normal_message(message);
  }

  return;
}

void vme_close()
{
  vme_mapclose( vme_rm[0].map_hdl );

  return;
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
