// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_RPV130_HH
#define VME_RPV130_HH

#include <stdint.h>
#include <string>
#include <vector>

#include <bit3/vmeslib.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class RPV130 : public VmeModule
{
public:
  RPV130( uint32_t addr );
  ~RPV130( void );

private:
  RPV130( const RPV130& );
  RPV130& operator =( const RPV130& );

private:
  static const int MapSize = 0x100;

public:
  enum Register
    {
      Latch1  = 0x0, // R/- [7:0]
      Latch2  = 0x2, // R/- [7:0]
      Rsff    = 0x4, // R/- [7:0]
      Through = 0x6, // R/- [7:0]
      Pulse   = 0x8, // -/W [7:0]
      Level   = 0xa, // -/W [7:0]
      Csr1    = 0xc, // R/W [7:0]
      Csr2    = 0xe  // R/W [7:0]
    };

  /** CSR1 [Read]    [Write]
   *  D7 : BUSY3     n/a
   *  D6 : ENABLE3   ENABLE3
   *  D5 : BUSY1     n/a
   *  D4 : ENABLE1   ENABLE1
   *  D3 : MASK1     MASK1
   *  D2 : n/a       n/a
   *  D1 : n/a       CLR1
   *  D0 : n/a       CLR3
   *
   ** CSR2 [Read]    [Write]
   *  D7 : BUSY3     n/a
   *  D6 : ENABLE3   ENABLE3
   *  D5 : BUSY2     n/a
   *  D4 : ENABLE2   ENABLE2
   *  D3 : MASK2     MASK2
   *  D2 : n/a       n/a
   *  D1 : n/a       CLR2
   *  D0 : n/a       CLR3
   */

  static const int GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void     Close( void );
  void     Open( void );
  uint16_t ReadRegister( uint16_t reg ) const;
  void     WriteRegister( uint16_t reg, uint16_t val );
  void     Print( void ) const;

};

//______________________________________________________________________________
inline const std::string&
RPV130::ClassName( void )
{
  static std::string g_name("RPV130");
  return g_name;
}

//______________________________________________________________________________
inline uint16_t
RPV130::ReadRegister( uint16_t reg ) const
{
  return *word16( m_map_hdl, reg );
}

}

#endif
