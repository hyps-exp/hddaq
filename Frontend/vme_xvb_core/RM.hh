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
      Event  = 0x00,
      Spill  = 0x04,
      Serial = 0x08,
      Dummy  = 0x0c,
      Input  = 0x10,
      Reset  = 0x14,
      Level  = 0x18,
      Pulse  = 0x1c,
      Time   = 0x20
    };

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
