// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_CAEN_V830_HH
#define VME_CAEN_V830_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class CaenV830 : public VmeModule
{
public:
  CaenV830( GEF_UINT32 addr );
  ~CaenV830( void );

private:
  CaenV830( const CaenV830& );
  CaenV830& operator =( const CaenV830& );

private:
  static const GEF_UINT32 MapSize = 0x10000;
  volatile GEF_UINT32 *m_counter;
  volatile GEF_UINT32 *m_enable;
  volatile GEF_UINT16 *m_offset16;
  volatile GEF_UINT32 *m_offset32;

public:
  enum Register
    {
      Counter   = 0x1000U, // R/- [31:0]
      ChEnable  = 0x1100U, // R/W [31:0]
      Ctrl      = 0x1108U, // R/W [7:0]
      BitSet    = 0x110aU, // -/W [7:0]
      BitClr    = 0x110cU, // -/W [7:0]
      Status    = 0x110eU, // R/- [7:0]
      GeoAddr   = 0x1110U, // R/W [4:0]
      EventNum  = 0x1130U, // R/W [4:0]
      SoftReset = 0x1120U, // -/W [4:0]
      SoftClr   = 0x1122U, // -/W [4:0]
      SoftTrig  = 0x1124U, // -/W [4:0]
    };

  static const GEF_UINT32 GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void       Open( void );
  GEF_UINT32 DataBuf( void );
  void       InitRegister( const GEF_MAP_PTR& ptr, int index );
  GEF_UINT32 ReadCounter( GEF_UINT32 channel ) const;
  GEF_UINT16 ReadRegister16( GEF_UINT16 reg ) const;
  GEF_UINT32 ReadRegister32( GEF_UINT32 reg ) const;
  void       WriteRegister16( GEF_UINT16 reg, GEF_UINT16 val );
  void       WriteRegister32( GEF_UINT32 reg, GEF_UINT32 val );
  void       Print( void ) const;
};

//______________________________________________________________________________
inline const std::string&
CaenV830::ClassName( void )
{
  static std::string g_name("CaenV830");
  return g_name;
}

//______________________________________________________________________________
inline GEF_UINT16
CaenV830::ReadRegister16( GEF_UINT16 reg ) const
{
  return __bswap_16( *(m_offset16+reg/GEF_VME_DWIDTH_D16) );
}

//______________________________________________________________________________
inline GEF_UINT32
CaenV830::ReadRegister32( GEF_UINT32 reg ) const
{
  return __bswap_32( *(m_offset32+reg/GEF_VME_DWIDTH_D32) );
}

}

#endif
