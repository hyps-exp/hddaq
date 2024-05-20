// -*- C++ -*-

// Author: Shuhei Hayakawa
// Modified by Koji Miwa for V1725

#include "OptCaenV1725.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <string.h>
#include <math.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"


namespace opt
{

//______________________________________________________________________________
CaenV1725::CaenV1725( int link_num, int conet_node, uint32_t base_addr )
  : OptlinkModule(link_num, conet_node, base_addr), buffer(NULL), AllocatedSize(0), eventIndex(-1)
{
}

//______________________________________________________________________________
CaenV1725::~CaenV1725( void )
{
}

//______________________________________________________________________________
CAENComm_ErrorCode
CaenV1725::Open( void )
{
  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB_A4818, m_link_num, m_conet_node, 0, &m_handle);
  if (ret != CAEN_DGTZ_Success) {
    std::cout << "["+ClassName()+"::"+__func__+"()] " << std::endl;
    std::cout << "OpenDigitizer error "  << std::endl;

    return CAENComm_CommError;
  }
  
  if (CAEN_DGTZ_GetInfo(m_handle, &BoardInfo) == CAEN_DGTZ_Success) {
    std::cout << "["+ClassName()+"::"+__func__+"()] " << std::endl;
    std::cout << "Connected to CAEN Digitizer Model " << BoardInfo.ModelName << std::endl;
    std::cout << "Board serial number  " << BoardInfo.SerialNumber << std::endl;
    std::cout << "ROC FPGA Release is  " << BoardInfo.ROC_FirmwareRel << std::endl;
    std::cout << "AMC FPGA Release is  " << BoardInfo.AMC_FirmwareRel << std::endl;

    uint32_t buff;
    CAEN_DGTZ_ReadRegister(m_handle, nAMCFirmware, &buff);
    
    if (((buff >> 8) & 0xff) == 0x8C) {
      return CAENComm_Success;
    } else {
      std::cout << "The firmware is not DDP_ZLE!" << std::endl;
      CAEN_DGTZ_CloseDigitizer( m_handle );
      return CAENComm_CommError;
    }
  }

  return CAENComm_CommError;
}

//______________________________________________________________________________
CAENComm_ErrorCode
CaenV1725::Close( void )
{
  //FreeReadoutBuffer 
  if (buffer)
    CAEN_DGTZ_FreeReadoutBuffer(&buffer);

  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_CloseDigitizer( m_handle );

  if (ret == CAEN_DGTZ_Success) {
    std::cout << "["+ClassName()+"::"+__func__+"()] " << std::endl;
    std::cout << "Close Digitizer successfully "  << std::endl;

    return CAENComm_Success;
  }

  std::cout << "["+ClassName()+"::"+__func__+"()] " << std::endl;
  std::cout << "Close Digitizer error "  << std::endl;
  
  return CAENComm_CommError;
}

//______________________________________________________________________________
void
CaenV1725::SetDefaultConfig( void )
{

  FPIOtype = CAEN_DGTZ_IOLevel_NIM; // NIM
  //FPIOtype = CAEN_DGTZ_IOLevel_TTL; //TTL
  EnableMask = 0xffff;
  // Software Trigger  
  // DISABLED               : CAEN_DGTZ_TRGMODE_DISABLED
  // ACQUISITION_ONLY       : CAEN_DGTZ_TRGMODE_ACQ_ONLY 
  // TRGOUT_ONLY            : CAEN_DGTZ_TRGMODE_EXTOUT_ONLY
  // ACQUISITION_AND_TRGOUT : CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT
  SWTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;

  // External Trigger
  // DISABLED               : CAEN_DGTZ_TRGMODE_DISABLED
  // ACQUISITION_ONLY       : CAEN_DGTZ_TRGMODE_ACQ_ONLY
  // ACQUISITION_AND_TRGOUT : CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT
  ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;

  StartMode = CAEN_DGTZ_SW_CONTROLLED;

  //GAIN (0/1)(B): sets the input dynamic range (0->2Vpp, 1->0.5Vpp)
  GainFactor = 0;

  RecordLength = 256;
  
  TrigCoupleMask = 0x00; // trigger request does not participate to the global trigger
  TrigOutCoupleMask = 0x00; // trigger request does not participate to the TRG-OUT signal

  for (int j = 0; j < NofCh; j++) {
    if (!(j / 2)) TrigCoupleLogic[j / 2] = 3;
    // 1: positive, 0: negative
    PulsePolarity[j] = 0;
    // Number of pre trigger event
    preTrgg[j] = 12;
    NSampBck[j] = 4;
    NSampAhe[j] = 4;
    ZleTrigThr[j] = 1000;
    ZleThr[j] = 1000;
    BLineMode[j] = 0;
    BLineDefValue[j] = 0x2000;
    BLineNoise[j] = 4;
    DCoffset[j] = 0;
    // NoThreshold 1 : no ZLE
    NoThreshold[j] = 0;
    TP_Enable[j] = 0;
    TP_Polarity[j] = 1;
    TP_Rate[j] = 0;
    TP_Scale[j] = 0;
  }

}
//______________________________________________________________________________
bool
CaenV1725::SetChannelEnable( uint16_t mask)
{
  EnableMask = mask;
  return true;
}


//______________________________________________________________________________
void
CaenV1725::SetRecordLength( uint32_t len )
{
  RecordLength = len;
}
//______________________________________________________________________________
bool
CaenV1725::SetDCOffset( int ch, int offset)
{
  if (ch >=0 && ch < NofCh) {
    DCoffset[ch] = offset;
    return true;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetNoZLEMode( int ch, int value)
{
  if (ch >=0 && ch < NofCh) {
    NoThreshold[ch] = value;
    return true;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetPreTriggerSamples( int channel, uint32_t samples )
{
  if (samples < 256) {
    preTrgg[channel] = samples;
    return true;    
  } else {
    std::cout << "invalid value for post-signal sample number of channel " << channel << std::endl;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetPreSamples( int channel, uint32_t samples )
{
  if (samples < 256) {
    NSampBck[channel] = samples;
    return true;    
  } else {
    std::cout << "invalid value for post-signal sample number of channel " << channel << std::endl;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetPostSamples( int channel, uint32_t samples )
{
  if (samples < 256) {
    NSampAhe[channel] = samples;
    return true;    
  } else {
    std::cout << "invalid value for post-signal sample number of channel " << channel << std::endl;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetBLineDefValue( int channel, uint32_t baseline )
{
  if (baseline < 16384) {
    BLineDefValue[channel] = baseline;
    return true;    
  } else {
    std::cout << "invalid value for default baseline of channel " << channel << std::endl;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetDataThreshold( int channel, uint16_t threshold )
{
  if (threshold < 16384) {
    ZleThr[channel] = threshold;
    return true;    
  } else {
    std::cout << "invalid value for data threshold of channel " << channel << std::endl;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1725::SetTriggerThreshold( int channel, uint16_t threshold )
{
  if (threshold < 16384) {
    ZleTrigThr[channel] = threshold;
    return true;    
  } else {
    std::cout << "invalid value for data threshold of channel " << channel << std::endl;
  }

  return false;
}

//______________________________________________________________________________
int
CaenV1725::ProgramRegister( void )
{
  int i, ret = 0;
  uint32_t regval;

  ret |= CAEN_DGTZ_Reset(m_handle);
  ret |= CAEN_DGTZ_SetIOLevel(m_handle, FPIOtype);
  //ret |= CAEN_DGTZ_SetAcquisitionMode(m_handle, StartMode);
  ret |= CAEN_DGTZ_SetSWTriggerMode(m_handle, SWTriggerMode);
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(m_handle, ExtTriggerMode);
  // SetTrigger Mask
  CAEN_DGTZ_ReadRegister(m_handle, 0x810C, &regval);
  ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x810C, (regval & ~(0x000000ff)) | (uint32_t)(TrigCoupleMask));

  // SetTriggerOut Mask
  CAEN_DGTZ_ReadRegister(m_handle, 0x8110, &regval);
  ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x8110, (regval & ~(0x000000ff)) | (uint32_t)(TrigOutCoupleMask));

  // Active channels
  ret |= CAEN_DGTZ_SetChannelEnableMask(m_handle, (uint32_t)EnableMask);
  // gain control
  ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x8028, GainFactor);
  // record length
  ret |= CAEN_DGTZ_SetRecordLength(m_handle, RecordLength);
  // max BLT events
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(m_handle, 1023);
    
  if (0) {
    /*
    // sets whether the LVDS quartets are input or output (bits [5:2]): 1st quartet is input, other outputs here
    // sets the LVDS "new" mode (bit 8)
    // TRG OUT is used to propagate signals (bits [17:16])
    // signal propagated through the trgout (bits [19:18]) is the busy signal
    // enable extended time-stamp (bits [22:21] = "10")
    // the other two quartets (not used) are also set to output
    ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x811C, &regval);
    ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x811C, regval | 0x4d0138);
    // acquisition mode is sw-controlled for the first board, LVDS-controlled for the others
    ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x8100, &regval);
    ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x8100, regval | (board == 0 ? 0x00000100 : 0x00000107));
    // register 0x816C: reduces the threshold at which the BUSY is raised at 2^buffer organization-10 events
    ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x800C, &regval);
    ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x816C, (uint32_t)(pow(2., regval) - 10));
    // register 0x8170: timestamp offset
    ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x8170, 3*(ConfigVar->Nhandle - board - 1) + (board == 0 ? -1 : 0));
    // register 0x81A0: select the lowest two quartet as "nBUSY/nVETO" type. BEWARE: set ALL the quartet bits to 2
    ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x81A0, &regval);
    ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x81A0, regval | 0x00002222);
    */
  }else {
    // enable extended timestamp (bits [22:21] = "10")
    ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x811C, &regval);
    ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x811C, regval | 0x400000);
    // set acquisition mode
    ret |= CAEN_DGTZ_SetAcquisitionMode(m_handle, StartMode);
    // register 0x8100: set bit 2 to 1 if not in sw-controlled mode
    ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x8100, &regval);
    if (StartMode == CAEN_DGTZ_SW_CONTROLLED)
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x00008100, regval | 0x000100);
    else
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x8100, regval | 0x00000104);
  }

  // channel-specific settings	
  for (i = 0; i < NofCh; i++) {
    //trigger generation mode from channel couples
    if ((i % 2) == 0) {
      uint32_t regvalue;
      int couple = i/2;
      uint32_t logic = TrigCoupleLogic[(int)(i / 2)];

      CAEN_DGTZ_ReadRegister(m_handle, 0x1068, &regvalue);
      regvalue = (regvalue & (uint32_t)(~(0x00000003 << (2 * couple)))) | ((uint32_t)(logic << (2 * couple))) ; // replace only the two bits affecting the selected couple's logic.                                                   
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x8068,regvalue);
    }
    if (EnableMask & (1 << i)) {
      // set DC offset
      ret |= CAEN_DGTZ_SetChannelDCOffset(m_handle, i, DCoffset[i]);
      // pretrigger
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1038 | (i << 8), preTrgg[i]);

      //ZLE baseline register
      //ZLE_SetBLineMode
      CAEN_DGTZ_ReadRegister(m_handle, 0x1034 | (i << 8), &regval);
      if (BLineMode[i])
	ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1034 | (i << 8), regval | (uint32_t)(1 << 24));
      else
	ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1034 | (i << 8), (regval & (~(uint32_t)(1 << 24))));

      //ZLE_SetBLineDefValue
      CAEN_DGTZ_ReadRegister(m_handle, 0x1034 | (i << 8), &regval);
      regval = (regval & (uint32_t)(~(0x00003fff))) | (uint32_t)(BLineDefValue[i] & 0x3fff); // replace only the two bits affecting the selected couple's logic.
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1034 | (i << 8), regval);

      //ZLE_SetBLineNoise
      CAEN_DGTZ_ReadRegister(m_handle, 0x1034 | (i << 8), &regval);
      regval = (regval & (uint32_t)(~(0x00ff0000))) | (uint32_t)(BLineNoise[i] << 16); // replace only the two bits affecting the selected couple's logic.
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1034 | (i << 8), regval);
      
      //NSampBack 
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1054 | (i << 8),NSampBck[i]);

      //NSampAhead
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1058 | (i << 8),NSampAhe[i]);

      //ZLE Threshold
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x105C | (i << 8), (uint32_t)(ZleThr[i] & 0x3FFF));

      //ZLE Trigger Threshold
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1060 | (i << 8), (uint32_t)(ZleTrigThr[i] & 0x3FFF));

      //ZLE signal logic register
      CAEN_DGTZ_ReadRegister(m_handle, 0x1064 | (i << 8), &regval);
      (PulsePolarity[i]) ? (regval = regval | 0x00000100) : (regval = regval & ~(0x00000100));
      ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1064 | (i << 8),regval);

      // collect the whole event, independently from threshold or trigger polarity
      if (NoThreshold[i] == 1) {
	// w/o ZLE
	CAEN_DGTZ_ReadRegister(m_handle, 0x1064 | (i << 8), &regval);
	ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1064 | (i << 8), regval | (uint32_t)(0x80));
      } else {
        CAEN_DGTZ_ReadRegister(m_handle, 0x1064 | (i << 8), &regval);
        ret |= CAEN_DGTZ_WriteRegister(m_handle, 0x1064 | (i << 8), regval & 0xffffff7f);
      }
    }
  }
  
  return ret;

}
//______________________________________________________________________________
int
CaenV1725::MallocReadoutBuffer( void )
{
  /* WARNING: This malloc must be done after the digitizer programming */
  CAEN_DGTZ_ErrorCode ret_last = CAEN_DGTZ_MallocReadoutBuffer(m_handle, 
							      &buffer, &AllocatedSize);
  if (ret_last != CAEN_DGTZ_Success) {
    std::cout <<  "ERR_BUFF_MALLOC" << std::endl;
    return -1;
  }

  return 0;
}
//______________________________________________________________________________
void
CaenV1725::StartRunMode( void )
{
  // Start run mode
  CAEN_DGTZ_SWStartAcquisition(m_handle);

}
//______________________________________________________________________________
uint32_t
CaenV1725::ReadData( void )
{
  //ReadData
  CAEN_DGTZ_ErrorCode ret_last = CAEN_DGTZ_ReadData(m_handle, 
						    CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, 
						    buffer, &BufferSize);
  if (ret_last != CAEN_DGTZ_Success) {
    std::cout << "ERR_READOUT CAEN_DGTZ_ReadData" << std::endl;
    return 0;
  }

  if (BufferSize != 0) {
    ret_last = CAEN_DGTZ_GetNumEvents(m_handle, buffer, BufferSize, &NumOfEvents);
    if (ret_last != CAEN_DGTZ_Success) {
      std::cout << "ERR_READOUT CAEN_DGTZ_GetNumEvents" << std::endl;
      return 0;
    }
  }

  if (BufferSize>0)
    eventIndex = 0;
  else
    eventIndex = -1;

  return BufferSize;
}

//______________________________________________________________________________
uint32_t
CaenV1725::GetOneEvent( char *ptr )
{
  CAEN_DGTZ_EventInfo_t EventInfo;
  char *EventPtr;
  /* Get one event from the readout buffer */
  int ret = CAEN_DGTZ_GetEventInfo(m_handle, buffer, BufferSize, 
			 eventIndex, &EventInfo, &EventPtr);

  if (ret == 0) {
    memcpy(ptr, EventPtr, EventInfo.EventSize);

    //buffer_word = reinterpret_cast<uint32_t *>(EventPtr);
    /*
      for (uint32_t i=0; i<EventInfo.EventSize/4; i++) {
      printf("%x ", buffer_word[i]);
      std::cout << std::endl;
      } 
    */
    
    eventIndex++;
    if (eventIndex>=(int)NumOfEvents)
      eventIndex = -1;

    return EventInfo.EventSize/4;
  }

  //std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
  //std::cout << "Error in getting one event by CAEN_DGTZ_GetEventInfo" << std::endl;

  return 0;
}
//______________________________________________________________________________
void
CaenV1725::ClearData( void )
{
  CAEN_DGTZ_ClearData(m_handle);
}

//______________________________________________________________________________
void
CaenV1725::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;

}

}
