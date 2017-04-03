// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_NOTICE_TDC64M_HH
#define VME_NOTICE_TDC64M_HH

#include <stdint.h>
#include <string>
#include <vector>

#include <bit3/vmeslib.h>

#include "VmeModule.hh"

namespace vme
{
  /*
    Onboard module address switch allocated between A16-A23.
  */

//______________________________________________________________________________
class NoticeTDC64M : public VmeModule
{
public:
  NoticeTDC64M( uint32_t addr );
  ~NoticeTDC64M( void );

private:
  NoticeTDC64M( const NoticeTDC64M& );
  NoticeTDC64M& operator =( const NoticeTDC64M& );

private:
  static const int MapSize = 0x10000;
  volatile uint32_t *m_data_buf;

public:
  enum Register
    {
      Ctrl    = 0x00, // R/W [ 9:0]
      ClStat  = 0x04, // R/W [12:0]
      Enable1 = 0x08, // R/W [31:0]
      Enable2 = 0x0c, // R/W [31:0]
      Window  = 0x10, // R/W [31:0]
      Event   = 0x14, // R/- [ 7:0]
      Data    = 0x4000
    };

  /** Ctrl
   *  D5-9 : Module ID
   *  D4   : Edge Mode (0:leading 1:both)
   *  D1-3 : Dynamic Range (2^0-2^7 [us])
   *  D0   : reset (0:nothing 1:reset local event counter)
   *
   ** ClStat
   *  D12   : Data Ready Status (0:not 1:ready)
   *  D0-11 : Number of data word in the buffer (0-4095)
   *  - Write access to this address will clear previous data
   *
   ** Enable1
   *  D0-31 : Enable channel (0-31)
   *
   ** Enable2
   *  D0-31 : Enable channel (32-63)
   *
   ** Window
   *  D16-31 : Mask Window 8ns unit (0-128us)
   *  D0-15  : Search Window 8ns unit (0-128us)
   *  e.g.
   *    1us -> 1000/8   = 125
   *  128us -> 128000/8 = 16000
   *
   ** Event
   *  D7-0 : Event Counter (0-65535)
   */

  static const int GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void     Close( void );
  void     Open( void );
  uint32_t DataBuf( uint32_t i ) const;
  uint32_t ReadRegister( uint32_t reg ) const;
  void     WriteRegister( uint32_t reg, uint32_t val );
  void     Print( void ) const;

};

//______________________________________________________________________________
inline const std::string&
NoticeTDC64M::ClassName( void )
{
  static std::string g_name("NoticeTDC64M");
  return g_name;
}

//______________________________________________________________________________
inline uint32_t
NoticeTDC64M::ReadRegister( uint32_t reg ) const
{
  return *word32( m_map_hdl, reg );
}

}

#endif
