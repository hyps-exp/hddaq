// vme01_node: vme_xvb.h

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

void vme_open( void );
void vme_close( void ); 
void check_handle_number( int handle_number );

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
  volatile GEF_UINT32 *dummy;
  volatile GEF_UINT32 *input;
  volatile GEF_UINT32 *reset;
  volatile GEF_UINT32 *level;
  volatile GEF_UINT32 *pulse;
  volatile GEF_UINT32 *time;
};
#define VME_RM_AM       GEF_VME_ADDR_SPACE_A32
#define VME_RM_MAP_SIZE 0x10000
extern const int VME_RM_NUM;
extern struct VME_RM_REG vme_rm[];

//### V792 ################################
struct V792_REG{
  uint64_t addr;
  GEF_VME_ADDR addr_param;
  volatile GEF_UINT32 *data_buf;
  volatile GEF_UINT16 *geo_addr;
  volatile GEF_UINT16 *chain_addr;
  volatile GEF_UINT16 *bitset1;
  volatile GEF_UINT16 *bitclr1;
  volatile GEF_UINT16 *str1;
  volatile GEF_UINT16 *chain_ctrl;
  volatile GEF_UINT16 *bitset2;
  volatile GEF_UINT16 *bitclr2;
  volatile GEF_UINT16 *iped;
};
#define V792_AM        GEF_VME_ADDR_SPACE_A32
#define V792_MAP_SIZE  0x10000
extern const int V792_NUM;
extern struct V792_REG v792[];

//### V775 ################################
struct V775_REG{
  uint64_t addr;
  GEF_VME_ADDR addr_param;
  volatile GEF_UINT32 *data_buf;
  volatile GEF_UINT16 *geo_addr;
  volatile GEF_UINT16 *chain_addr;
  volatile GEF_UINT16 *bitset1;
  volatile GEF_UINT16 *bitclr1;
  volatile GEF_UINT16 *str1;
  volatile GEF_UINT16 *chain_ctrl;
  volatile GEF_UINT16 *bitset2;
  volatile GEF_UINT16 *bitclr2;
  volatile GEF_UINT16 *range;
};
#define V775_AM        GEF_VME_ADDR_SPACE_A32
#define V775_MAP_SIZE  0x10000
extern const int V775_NUM;
extern struct V775_REG v775[];

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
