// vme03_node: vme_v7807.h

#ifndef VME_V7807_H_
#define VME_V7807_H_

#include <vme/vme.h>
#include <vme/vme_api.h>

#define DMA_BUF_LEN 1000

static const int max_hdl_num = 8;
extern vme_bus_handle_t bus_hdl;
extern vme_dma_handle_t dma_hdl;
extern uint32_t *dma_buf;

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
  volatile uint32_t *event;
  volatile uint32_t *spill;
  volatile uint32_t *serial;
};
#define VME_RM_AM VME_A32SD
#define VME_RM_MAP_SIZE 0x10000
extern const int VME_RM_NUM;
extern struct VME_RM_REG vme_rm[];

//### RPV-130 #############################
struct RPV130_REG{
  uint64_t addr;
  volatile uint16_t *latch1;
  volatile uint16_t *latch2;
  volatile uint16_t *rsff;
  volatile uint16_t *through;
  volatile uint16_t *pulse;
  volatile uint16_t *level;
  volatile uint16_t *csr1;
  volatile uint16_t *csr2;  
};
#define RPV130_AM   VME_A16S
#define RPV130_MAP_SIZE  0x100
extern const int RPV130_NUM;
extern struct RPV130_REG rpv130[];

//### V830 ################################
struct V830_REG{
  uint64_t addr;
  volatile uint32_t *counter[32];
  volatile uint32_t *enable;
  volatile uint16_t *clr;
  volatile uint16_t *str;
  volatile uint16_t *reset;
  volatile uint16_t *clear;
  volatile uint16_t *trig;  
};
#define V830_AM        VME_A32SD
#define V830_MAP_SIZE  0x10000
extern const int V830_NUM;
extern struct V830_REG v830[];

//### V775 ################################
struct V775_REG{
  uint64_t addr;
  volatile uint32_t *data_buf;
  volatile uint16_t *bitset1;
  volatile uint16_t *bitclr1;
  volatile uint16_t *str1;
  volatile uint16_t *bitset2;
  volatile uint16_t *bitclr2;
  volatile uint16_t *range;
};
#define V775_AM        VME_A32SD
#define V775_MAP_SIZE  0x10000
extern const int V775_NUM;
extern struct V775_REG v775[];

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
