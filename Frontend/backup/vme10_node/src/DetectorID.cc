#include "DetectorID.hh"

void init_vme_master_header(VME_MASTER_HEADER *vme_master_header,
                            uint64_t data_size, uint64_t nblock)
{
  vme_master_header->m_magic     = VME_MASTER_MAGIC;
  vme_master_header->m_data_size = data_size;
  vme_master_header->m_nblock    = nblock;
}                                                                                               

void init_vme_module_header(VME_MODULE_HEADER *vme_module_header,
                            uint64_t vme_address, uint64_t data_size)
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
