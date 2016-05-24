// emc01_node: vme_bit3.h

#include <stdint.h>
#include <iostream>
#include <bit3/vmeslib.h>
#include <bit3/vmeslib_internals.h>

#include "MessageHelper.h"

#ifndef VME_BIT3_H_
#define VME_BIT3_H_

#define DMA_BUF_LEN 1000

static const int max_hdl_num = 8;

void vme_open();
void vme_close(); 

/*  base address setting for multi modules
if,
base_address = 0x12340000 (first module)
map_offset   = 0x00010000 (LSB of address space)

then,
module[0]:base_address = 0x12340000
module[1]:base_address = 0x12350000 (=module[0]+map_offset)
module[2]:base_address = 0x12360000 (=module[1]+map_offset)
:
(Module base address should be set in serial order by map_offset increment.)
*/

//### VME_RM #############################
struct VME_RM_REG {
  uint64_t addr;
  VMEMAP   *map_hdl;
  volatile uint32_t *event;
  volatile uint32_t *spill;
  volatile uint32_t *serial;
  volatile uint32_t *input;
  volatile uint32_t *reset;
  volatile uint32_t *level;
  volatile uint32_t *pulse;
};
static const off_t  VME_RM_AM       = VME_A32D32;
static const size_t VME_RM_MAP_SIZE = 0x10000;
extern const int    VME_RM_NUM;
extern struct VME_RM_REG vme_rm[];

//### EMC ################################
struct EMC_REG
{
  uint64_t addr;
  uint32_t serial;
  uint32_t xpos;
  uint32_t ypos;
  uint32_t state;
  uint32_t utime;
  uint32_t ltime;
};
extern const  int     EMC_NUM;
extern struct EMC_REG emc[];

static const std::string emc_file = "/tmp/daq.txt";
static const std::string emc_param_magic = "fin";
enum e_emc_param {
  k_serial, k_xpos, k_ypos, k_state, k_time, k_magic,
  k_emc_param_size
};
static const uint32_t x_offset = 500000 - 303300;
static const uint32_t y_offset = 500000 + 164000;

static const uint32_t k_data_mask = 0xfffffff;
static const uint32_t k_word_type_shift = 28;
static const uint32_t k_word_type_mask  = 0xf;
static const uint32_t k_header_magic = 0xa;
static const uint32_t k_serial_magic = 0x0;
static const uint32_t k_xpos_magic   = 0x1;
static const uint32_t k_ypos_magic   = 0x2;
static const uint32_t k_statte_magic = 0x3;
static const uint32_t k_utime_magic  = 0x4;
static const uint32_t k_ltime_magic  = 0x5;
static const uint32_t k_footer_magic = 0xe;

//### For VME node ######################
#define VME_MASTER_MAGIC 0x00564d45
#define VME_MASTER_HSIZE 3
struct VME_MASTER_HEADER{
  uint32_t m_magic;
  uint32_t m_data_size;
  uint32_t m_nblock;
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

void init_vme_master_header( VME_MASTER_HEADER *vme_master_header,
			     uint64_t data_size, uint64_t nblock );
void init_vme_module_header( VME_MODULE_HEADER *vme_module_header,
			     uint64_t vme_address, uint64_t data_size );
			      
#endif
