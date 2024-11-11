// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_CAEN_V775_HH
#define VME_CAEN_V775_HH

#include <stdint.h>
#include <string>
#include <vector>

#include <bit3/vmeslib.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class CaenV775 : public VmeModule
{
public:
  CaenV775( uint32_t addr );
  ~CaenV775( void );

private:
  CaenV775( const CaenV775& );
  CaenV775& operator =( const CaenV775& );

private:
  static const uint32_t MapSize = 0x10000;
  volatile uint32_t *m_data_buf;

public:
  enum Register
    {
      GeoAddr   = 0x1002U, // R/W [4:0]
      ChainAddr = 0x1004U, // R/W [7:0]
      BitSet1   = 0x1006U, // R/W [7,4:3]
      BitClr1   = 0x1008U, // R/W [7,4:3]
      Str1      = 0x100eU, // R/- [8:0]
      ChainCtrl = 0x101aU, // R/W [1:0]
      BitSet2   = 0x1032U, // R/W [14:0]
      BitClr2   = 0x1034U, // -/W [14:0]
      EvReset   = 0x1040U, // -/W [0]
      Range     = 0x1060U  // R/W [7:0]
    };

  static const uint32_t GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void     Close( void );
  void     Open( void );
  uint32_t DataBuf( void );
  uint16_t ReadRegister( uint16_t reg ) const;
  void     WriteRegister( uint16_t reg, uint16_t val );
  void     Print( void ) const;
};

//______________________________________________________________________________
inline const std::string&
CaenV775::ClassName( void )
{
  static std::string g_name("CaenV775");
  return g_name;
}

//______________________________________________________________________________
inline uint16_t
CaenV775::ReadRegister( uint16_t reg ) const
{
  return *word16( m_map_hdl, reg );
}

}

#endif