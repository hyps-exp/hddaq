#ifndef APVDAQ_H

#define APVDAQ_H

#include <iostream>
#include <stdint.h>
#include "gef/gefcmn_vme.h"


//--------------------------------
//apvdaq.h
//ASANO,Hidemitsmu
//ver1.0 2009 6 25
//
//ver1.1 2009 7 22 
//-define apv_i2c_setting array
//
//ver1.2 2009 10 8
//-separete addresses and setting parameters (to setparam.h)
//
//last update 2009 10 25
//
//-------------------------------

#define INIT_APV_FLAG 0

#define SET 1
#define UNSET 0
#define SET_RESET_I2C 0x2
#define APV_WRITE 0
#define APV_READ  1

#define CLOCK_NORMAL 0
#define CLOCK_INVERTED 1

#define SEQUENCER_LENGTH 256
#define HW_TRG_SEQUENCE 8
#define SW_TRG_SEQUENCE 4
#define SW_CAL_SEQUENCE 2
#define SW_RES_SEQUENCE 1


#define READ_AI2C_WORD        0x0000     
#define READ_AI2C_ERR	      0x0004     
#define WRITE_AI2C_WORD       0x0000     
#define WRITE_AI2C_ADDR_RW    0x0004     
#define WRITE_AI2C_RESET      0x0008     
#define TLC_READ              0x0040     
#define TLC_WRITE             0x0040     
#define READ_PI2C_WORD        0x0020     
#define READ_PI2C_ERR         0x0024     
#define WRITE_PI2C_WORD       0x0020     
#define WRITE_PI2C_ADDR_RW    0x0024     
#define WRITE_PI2C_RESET      0x0028     
#define READ_JTAG             0x0018     
#define WRITE_JTAG            0x0018     
#define WRITE_NCONFIG         0x0060     
#define WRITE_LRES            0x007c     
#define WRITE_CONF            0xd000    
#define READ_CONF             0xd000     
#define WRITE_CNT             0xa800     
#define WRITE_TRG             0x9800     
#define READ_VETO	      0xa000     
#define WRITE_VETO	      0xa000     
#define WRITE_TRG_SEQ         0xb000     
#define WRITE_SEQ_RAM         0xc000     
#define WRITE_DAC_RAM         0xb800     
#define WRITE_TP_SEL          0xfc00     

//For the new FPGA, adresses modified : June 7 /2014 Jaeyong

#define READ_FIFO_ORIGINAL_AB          0x8800     
#define READ_FIFO_ORIGINAL_CD          0x9000

#define READ_FIFO_AA          0x8400
#define READ_FIFO_AB          0x8800
#define READ_FIFO_AC          0x8C00
#define READ_FIFO_BA          0x9000
#define READ_FIFO_BB          0x9400
#define READ_FIFO_BC          0x9C00
#define READ_FIFO_CA          0xAC00   
#define READ_FIFO_CB          0xCC00
#define READ_FIFO_CC          0xB400
#define READ_FIFO_DA          0xD400
#define READ_FIFO_DB          0xBC00
#define READ_FIFO_DC          0xDC00

#define READ_FIFO_TEMP_A      0xC400
#define READ_FIFO_TEMP_B      0xC800
#define READ_FIFO_SIZE        0xF000

#define READ_DREADY          0xF400
#define BUFFERSWITCH          0x8000
#define WRITE_USER_CONF        0xF800 // added 18 Aug. switch for FIR & ZS mode
#define WRITE_FIR_PARA        0xE000
#define WRITE_DEV_THRESHOLD   0xFC00

// 23 September 2015 v1.8                                                                                                                      
#define FIR_CALIB_TRIGGER 0xE400
#define WRITE_PEDESTAL_ADC 0xE800
#define WRITE_PULSE_SHAPE_PARAMETER  0xEC00

//define Command Register Codes of APV25 chip

#define IPRE 		      0x20
#define IPCASC                0x22
#define IPSF                  0x24
#define ISHA		      0x26
#define ISSF                  0x28
#define IPSP                  0x2a
#define IMUXIN		      0x2c
#define ISPARE                0x2e
#define ICAL                  0x30
#define VFP                   0x32
#define VFS                   0x34
#define VPSP                  0x36
#define CDRV		      0x38
#define CSEL		      0x3a
#define MODE                  0x02
#define LATENCY               0x04
#define MUXGAIN               0x06
#define ERROR                 0x01



int init_apvdaq();
int conf_apvdaq();
int open_apvdaq();
int read_apvdaq();
int close_apvdaq();

int SsdInitialization();



void vwrite32(unsigned long module, unsigned long offset, long* data);
void vwrite32_uint(unsigned long module, unsigned long offset, unsigned long* data);
void vread32(unsigned long module, unsigned long offset, long* data);
void vread32_uint(unsigned long module, unsigned long offset, unsigned long* data);

//void vread32_uint_block(unsigned long module, unsigned long offset, int nblock, unsigned long* data);


void veto_on_test();

int init_apvdaq_module(GEF_VME_BUS_HDL hdl_apvdaq);
int open_apvdaq_module(GEF_VME_BUS_HDL hdl_apvdaq, GEF_VME_ADDR addr_apvdaq, GEF_VME_MASTER_HDL* master_hdl_apvdaq, GEF_MAP_HDL* map_hdl_apvdaq);
int close_apvdaq_module(GEF_VME_BUS_HDL hdl_apvdaq, GEF_VME_MASTER_HDL master_hdl_apvdaq, GEF_MAP_HDL map_hdl_apvdaq);

//apvdaq_main.cc function
void Init_APVDAQ();
void Init_APVDAQ_warmStart();
void Read_APVDAQ();


#endif
