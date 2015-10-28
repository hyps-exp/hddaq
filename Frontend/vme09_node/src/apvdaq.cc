#include "stdio.h"
#include <iostream>
#include <stdint.h>
#include <cstdlib>

//#include "define_module.hh"
#include "gef/gefcmn_vme.h"
#include <iomanip>
#include "data_buffer.hh"

#include "apvdaq.hh"
#include "apvdaq_param.hh"


// Master 
static GEF_VME_BUS_HDL    hdl_apvdaq_master;
static GEF_VME_MASTER_HDL master_hdl_apvdaq_master=NULL;
static GEF_MAP_HDL        map_hdl_apvdaq_master;
static GEF_VME_ADDR       addr_apvdaq_master;

// Slaves
static GEF_VME_BUS_HDL    hdl_apvdaq_slave1,hdl_apvdaq_slave2,hdl_apvdaq_slave3;
static GEF_VME_BUS_HDL    hdl_apvdaq_slave4,hdl_apvdaq_slave5,hdl_apvdaq_slave6;
static GEF_VME_BUS_HDL    hdl_apvdaq_slave7,hdl_apvdaq_slave8,hdl_apvdaq_slave9;

static GEF_VME_MASTER_HDL master_hdl_apvdaq_slave1=NULL,master_hdl_apvdaq_slave2=NULL,master_hdl_apvdaq_slave3=NULL;
static GEF_VME_MASTER_HDL master_hdl_apvdaq_slave4=NULL,master_hdl_apvdaq_slave5=NULL,master_hdl_apvdaq_slave6=NULL;
static GEF_VME_MASTER_HDL master_hdl_apvdaq_slave7=NULL,master_hdl_apvdaq_slave8=NULL,master_hdl_apvdaq_slave9=NULL;

static GEF_MAP_HDL        map_hdl_apvdaq_slave1,map_hdl_apvdaq_slave2,map_hdl_apvdaq_slave3;
static GEF_MAP_HDL        map_hdl_apvdaq_slave4,map_hdl_apvdaq_slave5,map_hdl_apvdaq_slave6;
static GEF_MAP_HDL        map_hdl_apvdaq_slave7,map_hdl_apvdaq_slave8,map_hdl_apvdaq_slave9;

static GEF_VME_ADDR       addr_apvdaq_slave1,addr_apvdaq_slave2,addr_apvdaq_slave3;
static GEF_VME_ADDR       addr_apvdaq_slave4,addr_apvdaq_slave5,addr_apvdaq_slave6;
static GEF_VME_ADDR       addr_apvdaq_slave7,addr_apvdaq_slave8,addr_apvdaq_slave9;


// Broadcast
static GEF_VME_BUS_HDL    hdl_apvdaq_broadcast;
static GEF_VME_MASTER_HDL master_hdl_apvdaq_broadcast=NULL;
static GEF_MAP_HDL        map_hdl_apvdaq_broadcast;
static GEF_VME_ADDR       addr_apvdaq_broadcast;

static uint32_t data32;
volatile static void* ptr;
//volatile static void* ptr1;
//static uint8_t* rptr,rptr1;

int init_apvdaq_module(GEF_VME_BUS_HDL* hdl_apvdaq)
{
  std::cout << "#D init_apvdaq()" << std::endl;
  if(gefVmeOpen(hdl_apvdaq)!=GEF_STATUS_SUCCESS)
    {
      std::cerr << "#E Cannot Initialize VME(APVDAQ)!" << std::endl;
      exit(-1);
    }

  return 0;
}


int init_apvdaq()
{
  init_apvdaq_module(&hdl_apvdaq_master);
  //init_apvdaq_module(&hdl_apvdaq_slave1);
  init_apvdaq_module(&hdl_apvdaq_slave2);  
  //init_apvdaq_module(&hdl_apvdaq_slave3);
  init_apvdaq_module(&hdl_apvdaq_slave4);
  //init_apvdaq_module(&hdl_apvdaq_slave5);  
  //init_apvdaq_module(&hdl_apvdaq_slave6);
  //init_apvdaq_module(&hdl_apvdaq_slave7);
  //init_apvdaq_module(&hdl_apvdaq_slave8);
  //init_apvdaq_module(&hdl_apvdaq_slave9);

  init_apvdaq_module(&hdl_apvdaq_broadcast);

  return 0;
}

//---------------------------------------------------------------------------------------

int conf_apvdaq()
{
  //Master
  addr_apvdaq_master.upper               = 0x00000000;
  addr_apvdaq_master.lower               = MASTER;
  addr_apvdaq_master.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_master.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_master.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_master.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_master.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_master.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_master.flags               = GEF_VME_WND_PWEN;

  //Slave1
  addr_apvdaq_slave1.upper               = 0x00000000;
  addr_apvdaq_slave1.lower               = SLAVE1;
  addr_apvdaq_slave1.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave1.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave1.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave1.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave1.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave1.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave1.flags               = GEF_VME_WND_PWEN;

  //Slave2
  addr_apvdaq_slave2.upper               = 0x00000000;
  addr_apvdaq_slave2.lower               = SLAVE2;
  addr_apvdaq_slave2.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave2.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave2.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave2.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave2.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave2.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave2.flags               = GEF_VME_WND_PWEN;

  //Slave3
  addr_apvdaq_slave3.upper               = 0x00000000;
  addr_apvdaq_slave3.lower               = SLAVE3;
  addr_apvdaq_slave3.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave3.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave3.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave3.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave3.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave3.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave3.flags               = GEF_VME_WND_PWEN;

  //Slave4
  addr_apvdaq_slave4.upper               = 0x00000000;
  addr_apvdaq_slave4.lower               = SLAVE4;
  addr_apvdaq_slave4.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave4.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave4.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave4.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave4.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave4.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave4.flags               = GEF_VME_WND_PWEN;

  //Slave5
  addr_apvdaq_slave5.upper               = 0x00000000;
  addr_apvdaq_slave5.lower               = SLAVE5;
  addr_apvdaq_slave5.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave5.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave5.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave5.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave5.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave5.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave5.flags               = GEF_VME_WND_PWEN;

  //Slave6
  addr_apvdaq_slave6.upper               = 0x00000000;
  addr_apvdaq_slave6.lower               = SLAVE6;
  addr_apvdaq_slave6.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave6.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave6.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave6.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave6.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave6.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave6.flags               = GEF_VME_WND_PWEN;

  //Slave7
  addr_apvdaq_slave7.upper               = 0x00000000;
  addr_apvdaq_slave7.lower               = SLAVE7;
  addr_apvdaq_slave7.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave7.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave7.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave7.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave7.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave7.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave7.flags               = GEF_VME_WND_PWEN;

 //Slave8
  addr_apvdaq_slave8.upper               = 0x00000000;
  addr_apvdaq_slave8.lower               = SLAVE8;
  addr_apvdaq_slave8.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave8.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave8.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave8.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave8.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave8.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave8.flags               = GEF_VME_WND_PWEN;

 //Slave9
  addr_apvdaq_slave9.upper               = 0x00000000;
  addr_apvdaq_slave9.lower               = SLAVE9;
  addr_apvdaq_slave9.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_slave9.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_slave9.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_slave9.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_slave9.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_slave9.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_slave9.flags               = GEF_VME_WND_PWEN;


  //Broadcast
  addr_apvdaq_broadcast.upper               = 0x00000000;
  addr_apvdaq_broadcast.lower               = 0x80000000;
  addr_apvdaq_broadcast.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_apvdaq_broadcast.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_apvdaq_broadcast.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_apvdaq_broadcast.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_apvdaq_broadcast.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_apvdaq_broadcast.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_apvdaq_broadcast.flags               = GEF_VME_WND_PWEN;


  return 0;
}



//---------------------------------------------------------------------------------------

int open_apvdaq_module(GEF_VME_BUS_HDL hdl_apvdaq, GEF_VME_ADDR addr_apvdaq, GEF_VME_MASTER_HDL* master_hdl_apvdaq, GEF_MAP_HDL* map_hdl_apvdaq)
{
  //int k_size = 0x100ffff;
  int k_size = 0xffff;
  if(addr_apvdaq.lower==MASTER || addr_apvdaq.lower==SLAVE2 || addr_apvdaq.lower==SLAVE4)k_size=0x100ffff;
  if(addr_apvdaq.lower==0x80000000)k_size=0xffff;
  printf("k_size = %d\n",k_size);

  std::cout << "#D open_apvdaq() " << addr_apvdaq.lower << std::endl;  

  if(gefVmeCreateMasterWindow(hdl_apvdaq,&addr_apvdaq,k_size,master_hdl_apvdaq)
     !=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Create Master Window of APVDAQ!"<<std::endl;
      exit(-1);
    }

  if(gefVmeSetReadWriteByteSwap(*master_hdl_apvdaq,GEF_ENDIAN_BIG)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Byte Swap!"<<std::endl;
      exit(-1);
    }

  
  if(gefVmeMapMasterWindow(*master_hdl_apvdaq,0,k_size,map_hdl_apvdaq,(GEF_MAP_PTR*) &ptr)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Memory Mapping !"<<std::endl;
      exit(-1);
    }
  

  return 0;
}

int open_apvdaq()
{
  open_apvdaq_module(hdl_apvdaq_master,addr_apvdaq_master,&master_hdl_apvdaq_master,&map_hdl_apvdaq_master);
  //open_apvdaq_module(hdl_apvdaq_slave1,addr_apvdaq_slave1,&master_hdl_apvdaq_slave1,&map_hdl_apvdaq_slave1);
  open_apvdaq_module(hdl_apvdaq_slave2,addr_apvdaq_slave2,&master_hdl_apvdaq_slave2,&map_hdl_apvdaq_slave2);
  //open_apvdaq_module(hdl_apvdaq_slave3,addr_apvdaq_slave3,&master_hdl_apvdaq_slave3,&map_hdl_apvdaq_slave3);
  open_apvdaq_module(hdl_apvdaq_slave4,addr_apvdaq_slave4,&master_hdl_apvdaq_slave4,&map_hdl_apvdaq_slave4);
  //open_apvdaq_module(hdl_apvdaq_slave5,addr_apvdaq_slave5,&master_hdl_apvdaq_slave5,&map_hdl_apvdaq_slave5);
  //open_apvdaq_module(hdl_apvdaq_slave6,addr_apvdaq_slave6,&master_hdl_apvdaq_slave6,&map_hdl_apvdaq_slave6);
  //open_apvdaq_module(hdl_apvdaq_slave7,addr_apvdaq_slave7,&master_hdl_apvdaq_slave7,&map_hdl_apvdaq_slave7);
  //open_apvdaq_module(hdl_apvdaq_slave8,addr_apvdaq_slave8,&master_hdl_apvdaq_slave8,&map_hdl_apvdaq_slave8);
  //open_apvdaq_module(hdl_apvdaq_slave9,addr_apvdaq_slave9,&master_hdl_apvdaq_slave9,&map_hdl_apvdaq_slave9);

  open_apvdaq_module(hdl_apvdaq_broadcast,addr_apvdaq_broadcast,&master_hdl_apvdaq_broadcast,&map_hdl_apvdaq_broadcast);
  return 0;
}
//---------------------------------------------------------------------------------------
int close_apvdaq_module(GEF_VME_BUS_HDL hdl_apvdaq, GEF_VME_MASTER_HDL master_hdl_apvdaq, GEF_MAP_HDL map_hdl_apvdaq)
{
  std::cout<<"#D close_apvdaq()"<<std::endl;

  if(gefVmeUnmapMasterWindow(map_hdl_apvdaq)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Unmap Master Window!"<<std::endl;
    }

  if(gefVmeReleaseMasterWindow(master_hdl_apvdaq)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Release Master Window!"<<std::endl;
      exit(-1);
    }

  if(gefVmeClose(hdl_apvdaq)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Close VME!"<<std::endl;
      exit(-1);
    }

  return 0;
}

//---------------------------------------------------------------------------------------

int close_apvdaq()
{
  close_apvdaq_module(hdl_apvdaq_master,master_hdl_apvdaq_master,map_hdl_apvdaq_master);
  //close_apvdaq_module(hdl_apvdaq_slave1,master_hdl_apvdaq_slave1,map_hdl_apvdaq_slave1);
  close_apvdaq_module(hdl_apvdaq_slave2,master_hdl_apvdaq_slave2,map_hdl_apvdaq_slave2);
  //close_apvdaq_module(hdl_apvdaq_slave3,master_hdl_apvdaq_slave3,map_hdl_apvdaq_slave3);
  close_apvdaq_module(hdl_apvdaq_slave4,master_hdl_apvdaq_slave4,map_hdl_apvdaq_slave4);
  //close_apvdaq_module(hdl_apvdaq_slave5,master_hdl_apvdaq_slave5,map_hdl_apvdaq_slave5);
  //close_apvdaq_module(hdl_apvdaq_slave6,master_hdl_apvdaq_slave6,map_hdl_apvdaq_slave6);
  //close_apvdaq_module(hdl_apvdaq_slave7,master_hdl_apvdaq_slave7,map_hdl_apvdaq_slave7);
  //close_apvdaq_module(hdl_apvdaq_slave8,master_hdl_apvdaq_slave8,map_hdl_apvdaq_slave8);
  //close_apvdaq_module(hdl_apvdaq_slave9,master_hdl_apvdaq_slave9,map_hdl_apvdaq_slave9);

  close_apvdaq_module(hdl_apvdaq_broadcast,master_hdl_apvdaq_broadcast,map_hdl_apvdaq_broadcast);

  return 0;
}

  

//---------------------------------------------------------------------------------------


int read_apvdaq()
{
  Read_APVDAQ();
  return 0;
}


void vwrite32(unsigned long module, unsigned long offset, long* data)
{
  GEF_STATUS status=0;
  GEF_UINT32 roffset = static_cast<GEF_UINT32>(offset);
  GEF_UINT32 rdata   = static_cast<GEF_UINT32>(*data);

  if(module == addr_apvdaq_master.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_master,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave1.lower)
    {
      //status = gefVmeWrite32(master_hdl_apvdaq_slave1,roffset,1,&rdata);
      status = gefVmeWrite32(master_hdl_apvdaq_master,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave2.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave2,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave3.lower)
    {
      //status = gefVmeWrite32(master_hdl_apvdaq_slave3,roffset,1,&rdata);
      status = gefVmeWrite32(master_hdl_apvdaq_slave2,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave4.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave4,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave5.lower)
    {
      //status = gefVmeWrite32(master_hdl_apvdaq_slave5,roffset,1,&rdata);
      status = gefVmeWrite32(master_hdl_apvdaq_slave4,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave6.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave6,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave7.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave7,roffset,1,&rdata);
      //status = gefVmeWrite32(master_hdl_apvdaq_slave6,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave8.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave8,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave9.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave9,roffset,1,&rdata);
      //status = gefVmeWrite32(master_hdl_apvdaq_slave8,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_broadcast.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_broadcast,roffset,1,&rdata);
    }
  if(status!=GEF_STATUS_SUCCESS)
    {
      //std::cerr<<"#E VME WRITE ERROR !"<<status<<std::endl;
      std::cerr<<"#E VME WRITE ERROR !"<< "Module = " << module << "Status = " << status<<std::endl;
      //      exit(-1);
    }
}

void vwrite32_uint(unsigned long module, unsigned long offset, unsigned long* data)
{
  GEF_STATUS status=0;
  GEF_UINT32 roffset = static_cast<GEF_UINT32>(offset);
  GEF_UINT32 rdata   = static_cast<GEF_UINT32>(*data);
  
  if(module == addr_apvdaq_master.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_master,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave1.lower)
    {
      //status = gefVmeWrite32(master_hdl_apvdaq_slave1,roffset,1,&rdata);
      status = gefVmeWrite32(master_hdl_apvdaq_master,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave2.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave2,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave3.lower)
    {
      //status = gefVmeWrite32(master_hdl_apvdaq_slave3,roffset,1,&rdata);
      status = gefVmeWrite32(master_hdl_apvdaq_slave2,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave4.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave4,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave5.lower)
    {
      //status = gefVmeWrite32(master_hdl_apvdaq_slave5,roffset,1,&rdata);
      status = gefVmeWrite32(master_hdl_apvdaq_slave4,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave6.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave6,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave7.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave7,roffset,1,&rdata);
      //status = gefVmeWrite32(master_hdl_apvdaq_slave6,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave8.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave8,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave9.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_slave9,roffset,1,&rdata);
      //status = gefVmeWrite32(master_hdl_apvdaq_slave8,roffset+0x01000000,1,&rdata);
    }

  if(module == addr_apvdaq_broadcast.lower)
    {
      status = gefVmeWrite32(master_hdl_apvdaq_broadcast,roffset,1,&rdata);
    }

  if(status!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E VME WRITE ERROR !"<<status<<std::endl;
      //exit(-1);
    }
}


void vread32(unsigned long module, unsigned long offset, long* data)
{
  GEF_STATUS status=0;
  GEF_UINT32 roffset = static_cast<GEF_UINT32>(offset);
  GEF_UINT32 rdata;
  
  if(module == addr_apvdaq_master.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_master,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave1.lower)
    {
      //status = gefVmeRead32(master_hdl_apvdaq_slave1,roffset,1,&rdata);
      status = gefVmeRead32(master_hdl_apvdaq_master,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave2.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave2,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave3.lower)
    {
      //status = gefVmeRead32(master_hdl_apvdaq_slave3,roffset,1,&rdata);
      status = gefVmeRead32(master_hdl_apvdaq_slave2,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave4.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave4,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave5.lower)
    {
      //status = gefVmeRead32(master_hdl_apvdaq_slave5,roffset,1,&rdata);
      status = gefVmeRead32(master_hdl_apvdaq_slave4,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave6.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave6,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave7.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave7,roffset,1,&rdata);
      //status = gefVmeRead32(master_hdl_apvdaq_slave6,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave8.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave8,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave9.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave9,roffset,1,&rdata);
      //status = gefVmeRead32(master_hdl_apvdaq_slave8,roffset+0x01000000,1,&rdata);
    }

  if(module == addr_apvdaq_broadcast.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_broadcast,roffset,1,&rdata);
    }

  if(status!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E VME READ ERROR !"<<status<<std::endl;
      exit(-1);
    }
  *data = rdata;
}

void vread32_uint(unsigned long module, unsigned long offset, unsigned long* data)
{
  GEF_STATUS status=0;
  GEF_UINT32 roffset = static_cast<GEF_UINT32>(offset);
  GEF_UINT32 rdata;
  
  if(module == addr_apvdaq_master.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_master,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave1.lower)
    {
      //status = gefVmeRead32(master_hdl_apvdaq_slave1,roffset,1,&rdata);
      status = gefVmeRead32(master_hdl_apvdaq_master,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave2.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave2,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave3.lower)
    {
      //status = gefVmeRead32(master_hdl_apvdaq_slave3,roffset,1,&rdata);
      status = gefVmeRead32(master_hdl_apvdaq_slave2,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave4.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave4,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave5.lower)
    {
      //status = gefVmeRead32(master_hdl_apvdaq_slave5,roffset,1,&rdata);
      status = gefVmeRead32(master_hdl_apvdaq_slave4,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave6.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave6,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave7.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave7,roffset,1,&rdata);
      //status = gefVmeRead32(master_hdl_apvdaq_slave6,roffset+0x01000000,1,&rdata);
    }
  if(module == addr_apvdaq_slave8.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave8,roffset,1,&rdata);
    }
  if(module == addr_apvdaq_slave9.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_slave9,roffset,1,&rdata);
      //status = gefVmeRead32(master_hdl_apvdaq_slave8,roffset+0x01000000,1,&rdata);
    }

  if(module == addr_apvdaq_broadcast.lower)
    {
      status = gefVmeRead32(master_hdl_apvdaq_broadcast,roffset,1,&rdata);
    }

  if(status!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E VME READ ERROR !"<<status<<std::endl;
      //exit(-1);
    }
  *data = rdata;
}
