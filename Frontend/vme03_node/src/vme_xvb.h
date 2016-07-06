// vme03_node: vme_xvb.h

#ifndef VME_XVB_H_
#define VME_XVB_H_

#include <stdint.h>
#include <gef/gefcmn_vme.h>
#include <byteswap.h>

#define DMA_BUF_LEN 1000

static const int max_hdl_num = 8;
extern GEF_VME_DMA_HDL   dma_hdl;
extern GEF_UINT32       *dma_buf;
extern GEF_VME_ADDR      dma_addr;

void vme_open();
void vme_close(); 
void check_handle_number(int handle_number);

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

//### VME-RM ##############################
struct VME_RM_REG {
  uint64_t addr;
  GEF_VME_ADDR addr_param;
  volatile GEF_UINT32 *event;
  volatile GEF_UINT32 *spill;
  volatile GEF_UINT32 *serial;
  volatile GEF_UINT32 *time;
};
#define VME_RM_AM       GEF_VME_ADDR_SPACE_A32
#define VME_RM_MAP_SIZE 0x10000
extern const int VME_RM_NUM;
extern struct VME_RM_REG vme_rm[];

//### RPV-130 #############################
struct RPV130_REG{
  uint64_t addr;
  GEF_VME_ADDR addr_param;
  volatile GEF_UINT16 *latch1;
  volatile GEF_UINT16 *latch2;
  volatile GEF_UINT16 *rsff;
  volatile GEF_UINT16 *through;
  volatile GEF_UINT16 *pulse;
  volatile GEF_UINT16 *level;
  volatile GEF_UINT16 *csr1;
  volatile GEF_UINT16 *csr2;  
};
#define RPV130_AM        GEF_VME_ADDR_SPACE_A16
#define RPV130_MAP_SIZE  0x1000
extern const int RPV130_NUM;
extern struct RPV130_REG rpv130[];

//### V830 ################################
struct V830_REG{
  uint64_t addr;
  GEF_VME_ADDR addr_param;
  volatile GEF_UINT32 *counter[32];
  volatile GEF_UINT32 *enable;
  volatile GEF_UINT16 *clr;
  volatile GEF_UINT16 *str;
  volatile GEF_UINT16 *geo_addr;
  volatile GEF_UINT16 *chain_addr;
  volatile GEF_UINT16 *chain_ctrl;
  volatile GEF_UINT16 *reset;
  volatile GEF_UINT16 *clear;
  volatile GEF_UINT16 *trig;  
};
#define V830_AM        GEF_VME_ADDR_SPACE_A32
#define V830_MAP_SIZE  0x10000
extern const int V830_NUM;
extern struct V830_REG v830[];

//### For VME node ######################
#define VME_MASTER_MAGIC 0x00564d45 /* VME */
#define VME_MASTER_HSIZE 3
struct VME_MASTER_HEADER{
  uint32_t m_magic;
  uint32_t m_data_size;
  uint32_t m_nblock;
};

#define VME_MODULE_MAGIC 0x766d65626f617264ULL /* VMEBOARD */
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
