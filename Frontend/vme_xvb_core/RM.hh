
#ifndef VME_RM_HH
#define VME_RM_HH

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
  RM( GEF_UINT64 addr );
  ~RM( void );

private:
  RM( const RM& );
  RM& operator =( const RM& );

private:
  static const int MapSize = 0x10000;
  volatile GEF_UINT32 *m_offset;
  volatile GEF_UINT32 *m_event;
  volatile GEF_UINT32 *m_spill;
  volatile GEF_UINT32 *m_serial;
  volatile GEF_UINT32 *m_dummy;
  volatile GEF_UINT32 *m_input;
  volatile GEF_UINT32 *m_reset;
  volatile GEF_UINT32 *m_level;
  volatile GEF_UINT32 *m_pulse;
  volatile GEF_UINT32 *m_time;

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
  GEF_UINT32 ReadRegister( GEF_UINT32 reg );
  void       WriteRegister( GEF_UINT32 reg, GEF_UINT32 val );
  void       Print( void );

};

//______________________________________________________________________________
inline const std::string&
RM::ClassName( void )
{
  static std::string g_name("RM");
  return g_name;
}

}

#endif
