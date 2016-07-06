/*
  GPIO Test Version
  15 September 2015
*/

#include "gpio.hh"

#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include <iomanip>

#include "data_buffer.hh"
#include "gef/gefcmn_vme.h"

static GEF_VME_BUS_HDL hdl_gpio;
static GEF_VME_MASTER_HDL master_hdl_gpio=NULL;
static GEF_MAP_HDL map_hdl_gpio;
static GEF_VME_ADDR addr_gpio;

GEF_MAP_PTR ptr_gpio;

//static GpioRegister_t* GpioReg;
//volatile static void *ptr;

const int k_size = 0x20;
//static uint32_t data32;

//______________________________________________________
int init_gpio()
{
  std::cout<<"#D init_gpio()"<<std::endl;
  
  if(gefVmeOpen(&hdl_gpio)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Initialize VME!"<<std::endl;
      exit(-1);
    }
  
  return 0;
}
//______________________________________________________
int conf_gpio()
{
  std::cout<<"#D conf_gpio()"<<std::endl;

  addr_gpio.upper               = 0x00000000; //upper 32bits
  addr_gpio.lower               = address_gpio;
  addr_gpio.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_gpio.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  addr_gpio.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_gpio.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_gpio.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_gpio.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_gpio.flags               = GEF_VME_WND_PWEN;

  return 0;
}
//______________________________________________________
int open_gpio()
{
  std::cout<<"#D open_gpio()"<<std::endl;

  if(gefVmeCreateMasterWindow(hdl_gpio,&addr_gpio,k_size,&master_hdl_gpio)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Create Master Window !"<<std::endl;
      exit(-1);
    }
  
  if(gefVmeMapMasterWindow(master_hdl_gpio,0,k_size,&map_hdl_gpio,&ptr_gpio)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E gefVmeMapMasterWindow failed"<<std::endl;
      exit(-1);
    }

  if(gefVmeSetReadWriteByteSwap(master_hdl_gpio,GEF_ENDIAN_BIG)!=GEF_STATUS_SUCCESS)
    {                                                                           
      std::cerr<<"#E Cannot Byte Swap!"<<std::endl;                             
      exit(-1);                                                                 
    }        

  //define addresses_

  //____________________________________________________for debug
  uint32_t data32 = 0;
  for(int i = 0; i < 10 ; i++ )
    {
      GEF_STATUS status = gefVmeRead32(master_hdl_gpio,0x8,1,&data32);
      if(status!=GEF_STATUS_SUCCESS)
	{
	  std::cerr<<"#E Cannot Read data!"<<std::endl;
	  exit(-1);
	}
      std::cout<<"data: "<<data32<<std::endl;
    }
  //____________________________________________________for debug

  return 0;
}
//______________________________________________________
int read_gpio()
{
  std::cout<<"#D read_gpio()"<<std::endl;
  data_buffer & g_dbuf = data_buffer::get_instance();
  //  g_dbuf.add_data(0x4750494f); //GPIO (in ASCII code)
  uint32_t data32 = 0;
  GEF_STATUS status = gefVmeRead32(master_hdl_gpio,0,1,&data32);//event number
  if(status!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Read data!"<<std::endl;
      //      exit(-1);
    }
  else  g_dbuf.add_data(data32);

  // R.K.
  std::cout << "Event num = " << data32 << std::endl;

  status = gefVmeRead32(master_hdl_gpio,0x4,1,&data32);//spill number
  if(status!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Read data!"<<std::endl;
      //      exit(-1);
    }
  else  g_dbuf.add_data(data32);

  gefVmeRead32(master_hdl_gpio,0x8,1,&data32);//serial
  g_dbuf.add_data(data32);
  data32 = 0x0;
  g_dbuf.add_data(data32);//spill end flag

  return 0;
}
//______________________________________________________
int close_gpio()
{

  if (gefVmeUnmapMasterWindow(map_hdl_gpio)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E gefVmeUnmapMasterWindow failed"<<std::endl;
    }


  if(gefVmeReleaseMasterWindow(master_hdl_gpio)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Release Master Window !"<<std::endl;
      exit(-1);
    }

  if(gefVmeClose(hdl_gpio)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Close VME!"<<std::endl;
      exit(-1);
    }

  return 0;
}
//______________________________________________________
int reset_inputreg_gpio()
{
  std::cout<<"reset input register"<<std::endl;
  GEF_UINT32 data32  = 0;
  gefVmeWrite32(master_hdl_gpio,0x14,1,&data32);
  return 0;
}
//______________________________________________________
bool wait_gpio()
{
  GEF_UINT32 data32  = 0;
  gefVmeRead32(master_hdl_gpio,0x10,1,&data32);
  if( (data32>>8) & 0x1 == 0x1)
    {
      reset_inputreg_gpio();
      return true;
    }
  else return false;
}
//______________________________________________________
void make_pulse_gpio(int channel)
{
  GEF_UINT32 data = (GEF_UINT32)(1<<channel);
  gefVmeWrite32(master_hdl_gpio,0x1C,1,&data);
}
//______________________________________________________
void make_level_gpio(int channel)
{
}
//______________________________________________________
void test_gpio()
{

}
//______________________________________________________
