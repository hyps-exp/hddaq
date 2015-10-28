#ifndef TDC_H
#define TDC_H

#include <iostream>
#include <stdint.h>
#include "gef/gefcmn_vme.h"

static uint32_t address_tdc = 0x20030000;

struct TdcRegister_t  
{
  uint16_t m_Firmware_Revision;  // (r)     0x1000
  uint16_t m_Geo_Addr;           // (rw)    0x1002
  uint16_t m_Mcst_Cblt_Addr;     // (rw)    0x1004
  uint16_t m_Bit_Set1;           // (rw)    0x1006
  uint16_t m_Bit_Clear1;         // (rw)    0x1008
  uint16_t m_Interrupt_Level;    // (rw)    0x100a
  uint16_t m_Interrupt_Vector;   // (rw)    0x100c
  uint16_t m_Status_Reg1;        // (r)     0x100e
  uint16_t m_Control_Reg;        // (rw)    0x1010
  uint16_t m_ADER_High;          // (rw)    0x1012
  uint16_t m_ADER_Low;           // (rw)    0x1014
  uint16_t m_Single_Shot_Reset;  // (w)     0x1016
  uint16_t reserve1;             //         0x1018
  uint16_t m_Mcst_Cblt_Ctrl;     // (rw)    0x101a
  uint16_t reserve2;             //         0x101c
  uint16_t reserve3;             //         0x101e
  uint16_t m_Event_Trig_Reg;     // (rw)    0x1020
  uint16_t m_Status_Reg2;        // (r)     0x1022
  uint16_t m_Event_Counter_L;    // (r)     0x1024
  uint16_t m_Event_Counter_H;    // (r)     0x1026
  uint16_t m_Increment_Event;    // (w)     0x1028
  uint16_t m_Increment_Offset;   // (w)     0x102a
  uint16_t m_Load_Test_Reg;      // (rw)    0x102c
  uint16_t m_Fast_Clr_Window;    // (rw)    0x102e
  uint16_t reserve4;             //         0x1030
  uint16_t m_Bit_Set2;           // (rw)    0x1032  
  uint16_t m_Bit_Clear2;         // (w)     0x1034
  uint16_t m_W_Mem_Test_Addr;    // (w)     0x1036
  uint16_t m_MTW_High;           // (w)     0x1038
  uint16_t m_MTW_Low;            // (w)     0x103a
  uint16_t m_Crate_Select;       // (rw)    0x103c
  uint16_t m_Test_Event_Write;   // (w)     0x103e
  uint16_t m_Event_Counter_Rst;  // (w)     0x1040
  uint16_t reserve5[15];         //  0x1042-0x105e
  uint16_t m_Full_Scale_Range;   // (rw)    0x1060
  uint16_t reserve6;             //         0x1062
  uint16_t m_R_Test_Addr;        // (w)     0x1064
  uint16_t reserve7;             //         0x1066
  uint16_t m_SW_Comm;            // (w)     0x1068     
  uint16_t m_Slide_Constant;     // (rw)    0x106a
  uint16_t reserve8;             //         0x106c
  uint16_t reserve9;             //         0x106e
  uint16_t m_AAD;                // (r)     0x1070
  uint16_t m_BAD;                // (r)     0x1072
};

enum data_format
  {
    k_header = 0x2,
    k_valid_data = 0x0,
    k_footer = 0x4
  };

enum TypeOfData{Header,ValidData,Footer};
int init_tdc();
int conf_tdc();
int open_tdc();
int close_tdc();
int read_tdc();

enum TypeOfData tdc_data_check(uint32_t data);

#endif
