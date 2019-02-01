// -*- C++ -*-

// Author: Yoshiyuki Nakada

#ifndef VME_CAEN_V1724_HH
#define VME_CAEN_V1724_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class CaenV1724 : public VmeModule
{
public:
  CaenV1724( GEF_UINT32 addr );
  ~CaenV1724( void );

private:
  CaenV1724( const CaenV1724& );
  CaenV1724& operator =( const CaenV1724& );

private:
  static const GEF_UINT32 MapSize = 0x10000;
  volatile GEF_UINT32 *m_data_buf;
  volatile GEF_UINT32 *m_offset;
  

public:
  static const int          NofCh = 8;
  static const unsigned int IReg  = 0x100U;

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

  

  static const GEF_UINT32 GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void       Open( void );
  GEF_UINT32 DataBuf( void );
  void       InitRegister( const GEF_MAP_PTR& ptr, int index );
  GEF_UINT32 ReadRegister( GEF_UINT32 reg ) const;
  void       WriteRegister( GEF_UINT32 reg, GEF_UINT32 val );
  void       Print( void ) const;
};

//______________________________________________________________________________
inline const std::string&
CaenV1724::ClassName( void )
{
  static std::string g_name("CaenV1724");
  return g_name;
}

//______________________________________________________________________________
inline GEF_UINT32
CaenV1724::ReadRegister( GEF_UINT32 reg ) const
{
  return __bswap_32( *(m_offset+reg/GEF_VME_DWIDTH_D32) );
}

}

#endif
