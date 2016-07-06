// vme03_node: vme_xvb.cc

#include <cstdio>
#include <cstdlib>

#include "userdevice.h"
#include "vme_xvb.h"

//vme module list
// const int VME_RM_NUM = 1;
const int RPV130_NUM = 1;
const int V830_NUM   = 3;
// VME_RM_REG vme_rm[VME_RM_NUM] = { { 0xff030000 } };
RPV130_REG rpv130[RPV130_NUM] = { { 0x00008ff0 } };
V830_REG   v830[V830_NUM]     =
  { { 0x20010000 }, { 0x20020000 }, { 0x20030000 } };

//global variables
GEF_VME_DMA_HDL  dma_hdl;
GEF_UINT32      *dma_buf;
GEF_VME_ADDR     dma_addr;

//local variables
static GEF_VME_BUS_HDL    bus_hdl;
static GEF_VME_MASTER_HDL mst_hdl[max_hdl_num];
static GEF_MAP_HDL        map_hdl[max_hdl_num];
static int hdl_num = 0;

void vme_open()
{
  int status = 0;
  char message[256];
  //open device
  status = gefVmeOpen(&bus_hdl);
  if(status!=GEF_STATUS_SUCCESS){
    sprintf(message, "vme03: gefVmeOpen() failed -- %d", GEF_GET_ERROR(status));
    send_fatal_message(message);
    std::exit(EXIT_FAILURE);
  }
  status = gefVmeAllocDmaBuf( bus_hdl, 4*DMA_BUF_LEN, &dma_hdl, (GEF_MAP_PTR*)&dma_buf );
  //### DMA ####################################################
  {
    GEF_VME_ADDR addr_param = {
      0x00000000,                     //upoper
      0xAA000000,                     //lower
      GEF_VME_ADDR_SPACE_A32,         //addr_space
      GEF_VME_2ESST_RATE_INVALID,     //vme_2esst_rate
      GEF_VME_ADDR_MODE_DEFAULT,      //addr_mode
      GEF_VME_TRANSFER_MODE_BLT,      //transfer_mode
      GEF_VME_BROADCAST_ID_DISABLE,   //broadcast_id
      GEF_VME_TRANSFER_MAX_DWIDTH_32, //transfer_max_dwidth
      GEF_VME_DMA_DEFAULT|GEF_VME_DMA_PFAR //flags
    };
    dma_addr = addr_param;
  }
  //### VME_RM #################################################
  // {
  //   check_handle_number(hdl_num);
  //   for(int i=0;i<VME_RM_NUM;i++){
  //     GEF_VME_ADDR addr_param = {
  // 	0x00000000,                     //upoper
  // 	vme_rm[i].addr & 0xffffffff,    //lower
  // 	VME_RM_AM,                      //addr_space
  // 	GEF_VME_2ESST_RATE_INVALID,     //vme_2esst_rate
  // 	GEF_VME_ADDR_MODE_DEFAULT,      //addr_mode
  // 	GEF_VME_TRANSFER_MODE_SCT,      //transfer_mode
  // 	GEF_VME_BROADCAST_ID_DISABLE,   //broadcast_id
  // 	GEF_VME_TRANSFER_MAX_DWIDTH_32, //transfer_max_dwidth
  // 	GEF_VME_WND_EXCLUSIVE           //flags
  //     };
  //     vme_rm[i].addr_param = addr_param;
  //   }
  //   GEF_MAP_PTR ptr;
  //   GEF_UINT32 w_size = VME_RM_MAP_SIZE * VME_RM_NUM;
  //   status = gefVmeCreateMasterWindow(bus_hdl, &vme_rm[0].addr_param, w_size, &mst_hdl[hdl_num]);
  //   if(status!=GEF_STATUS_SUCCESS){
  //     sprintf(message, "vme03: VME_RM: gefVmeCreateMasterWindow() failed -- %d", GEF_GET_ERROR(status));
  //     send_fatal_message(message);
  //     std::exit(EXIT_FAILURE);
  //   }
  //   status = gefVmeMapMasterWindow(mst_hdl[hdl_num], 0, w_size, &map_hdl[hdl_num], &ptr);
  //   if(status!=GEF_STATUS_SUCCESS){
  //     sprintf(message, "vme03: VME_RM: gefVmeMapMasterWindow() failed -- %d", GEF_GET_ERROR(status));
  //     send_fatal_message(message);
  //     std::exit(EXIT_FAILURE);
  //   }
  //   hdl_num++;
  //   for(int i=0;i<VME_RM_NUM;i++){
  //     const int d32 = GEF_VME_DWIDTH_D32;
  //     const int offset32 = VME_RM_MAP_SIZE/d32*i;
  //     vme_rm[i].event  = (GEF_UINT32*)ptr +offset32 +0x00/d32;
  //     vme_rm[i].spill  = (GEF_UINT32*)ptr +offset32 +0x04/d32;
  //     vme_rm[i].serial = (GEF_UINT32*)ptr +offset32 +0x08/d32;
  //     vme_rm[i].time   = (GEF_UINT32*)ptr +offset32 +0x20/d32;
  //     sprintf(message, "vme03: VME_RM  [%08llx] joined", vme_rm[i].addr);
  //     send_normal_message(message);
  //   }
  // }
  //### RPV130 #################################################
  {
    check_handle_number(hdl_num);
    for(int i=0;i<RPV130_NUM;i++){
      GEF_VME_ADDR addr_param = {
	0x00000000,                     //upoper
	rpv130[i].addr & 0xffffffff,    //lower
	RPV130_AM,                      //addr_space
	GEF_VME_2ESST_RATE_INVALID,     //vme_2esst_rate
	GEF_VME_ADDR_MODE_DEFAULT,      //addr_mode
	GEF_VME_TRANSFER_MODE_SCT,      //transfer_mode
	GEF_VME_BROADCAST_ID_DISABLE,   //broadcast_id
	GEF_VME_TRANSFER_MAX_DWIDTH_32, //transfer_max_dwidth
	GEF_VME_WND_EXCLUSIVE           //flags
      };
      rpv130[i].addr_param = addr_param;
    }
    GEF_MAP_PTR ptr;
    GEF_UINT32 w_size = RPV130_MAP_SIZE * RPV130_NUM;
    status = gefVmeCreateMasterWindow(bus_hdl, &rpv130[0].addr_param, w_size, &mst_hdl[hdl_num]);
    if(status!=GEF_STATUS_SUCCESS){
      sprintf(message, "vme03: RPV130: gefVmeCreateMasterWindow() failed -- %d", GEF_GET_ERROR(status));
      send_fatal_message(message);
      std::exit(EXIT_FAILURE);
    }
    status = gefVmeMapMasterWindow(mst_hdl[hdl_num], 0, w_size, &map_hdl[hdl_num], &ptr);
    if(status!=GEF_STATUS_SUCCESS){
      sprintf(message, "vme03: RPV130: gefVmeMapMasterWindow() failed -- %d", GEF_GET_ERROR(status));
      send_fatal_message(message);
      std::exit(EXIT_FAILURE);
    }
    hdl_num++;
    for(int i=0;i<RPV130_NUM;i++){
      const int d16 = GEF_VME_DWIDTH_D16;
      const int offset16 = RPV130_MAP_SIZE/d16*i;
      rpv130[i].latch1  = (GEF_UINT16*)ptr +offset16 +0x0/d16;
      rpv130[i].latch2  = (GEF_UINT16*)ptr +offset16 +0x2/d16;
      rpv130[i].rsff    = (GEF_UINT16*)ptr +offset16 +0x4/d16;
      rpv130[i].through = (GEF_UINT16*)ptr +offset16 +0x6/d16;
      rpv130[i].pulse   = (GEF_UINT16*)ptr +offset16 +0x8/d16;
      rpv130[i].level   = (GEF_UINT16*)ptr +offset16 +0xA/d16;
      rpv130[i].csr1    = (GEF_UINT16*)ptr +offset16 +0xC/d16;
      rpv130[i].csr2    = (GEF_UINT16*)ptr +offset16 +0xE/d16;
      sprintf(message, "vme03: RPV130  [%08llx] joined", rpv130[i].addr);
      send_normal_message(message);
    }
  }
  //### V830 ###################################################
  {
    check_handle_number(hdl_num);
    for(int i=0;i<V830_NUM;i++){
      GEF_VME_ADDR addr_param = {
	0x00000000,                     //upoper
	v830[i].addr & 0xffffffff,      //lower
	V830_AM,                        //addr_space
	GEF_VME_2ESST_RATE_INVALID,     //vme_2esst_rate
	GEF_VME_ADDR_MODE_DEFAULT,      //addr_mode
	GEF_VME_TRANSFER_MODE_SCT,      //transfer_mode
	// GEF_VME_TRANSFER_MODE_BLT,      //transfer_mode
	GEF_VME_BROADCAST_ID_DISABLE,   //broadcast_id
	GEF_VME_TRANSFER_MAX_DWIDTH_32, //transfer_max_dwidth
	GEF_VME_WND_EXCLUSIVE           //flags
      };
      v830[i].addr_param = addr_param;
    }
    GEF_MAP_PTR ptr;
    GEF_UINT32 w_size = V830_MAP_SIZE * V830_NUM;
    status = gefVmeCreateMasterWindow(bus_hdl, &v830[0].addr_param, w_size, &mst_hdl[hdl_num]);
    if(status!=GEF_STATUS_SUCCESS){
      sprintf(message, "vme03: V830: gefVmeCreateMasterWindow() failed -- %d", GEF_GET_ERROR(status));
      send_fatal_message(message);
      std::exit(EXIT_FAILURE);
    }
    status = gefVmeMapMasterWindow(mst_hdl[hdl_num], 0, w_size, &map_hdl[hdl_num], &ptr);
    if(status!=GEF_STATUS_SUCCESS){
      sprintf(message, "vme03: V830: gefVmeMapMasterWindow() failed -- %d", GEF_GET_ERROR(status));
      send_fatal_message(message);
      std::exit(EXIT_FAILURE);
    }
    hdl_num++;
    for(int i=0;i<V830_NUM;i++){
      const int d16 = GEF_VME_DWIDTH_D16;
      const int d32 = GEF_VME_DWIDTH_D32;
      const int offset16 = V830_MAP_SIZE/d16*i;
      const int offset32 = V830_MAP_SIZE/d32*i;
      for(int ch=0;ch<32;ch++){
	v830[i].counter[ch] = (GEF_UINT32*)ptr +offset32 +0x1000/d32 +0x4/d32*ch;
      }
      v830[i].enable     = (GEF_UINT32*)ptr +offset32 +0x1100/d32;
      v830[i].clr        = (GEF_UINT16*)ptr +offset16 +0x1108/d16;
      v830[i].str        = (GEF_UINT16*)ptr +offset16 +0x110e/d16;
      v830[i].geo_addr   = (GEF_UINT16*)ptr +offset16 +0x1110/d16;
      v830[i].chain_addr = (GEF_UINT16*)ptr +offset16 +0x111c/d16;
      v830[i].chain_ctrl = (GEF_UINT16*)ptr +offset16 +0x111e/d16;
      v830[i].reset      = (GEF_UINT16*)ptr +offset16 +0x1120/d16;
      v830[i].clear      = (GEF_UINT16*)ptr +offset16 +0x1122/d16;
      v830[i].trig       = (GEF_UINT16*)ptr +offset16 +0x1124/d16;
      sprintf(message, "vme03: V830    [%08llx] joined", v830[i].addr);
      send_normal_message(message);
    }
  }

  return;
}

void vme_close( void )
{
  int status = 0;
  char message[256];
  //unmap and release
  status = gefVmeFreeDmaBuf( dma_hdl );
  if(status!=GEF_STATUS_SUCCESS){
    sprintf(message, "vme03: gefVmeFreeDmaBuf() failed -- %d", GEF_GET_ERROR(status));
    send_fatal_message(message);
    std::exit(EXIT_FAILURE);
  }
  for(int i=0;i<hdl_num;i++){
    status = gefVmeUnmapMasterWindow( map_hdl[i] );    
    if(status!=GEF_STATUS_SUCCESS){
      sprintf(message, "vme03: gefVmeUnmapMasterWindow() failed -- %d", GEF_GET_ERROR(status));
      send_fatal_message(message);      
      std::exit(EXIT_FAILURE);
    }
    status = gefVmeReleaseMasterWindow( mst_hdl[i] );
    if(status!=GEF_STATUS_SUCCESS){
      sprintf(message, "vme04: gefVmeReleaseMasterWindow() failed -- %d", GEF_GET_ERROR(status));
      send_fatal_message(message);
      std::exit(EXIT_FAILURE);
    }
  }
  //close device
  status = gefVmeClose(bus_hdl);
  if(status!=GEF_STATUS_SUCCESS){
    sprintf(message, "vme04: gefVmeClose() failed -- %d", GEF_GET_ERROR(status));
    send_fatal_message(message);
    std::exit(EXIT_FAILURE);
  }
}

void check_handle_number(int handle_number)
{
  if(handle_number>=max_hdl_num){
    char message[256];
    sprintf(message, "vme03: too many Master Windows: %d/%d",
            handle_number, max_hdl_num);
    send_fatal_message(message);
    std::exit(EXIT_FAILURE);
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
