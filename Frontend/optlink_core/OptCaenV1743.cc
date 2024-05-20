// -*- C++ -*-

// Author: Shuhei Hayakawa
// Modified by Koji Miwa for V1743

#include "OptCaenV1743.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <string.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"


namespace opt
{

//______________________________________________________________________________
CaenV1743::CaenV1743( int link_num, int conet_node, uint32_t base_addr )
  : OptlinkModule(link_num, conet_node, base_addr), buffer(NULL), AllocatedSize(0), eventIndex(-1)
{
}

//______________________________________________________________________________
CaenV1743::~CaenV1743( void )
{
}

//______________________________________________________________________________
CAENComm_ErrorCode
CaenV1743::Open( void )
{
  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, m_link_num, m_conet_node, 0, &m_handle);

  if (ret == CAEN_DGTZ_Success)
    return CAENComm_Success;

  return CAENComm_CommError;
}

//______________________________________________________________________________
CAENComm_ErrorCode
CaenV1743::Close( void )
{
  //FreeReadoutBuffer 
  if (buffer)
    CAEN_DGTZ_FreeReadoutBuffer(&buffer);

  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_CloseDigitizer( m_handle );

  if (ret == CAEN_DGTZ_Success)
    return CAENComm_Success;

  return CAENComm_CommError;
}

//______________________________________________________________________________
void
CaenV1743::SetDefaultConfig( void )
{
  memset(&v1743config, 0, sizeof(V1743Board_t));

  v1743config.Enable = 0;
  v1743config.RefChannel = 0;
  v1743config.RecordLength = 1024;                                            
  v1743config.SamplingFrequency = CAEN_DGTZ_SAM_3_2GHz;
  //v1743config.CorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_DISABLED;
  //v1743config.CorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_PEDESTAL_ONLY;
  //v1743config.CorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_INL;
  v1743config.CorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_ALL;
  v1743config.TriggerType = SYSTEM_TRIGGER_EXTERN;
  //v1743config.TriggerType = SYSTEM_TRIGGER_NORMAL;

  v1743config.FPIOtype = CAEN_DGTZ_IOLevel_NIM;

  v1743config.GWn = 0;

  for (int g = 0; g < MAX_GR; g++) {
    // get pointer to substructure                                              
    V1743Group_t *WDg = &v1743config.groups[g];
    // Post_Trigger                                                             
    // POST_TRIGGER: post trigger size in percent of the whole acquisition window 
    // options: 0 to 100 (%)
    //WDg->TriggerDelay = 50;
    //WDg->TriggerDelay = 37;
    //WDg->TriggerDelay = 32;
    //WDg->TriggerDelay = 27;
    //WDg->TriggerDelay = 18; //<--
    WDg->TriggerDelay = 35; //<--
  }

  for (int c = 0; c < MAX_CH; c++) {
    // get pointer to substructure                                              
    V1743Channel_t *WDc = &v1743config.channels[c];
    // assign defaults                                                          
    WDc->ChannelEnable = 0;
    WDc->EnablePulseChannels = 0;
    WDc->PulsePattern = 1;
    WDc->DCOffset = 0;
    WDc->m = 1;
    WDc->q = 0;
    if (c==MAX_CH-1)
      WDc->ChannelTriggerEnable = 1;

    WDc->TriggerPolarity = CAEN_DGTZ_TriggerOnFallingEdge;
    WDc->TriggerThreshold_v = 0;
    if (c==MAX_CH-1)
      WDc->TriggerThreshold_v = -0.5;

    WDc->CFDThreshold = -1;

    WDc->DiscrMode = 1;
    WDc->NsBaseline = 10;
    WDc->GateWidth = 0;
    WDc->PreGate = 0;

    WDc->CFDatten = 1.0;
    WDc->TTFsmoothing = 0;

    WDc->ECalibration_m = 1.0;
    WDc->ECalibration_q = 0;
  }

}
//______________________________________________________________________________
void
CaenV1743::SetRecordLength( uint32_t len )
{
  uint32_t length = len;
  if (length % 16 != 0) {
    uint32_t warizan = length/16;
    length = 16*warizan;
    std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
    std::cout << "Record length should be multiple number of 16" << std::endl;
    std::cout << "Record length was set to " << length << std::endl;
  }
  v1743config.RecordLength = length;

}
//______________________________________________________________________________
bool
CaenV1743::SetDelay( int delay )
{
  if (delay>0 && delay<100) {
    for (int g = 0; g < MAX_GR; g++) {
      // get pointer to substructure
      V1743Group_t *WDg = &v1743config.groups[g];
      // Post_Trigger                                                                // POST_TRIGGER: post trigger size in percent of the whole acquisition window 
      // options: 0 to 100 (%)
      WDg->TriggerDelay = delay; //<--
    }
    return true;
  }

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
  std::cout << "Invalid delay (should be 0-100) " << delay << std::endl;

  return false;
}
//______________________________________________________________________________
bool
CaenV1743::SetSamplingFrequency( CAEN_DGTZ_SAMFrequency_t frequency )
{
  if (frequency == CAEN_DGTZ_SAM_3_2GHz ||
      frequency == CAEN_DGTZ_SAM_1_6GHz ||
      frequency == CAEN_DGTZ_SAM_800MHz ||
      frequency == CAEN_DGTZ_SAM_400MHz ) {

    v1743config.SamplingFrequency = frequency;
    
    return true;
  }

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
  std::cout << "Invalid Sampling Frequency " << frequency << std::endl;

  return false;
}
//______________________________________________________________________________
bool
CaenV1743::SetChannelEnable( int ch)
{
  if (ch >=0 && ch < NofCh) {
    V1743Channel_t *WDc = &v1743config.channels[ch];
    WDc->ChannelEnable = 1;
    
    return true;
  }

  return false;
}
//______________________________________________________________________________
bool
CaenV1743::SetChannelDisable( int ch)
{
  if (ch >=0 && ch < NofCh) {
    V1743Channel_t *WDc = &v1743config.channels[ch];
    WDc->ChannelEnable = 0;
    
    return true;
  }

  return false;
}

//______________________________________________________________________________
bool
CaenV1743::SetDCOffset( int ch, float offset)
{
  if (ch >=0 && ch < NofCh) {
    V1743Channel_t *WDc = &v1743config.channels[ch];
    WDc->DCOffset = offset;
    
    return true;
  }

  return false;
}

//______________________________________________________________________________
int
CaenV1743::ProgramRegister( void )
{
  int ret = 0;
  /* reset the digitizer */
  ret |= CAEN_DGTZ_Reset(m_handle);
  if (ret != 0) {
    std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
    std::cout <<  "Error: Unable to reset digitizer." << std::endl;
    std::cout << "Please reset digitizer manually then restart the program" << std::endl;
    return -1;
  }

  //Board Fail Status
  uint32_t d32;
  ret |= CAEN_DGTZ_ReadRegister(m_handle, 0x8178, &d32);
  if ((d32 & 0xF) != 0) {
    std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
    std::cout << "Error: Internal Communication Timeout occurred." << std::endl;
    std::cout << "Please reset digitizer manually then restart the program" << std::endl;
    return -1;
  }

  // Front Panel IO control
  ret |= CAEN_DGTZ_ReadRegister(m_handle, opt::CaenV1743::IOCtrl, &d32);
  // Trig Out port --> Busy out
  d32 = d32 | 0x000d0000;
  ret |= CAEN_DGTZ_WriteRegister(m_handle, opt::CaenV1743::IOCtrl, d32);


  /* Set Group Enable Mask*/
  int groupsMask = 0;
  for (int c = 0; c < NofCh; c++) {
    if (v1743config.channels[c].ChannelEnable)
      groupsMask |= (1 << (c / 2));
  }
  ret |= CAEN_DGTZ_SetGroupEnableMask(m_handle, groupsMask);

  /* Set Trigger Delay */
  for (int samIndex = 0; samIndex < MAX_GR; samIndex++) {
    ret |= CAEN_DGTZ_SetSAMPostTriggerSize(m_handle, samIndex, 
					   v1743config.groups[samIndex].TriggerDelay & 0xFF);
  }

  /* Set Sampling Frequency */
  ret |= CAEN_DGTZ_SetSAMSamplingFrequency(m_handle, v1743config.SamplingFrequency);

  /* Set Pulser Parameters */
  for (int c = 0; c < NofCh; c++) {
    if (v1743config.channels[c].EnablePulseChannels == 1)
      ret |= CAEN_DGTZ_EnableSAMPulseGen(m_handle, c, 
					 v1743config.channels[c].PulsePattern, 
					 CAEN_DGTZ_SAMPulseCont);
    else
      ret |= CAEN_DGTZ_DisableSAMPulseGen(m_handle, c);
  }
  
  /* Set Trigger Threshold */
  for (int c = 0; c < NofCh; c++) {
    float valF = v1743config.channels[c].TriggerThreshold_v + v1743config.channels[c].DCOffset;
    int reg_val = (int)((MAX_DAC_RAW_VALUE - valF) / 
			(MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 65535);  // Inverted Range
    ret |= CAEN_DGTZ_SetChannelTriggerThreshold(m_handle, c, reg_val);
  }
  /* Set Trigger Source */
  int channelsMask = (1 << NofCh) - 1; //all channels of the board   
  ret |= CAEN_DGTZ_SetChannelSelfTrigger(m_handle, 
					 CAEN_DGTZ_TRGMODE_DISABLED, 
					 channelsMask); //disable self trigger on all channels  
  switch (v1743config.TriggerType) {
  case SYSTEM_TRIGGER_SOFT:
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(m_handle, CAEN_DGTZ_TRGMODE_DISABLED);
    break;
  case SYSTEM_TRIGGER_NORMAL:
    channelsMask = 0;
    for (int i = 0; i < NofCh / 2; i++)
      channelsMask += (v1743config.channels[i * 2].ChannelTriggerEnable + 
		       ((v1743config.channels[i * 2 + 1].ChannelTriggerEnable) << 1)) << (2 * i);
    ret |= CAEN_DGTZ_SetChannelSelfTrigger(m_handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, channelsMask);
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(m_handle, CAEN_DGTZ_TRGMODE_DISABLED);
    break;
  case SYSTEM_TRIGGER_AUTO:
    channelsMask = 0;
    for (int i = 0; i < NofCh / 2; i++)
      channelsMask += (v1743config.channels[i * 2].ChannelTriggerEnable + 
		       ((v1743config.channels[i * 2 + 1].ChannelTriggerEnable) << 1)) << (2 * i);
    ret |= CAEN_DGTZ_SetChannelSelfTrigger(m_handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, channelsMask);
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(m_handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    break;
  case SYSTEM_TRIGGER_EXTERN:
    ret |= CAEN_DGTZ_SetSWTriggerMode(m_handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(m_handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    channelsMask = 0;
    for (int i = 0; i < NofCh / 2; i++)
      channelsMask += (v1743config.channels[i * 2].ChannelTriggerEnable + 
		       ((v1743config.channels[i * 2 + 1].ChannelTriggerEnable) << 1)) << (2 * i);
    ret |= CAEN_DGTZ_SetChannelSelfTrigger(m_handle, 
					   CAEN_DGTZ_TRGMODE_EXTOUT_ONLY, channelsMask);
    break;
  }

  /* Set Trigger Polarity */
  for (int c = 0; c < NofCh; c++) {
    ret |= CAEN_DGTZ_SetTriggerPolarity(m_handle, c, 
					v1743config.channels[c].TriggerPolarity);
  }

  /* Set Channel DC Offset */
  for (int c = 0; c < NofCh; c++) {
    float valF = v1743config.channels[c].DCOffset;
    int reg_val = (int)((MAX_DAC_RAW_VALUE + valF) / 
			(MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 65535);  // Inverted Range 
    ret |= CAEN_DGTZ_SetChannelDCOffset(m_handle, c, reg_val);
    //sleep(1);
  }

  /* Set Correction Level */
  ret |= CAEN_DGTZ_SetSAMCorrectionLevel(m_handle, v1743config.CorrectionLevel);

  /* Set MAX NUM EVENTS */
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(m_handle, MAX_NUM_EVENTS_BLT);

  /* Set Recording Depth */
  ret |= CAEN_DGTZ_SetRecordLength(m_handle, v1743config.RecordLength);

  /* Set Front Panel I/O Control */
  ret |= CAEN_DGTZ_SetIOLevel(m_handle, v1743config.FPIOtype);

  ret |= CAEN_DGTZ_SetAcquisitionMode(m_handle, CAEN_DGTZ_SW_CONTROLLED);

  if (ret) {
    std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;
    std::cout << "WARN: there were errors when configuring the digitizer." << std::endl;
    std::cout  << "\tSome settings may not be executed" << std::endl;
  }

  return 0;


}
//______________________________________________________________________________
int
CaenV1743::MallocReadoutBuffer( void )
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
CaenV1743::StartRunMode( void )
{
  // Start run mode
  CAEN_DGTZ_SWStartAcquisition(m_handle);

}
//______________________________________________________________________________
uint32_t
CaenV1743::ReadData( void )
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
    eventIndex=0;
  else
    eventIndex=-1;

  return BufferSize;
}

//______________________________________________________________________________
uint32_t
CaenV1743::GetOneEvent( char *ptr )
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
CaenV1743::WriteRegister( uint32_t addr, uint32_t reg )
{
  CAENComm_Write32(m_handle, addr, reg);
}
//______________________________________________________________________________
void
CaenV1743::ClearData( void )
{
  CAEN_DGTZ_ClearData(m_handle);
}

//______________________________________________________________________________
void
CaenV1743::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl;

}

}
