// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_RM_HH
#define VME_RM_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class RM : public VmeModule
{
public:
  RM( GEF_UINT32 addr );
  ~RM( void );

private:
  RM( const RM& );
  RM& operator =( const RM& );

private:
  static const int MapSize = 0x10000;
  volatile GEF_UINT32 *m_offset;

public:
  enum Register
    {
      Event  = 0x00, // R/- [31:0]
      Spill  = 0x04, // R/- [31:0]
      Serial = 0x08, // R/- [31:0]
      Dummy  = 0x0c, // R/- [31:0]
      Input  = 0x10, // R/- [15:0]
      Reset  = 0x14, // -/W [0]
      Level  = 0x18, // R/W [15:0]
      Pulse  = 0x1c  // -/W [15:0]
      // Time   = 0x20
    };

  /** Event
   *  D31   : Lock Bit
   *  D11-0 : Event Number
   *
   ** Spill
   *  D31   : Lock Bit
   *  D7-0  : Spill Number
   *
   ** Serial
   *  D31-0 : Serial Number
   *          count the number of access
   *
   ** Dummy
   *
   ** Input
   *  D15-0 : Latch with 32MHz clock
   *
   ** Reset
   *  Write access to this address will clear Input register
   *
   ** Level
   *  D15-0 : Level output (1:high,0:low)
   *
   ** Pulse
   *  D15-0 : Pulse output with the width of 33 ns
   *
   ** Time
   *  not supported in the current firmware of VMR-RM
   */

  static const int GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void Open( void );
  void InitRegister( const GEF_MAP_PTR& ptr, int index );
  GEF_UINT32 ReadRegister( GEF_UINT32 reg ) const;
  void       WriteRegister( GEF_UINT32 reg, GEF_UINT32 val );
  void       Print( void ) const;

};

//______________________________________________________________________________
inline const std::string&
RM::ClassName( void )
{
  static std::string g_name("RM");
  return g_name;
}

//______________________________________________________________________________
inline GEF_UINT32
RM::ReadRegister( GEF_UINT32 reg ) const
{
  return __bswap_32( *(m_offset+reg/GEF_VME_DWIDTH_D32) );
}

}

#endif
