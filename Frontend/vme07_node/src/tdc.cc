#include "tdc.hh"

#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include <iomanip>

#include "data_buffer.hh"
#include "DetectorID.hh"

#include "gef/gefcmn_vme.h"
#include "userdevice.h"

static GEF_VME_BUS_HDL hdl_tdc;
static GEF_VME_MASTER_HDL master_hdl_tdc;
static GEF_MAP_HDL map_hdl_tdc;
static GEF_VME_ADDR addr_tdc;

static TdcRegister_t* TdcReg;

volatile static void* ptr;
//volatile static void *data;
static uint8_t* rptr;
uint32_t* rdata;

const int      k_buf_size= 0x1000;
const int      k_size    = k_buf_size + sizeof(TdcRegister_t);

static uint32_t data32;



#include <algorithm>
template<class T>
T reverse_endian(T value)
{
  char* first = reinterpret_cast<char*>(&value);
  char* last  = first + sizeof(T);
  std::reverse(first, last);
  return value;
}

//______________________________________________________________________________
int init_tdc()
{
  std::cout<<"#D init_tdc()"<<std::endl;

  if(gefVmeOpen(&hdl_tdc)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Initialize VME!"<<std::endl;
      exit(-1);
    }

  return 0;
}
//______________________________________________________________________________
int conf_tdc()
{
  std::cout<<"#D conf_tdc"<<std::endl;

  addr_tdc.upper               = 0x00000000; // upper 32bits
  //  addr_tdc.lower               = 0x20030000; //lower 32bits
  addr_tdc.lower               = address_tdc;
  addr_tdc.addr_space          = GEF_VME_ADDR_SPACE_A32;
  addr_tdc.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;//Default value for non 2eSST transfer mode
  addr_tdc.addr_mode           = GEF_VME_ADDR_MODE_SUPER | GEF_VME_ADDR_MODE_DATA;
  addr_tdc.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  addr_tdc.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  addr_tdc.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  addr_tdc.flags               = GEF_VME_WND_PWEN;

  return 0;
}
//______________________________________________________________________________
int open_tdc()
{
  std::cout<<"#D open_tdc()"<<std::endl;
  if(gefVmeCreateMasterWindow(hdl_tdc,&addr_tdc,k_size,&master_hdl_tdc)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Create Master Window !"<<std::endl;
      exit(-1);
    }

  if(gefVmeSetReadWriteByteSwap(master_hdl_tdc,GEF_ENDIAN_BIG)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Byte Swap!"<<std::endl;
      exit(-1);
    }

  if(gefVmeMapMasterWindow(master_hdl_tdc,
			   0,
			   k_size,
			   &map_hdl_tdc,
			   (GEF_MAP_PTR*) &ptr)
     !=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Memory Mapping !"<<std::endl;
      exit(-1);
    }
  rptr = reinterpret_cast<uint8_t*>(const_cast<void*>(ptr));
  TdcReg = reinterpret_cast<TdcRegister_t*>(rptr+0x1000);

  uint16_t data = 0x80; //Module Reset
  TdcReg->m_Bit_Set1 = reverse_endian(data);
  TdcReg->m_Bit_Clear1 = reverse_endian(data);
  
  data = 0xff; //140[ns] full scaler range
  TdcReg->m_Full_Scale_Range = reverse_endian(data);

  data = (0<<10) | (1 <<12); // common start + always make header and footer
  TdcReg->m_Bit_Set2 = reverse_endian(data);
  data = 0;
  TdcReg->m_Control_Reg = reverse_endian(data);
#if 0
 std::cout<<"***********Status**************"<<std::hex<<std::endl;
 std::cout<<"Bit Set 1         : "<<reverse_endian(TdcReg->m_Bit_Set1)<<std::endl;
 std::cout<<"Bit Set 2         : "<<reverse_endian(TdcReg->m_Bit_Set2)<<std::endl;
 // std::cout<<"Bit Clear 1       : "<<TdcReg->m_Bit_Clear1<<std::endl;
 // std::cout<<"Status 1          : "<<TdcReg->m_Status_Reg1<<std::endl;
 // std::cout<<"Status 2          : "<<TdcReg->m_Status_Reg2<<std::endl;
 // std::cout<<"Interrupt Level   : "<<TdcReg->m_Interrupt_Level<<std::endl;
 // std::cout<<"Interrupt Vector  : "<<TdcReg->m_Interrupt_Vector<<std::endl;
 // std::cout<<"Crate Number      : "<<TdcReg->m_Crate_Select<<std::endl;
 // std::cout<<"Address High      : "<<TdcReg->m_ADER_High<<std::endl;
 // std::cout<<"Address Low       : "<<TdcReg->m_ADER_Low<<std::endl;
 std::cout<<"Full Scale Range  : "<<reverse_endian(TdcReg->m_Full_Scale_Range)<<std::endl;
#endif


  return 0;
}
//______________________________________________________________________________
void event_count_reset()
{
  TdcReg->m_Event_Counter_Rst = reverse_endian(0x0);  
  return;
}
//______________________________________________________________________________
int close_tdc()
{
  std::cout<<"#D close_tdc()"<<std::endl;
  if(gefVmeUnmapMasterWindow(map_hdl_tdc)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Unmap Master Window!"<<std::endl;
    }

  if(gefVmeReleaseMasterWindow(master_hdl_tdc)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Release Master Window!"<<std::endl;
      exit(-1);
    }

  if(gefVmeClose(hdl_tdc)!=GEF_STATUS_SUCCESS)
    {
      std::cerr<<"#E Cannot Close VME!"<<std::endl;
      exit(-1);
    }
  return 0;
}
//______________________________________________________________________________
int read_tdc()
{
  data_buffer & g_dbuf = data_buffer::get_instance();
  std::cout<<"#D read_tdc()"<<std::endl;
  const int max_try = 100;
  bool valid_data = true;
  uint16_t dready = 0;
  for(int j = 0;j<max_try;j++)
    {
      dready = reverse_endian(TdcReg->m_Status_Reg1)&0x1;
      if(dready==1) break;
    }
  if(dready==1)
    {
     while(valid_data)
       {
	 GEF_STATUS status = gefVmeRead32(master_hdl_tdc,0,1,&data32);
	 if(status!=GEF_STATUS_SUCCESS)
	   {
	     std::cerr<<"#E Cannot Read data!"<<status<<std::endl;
	     //	  exit(-1);
	}
	 
	 int  datatype = (data32>>24) & 0x7;
	 if(datatype==0 || datatype==2 ||datatype==4)
	   {
	     g_dbuf.add_data(data32);
	     /*
	     if(datatype==0)
	       {
		 std::cout<<"channel :"<<((data32>>17)&0xf)<<std::endl;
		 std::cout<<"tdc value:"<<(data32&0xfff)<<std::endl;
	       }
	       */
	   }
	 else
	   valid_data = false;
       }
     }
  else
    {
      std::string message = "vme07 v775 data is not ready";
      send_warning_message(message);                                           
    }
  return 0;
}
//______________________________________________________________________________
enum TypeOfData tdc_data_check(uint32_t data)
{
  enum TypeOfData tod;
  int type_of_word = data>>24 & 0x07;
  if(type_of_word==k_header)
    tod = Header;
  if(type_of_word==k_valid_data)
    tod = ValidData;
  if(type_of_word==k_footer)
    tod = Footer;
  return tod;
}

