#ifndef DETECTOR_ID_H
#define DETECTOR_ID_H

#include "gef/gefcmn_vme.h"
#include "stdint.h"

const uint32_t header        = 0x48454144;
const uint32_t footer        = 0x464F4F54;
const uint32_t fadc_header   = 0x46414443;
const uint32_t fadc_footer   = 0x43444146;
const uint32_t tdc_header    = 0x00544443;
const uint32_t tdc_footer    = 0x43445400;
const uint32_t ssd_header    = 0x00535344;
const uint32_t ssd_footer    = 0x44535300;
const uint32_t scaler_header = 0x53434c52;
const uint32_t scaler_footer = 0x524c4353;

//### For VME node ######################                                  
#define VME_MASTER_MAGIC 0x00564d45
#define VME_MASTER_HSIZE 3
struct VME_MASTER_HEADER{
  GEF_UINT32 m_magic;
  GEF_UINT32 m_data_size;
  GEF_UINT32 m_nblock;
};

#define VME_MODULE_MAGIC 0x766d65626f617264ULL
#define VME_MODULE_HSIZE 16
struct VME_MODULE_HEADER{
  uint64_t m_magic;
  uint64_t m_vme_address;
  uint64_t m_data_size;
  uint64_t m_n_times_read_device;
  uint64_t m_module_type[2];
  uint64_t m_tv_sec;
  uint64_t m_tv_nsec;
};

void init_vme_master_header(VME_MASTER_HEADER *vme_master_header,
                            uint64_t data_size, uint64_t nblock);
void init_vme_module_header(VME_MODULE_HEADER *vme_module_header,
                            uint64_t vme_address, uint64_t data_size);
#endif
