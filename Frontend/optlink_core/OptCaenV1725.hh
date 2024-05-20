// -*- C++ -*-

// Author: Koji Miwa

#ifndef OPT_CAEN_V1725_HH
#define OPT_CAEN_V1725_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "OptlinkModule.hh"

namespace opt
{
  //______________________________________________________________________________
class CaenV1725 : public OptlinkModule
{
public:
  CaenV1725( int link_num, int conet_node, uint32_t base_addr );
  ~CaenV1725( void );

private:
  CaenV1725( const CaenV1725& );
  CaenV1725& operator =( const CaenV1725& );

public:
  static const int          NofCh = 16;
  static const unsigned int IReg  = 0x100U;
  char                      *buffer;
  uint32_t                  BufferSize;
  uint32_t                  AllocatedSize;
  uint32_t                  NumOfEvents;
  int                       eventIndex;

  uint16_t EnableMask;
  CAEN_DGTZ_BoardInfo_t BoardInfo;
  // Parameters for the digitizer
  CAEN_DGTZ_IOLevel_t FPIOtype;
  CAEN_DGTZ_TriggerMode_t SWTriggerMode;
  CAEN_DGTZ_TriggerMode_t ExtTriggerMode;
  CAEN_DGTZ_AcqMode_t StartMode;
  uint32_t GainFactor;
  uint32_t RecordLength;
  uint16_t TrigCoupleMask;
  uint16_t TrigOutCoupleMask;
  uint32_t TrigCoupleLogic[NofCh/2];
  int PulsePolarity[NofCh];
  int preTrgg[NofCh];
  int NSampBck[NofCh];
  int NSampAhe[NofCh];
  int ZleTrigThr[NofCh];
  int ZleThr[NofCh];
  int BLineMode[NofCh];
  int BLineDefValue[NofCh];
  int BLineNoise[NofCh];
  int DCoffset[NofCh];
  int NoThreshold[NofCh];
  int TP_Enable[NofCh];
  int TP_Polarity[NofCh];
  int TP_Rate[NofCh];
  int TP_Scale[NofCh];


  enum Register
    {
      // Data buffer ----------------------------
      DataBuffer     = 0x0000U,
      // Indivisual registers -------------------
      nInputDynamicRange = 0x1028U,
      nZLE_Baseline      = 0x1034U,
      nPreTrigger        = 0x1038U,
      nSamplesLookBack   = 0x1054U,
      nSamplesLookForward = 0x1058U,
      nZLE_Threshold      = 0x105CU,
      nTriggerThreshold   = 0x105CU,
      nInputControl       = 0x105CU,
      nCoupleTriggerLogic = 0x1068U,
      nChannelStatus      = 0x1088U,
      nAMCFirmware        = 0x108CU,
      nDCOffset           = 0x1098U,
      nChADCTemp          = 0x10A8U,            
      // Common registers -----------------------
      InputDynamicRange  = 0x8028U,
      ZLE_Baseline       = 0x8034U,
      PreTrigger         = 0x8038U,
      SamplesLookBack    = 0x8054U,
      SamplesLookForward = 0x8058U,
      ZLE_Threshold      = 0x805CU,
      TriggerThreshold   = 0x805CU,
      InputControl       = 0x805CU,
      CoupleTriggerLogic = 0x8068U,
      DCOffset           = 0x8098U,


      BoardConf      = 0x8000U,
      BitSet         = 0x8004U,
      BitClear       = 0x8008U,
      BufOrg         = 0x800CU,
      CustomSize     = 0x8020U,
      InspectMode    = 0x802AU,
      DecimFactor    = 0x8044U,
      AcqCtrl        = 0x8100U,
      AcqStatus      = 0x8104U,
      SoftTrig       = 0x8108U,
      TrigMask       = 0x810CU,
      GPOEnMask      = 0x8110U,
      PostTrig       = 0x8114U,
      LVDSData       = 0x8118U,
      IOCtrl         = 0x811CU,
      ChEnMask       = 0x8120U,
      RocFwRevision  = 0x8124U,
      EventStored    = 0x812CU,
      VlevelModeConf = 0x8138U,
      SoftClockSync  = 0x813CU,
      //BoardInfo      = 0x8140U,
      AnalogMonMode  = 0x8144U,
      EventSize      = 0x814CU,
      IncpectConf    = 0x8150U,
      FanSpeed       = 0x8168U,
      MemoryAfullLv  = 0x816CU,
      StartStopDelay = 0x8170U,
      FailureStatus  = 0x8178U,
      IONewFeature   = 0x81A0U,
      BufOccGain     = 0x81B4U,
      ExtendVetoDelay= 0x81C4U,
      ReadoutCtrl    = 0xEF00U,
      ReadoutStatus  = 0xEF04U,
      BoardID        = 0xEF08U,
      MSCTAddress    = 0xEF0CU,
      RelocationAddr = 0xEF10U,
      InterruptStatus= 0xEF14U,
      InterruptEvNum = 0xEF18U,
      MaxNofEvBLT    = 0xEF1CU,
      Scratch        = 0xEF20U,
      SoftReset      = 0xEF24U,
      SoftClear      = 0xEF28U,
      ConfReload     = 0xEF34U
    };

  
  static const std::string& ClassName( void );
  CAENComm_ErrorCode  Close( void );
  CAENComm_ErrorCode  Open( void );
  void                SetDefaultConfig( void );
  bool                SetChannelEnable( uint16_t mask);
  void                SetRecordLength( uint32_t len );
  bool                SetDCOffset( int ch, int offset);
  bool                SetNoZLEMode( int ch, int value);
  bool                SetPreTriggerSamples( int channel, uint32_t samples );
  bool                SetPreSamples( int channel, uint32_t samples );
  bool                SetPostSamples( int channel, uint32_t samples );
  bool                SetBLineDefValue( int channel, uint32_t baseline );
  bool                SetDataThreshold( int channel, uint16_t threshold );
  bool                SetTriggerThreshold( int channel, uint16_t threshold );
  int                 ProgramRegister( void );
  int                 MallocReadoutBuffer( void );
  void                StartRunMode( void );
  uint32_t            ReadData( void );
  int                 GetEventIndex( void ) { return eventIndex; };
  void                SetEventIndex( int index ) { eventIndex=index; };
  uint32_t            GetOneEvent( char *ptr );
  void                Print( void ) const;
  void                ClearData( void );
};

//______________________________________________________________________________
inline const std::string&
CaenV1725::ClassName( void )
{
  static std::string g_name("CaenV1725");
  return g_name;
}


}

#endif
