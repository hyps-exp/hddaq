// -*- C++ -*-

// Author: Yoshiyuki Nakada

#ifndef OPT_CAEN_V1743_HH
#define OPT_CAEN_V1743_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "OptlinkModule.hh"

namespace opt
{

#define MAX_GR  8          /* max. number of groups for board */
#define MAX_CH  16         /* max. number of channels for board */

  //#define MAX_NUM_EVENTS_BLT 1000 /* maximum number of events to read out in one Block Transfer (range from 1 to 1023) */
#define MAX_NUM_EVENTS_BLT 1 /* maximum number of events to read out in one Block Transfer (range from 1 to 1023) */

#define MIN_DAC_RAW_VALUE       -1.25
#define MAX_DAC_RAW_VALUE       +1.25

  typedef enum {
    SYSTEM_TRIGGER_SOFT,
    SYSTEM_TRIGGER_NORMAL,
    SYSTEM_TRIGGER_AUTO,
    SYSTEM_TRIGGER_EXTERN
  } TriggerType_t;

  typedef enum {
    START_SW_CONTROLLED,
        START_HW_CONTROLLED
  } StartMode_t;

  typedef enum {
    COMMONT_EXTERNAL_TRIGGER_TRGIN_TRGOUT,
    INDIVIDUAL_TRIGGER_SIN_TRGOUT,
    TRIGGER_ONE2ALL_EXTOR,
  } SyncMode_t;

  typedef struct {
    int TriggerDelay;
  } V1743Group_t;

  typedef struct {
    char ChannelEnable;
    int EnablePulseChannels;
    unsigned short PulsePattern;
    float DCOffset; //in volt                                                                                                                                                       
    float m;
    float q;
    int ChannelTriggerEnable;
    float TriggerThreshold_v;
    float TriggerThreshold_adc;
    CAEN_DGTZ_TriggerPolarity_t TriggerPolarity;
    char PlotEnable;

    int DiscrMode;                  // Discriminator Mode: 0=LED, 1=CFD   
    int NsBaseline;                 // Num of Samples for the input baseline calculation
    float GateWidth;                // Gate Width (in ns) 
    float PreGate;                  // Pre Gate (in ns)
    float CFDdelay;                 // CFD delay (in ns)
    float CFDatten;                 // CFD attenuation (between 0.0 and 1.0)
    int CFDThreshold;
    int TTFsmoothing;               // Smoothing factor in the trigger and timing filter (0 = disable) 
    // Energy Gain and Calibration
    float EnergyCoarseGain; // Energy Coarse Gain (requested by the user); can be a power of two (1, 2, 4, 8...) or a fraction (0.5, 0.25, 0.125...)
    float EnergyFineGain;   // Energy Fine Gain (requested by the user)
    uint16_t EnergyDiv;             // Energy Divisor to get the requested CoarseGain and Rebinning
    float EnergyOffset;             // Energy Offset
    float ECalibration_m;   // Energy Calibration slope (y=mx+q)
    float ECalibration_q;   // Energy Calibration offset (y=mx+q) 
  } V1743Channel_t;

  typedef struct {
    char Enable;
    int LinkType;
    int LinkNum;
    int ConetNode;
    uint32_t BaseAddress;
    uint32_t RecordLength;
    TriggerType_t TriggerType;
    CAEN_DGTZ_SAMFrequency_t SamplingFrequency;
    CAEN_DGTZ_SAM_CORRECTION_LEVEL_t CorrectionLevel;
    CAEN_DGTZ_IOLevel_t FPIOtype;
    int GWn;
    V1743Group_t groups[MAX_GR];
    V1743Channel_t channels[MAX_CH];
    int RefChannel; // reference channel 
  } V1743Board_t;

//______________________________________________________________________________
class CaenV1743 : public OptlinkModule
{
public:
  CaenV1743( int link_num, int conet_node, uint32_t base_addr );
  ~CaenV1743( void );

private:
  CaenV1743( const CaenV1743& );
  CaenV1743& operator =( const CaenV1743& );
  V1743Board_t v1743config;

public:
  static const int          NofCh = 16;
  static const int          NofGroup = 8;
  static const unsigned int IReg  = 0x100U;
  char                      *buffer;
  uint32_t                  BufferSize;
  uint32_t                  AllocatedSize;
  uint32_t                  NumOfEvents;
  int                       eventIndex;



  enum Register
    {
      // Data buffer ----------------------------
      DataBuffer     = 0x0000U,
      // Indivisual registers -------------------
      ZeroSuppThre   = 0x1024U,
      ZeroSuppSample = 0x1028U,
      TrigThre       = 0x1080U,
      TimeThre       = 0x1084U,
      ChStatus       = 0x1088U,
      FwRevision     = 0x108CU,
      DcOffset       = 0x1098U,
      // Common registers -----------------------
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
      BoardInfo      = 0x8140U,
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
  void                SetRecordLength( uint32_t len );
  bool                SetDelay( int delay );
  bool                SetSamplingFrequency( CAEN_DGTZ_SAMFrequency_t frequency );
  bool                SetChannelEnable( int ch);
  bool                SetChannelDisable( int ch);
  bool                SetDCOffset( int ch, float offset);
  int                 ProgramRegister( void );
  int                 MallocReadoutBuffer( void );
  void                StartRunMode( void );
  uint32_t            ReadData( void );
  int                 GetEventIndex( void ) { return eventIndex; };
  void                SetEventIndex( int index ) { eventIndex=index; };
  uint32_t            GetOneEvent( char *ptr );
  void                Print( void ) const;
  uint32_t            ReadRegister( uint32_t addr ) const;
  void                WriteRegister( uint32_t addr, uint32_t reg );
  void                ClearData( void );
};

//______________________________________________________________________________
inline const std::string&
CaenV1743::ClassName( void )
{
  static std::string g_name("CaenV1743");
  return g_name;
}

//______________________________________________________________________________
inline uint32_t
CaenV1743::ReadRegister( uint32_t addr ) const
{
  uint32_t reg;
  CAENComm_Read32(m_handle, addr, &reg);
  return reg;
}

}

#endif
