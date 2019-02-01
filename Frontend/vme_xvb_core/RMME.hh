// -*- C++ -*-

// Author: Ryotaro Honda
// Receiver module with Ikeda's FW.
// The multi-event buffer is implemented.

#ifndef VME_RMME_HH
#define VME_RMME_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "VmeModule.hh"

namespace vme
{

//_____________________________________________________________________________
class RMME : public VmeModule
{
public:
  RMME( GEF_UINT32 addr );
  ~RMME( void );

private:
  RMME( const RMME& );
  RMME& operator =( const RMME& );

private:
  static const int MapSize = 0x10000;
  volatile GEF_UINT32 *m_offset;

public:
  enum Register
    {
      FifoData   = 0x00, // R/- [31:0], Multi-event (ME) buffer data out
      WriteCount = 0x04, // R/- [31:0], ME buf Write count
      Serial     = 0x08, // R/- [31:0], Local event counter (# of data read)
      FifoDepth  = 0x0c, // R/- [31:0], ME buf full threshold to assert busy
      Input      = 0x10, // R/- [15:0], Input register (Ex-NIMIN)
      Control    = 0x14, // -/W [31:0], Control registers
      Level      = 0x18, // R/W [15:0], Output register from Ex-NIMOUT
      Pulse      = 0x1c  // -/W [15:0], Output register from Ex-NIMOUT
    };

  enum CtrlReg
    {
      regFifoReset   = 0x1,
      regInputReset  = 0x2,
      regSerialReset = 0x4,
      regSelNIM4     = 0x10000,
      regDaqGate     = 0x20000
    };

  static const int shift_enc  = 16;
  static const int mask_enc   = 0xfff;

  static const int shift_snc  = 0;
  static const int mask_snc   = 0xff;

  static const int mask_lock  = 0x80000000U;

  /** FifoData
   *  D31   : Lock Bit
   *  D30-28: NC
   *  D27-16: Event number (12bit)
   *  D15   : Lock Bit (identical to D31)
   *  D14-8 : NC
   *  D7-0  : Spill number (8bit)
   *
   ** Write Count
   *  D31-10: NC
   *  D9-0  : ME buf write count (10bit)
   *
   ** Serial
   *  D31-0 : Serial Number
   *          count the number of access
   *
   ** FifoDepth
   *  Full threshold of multi-event buffer to assert busy signal
   *
   ** Input
   *  D15-0 : Latch with 32MHz clock
   *
   ** Control
   *  D15-0 : Assert one shot reset pulse in FPGA. Clear some counters
   *  D31-16: Set level.
   *
   ** Level
   *  D15-0 : Level output (1:high,0:low)
   *
   ** Pulse
   *  D15-0 : Pulse output with the width of 33 ns
   *
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
RMME::ClassName( void )
{
  static std::string g_name("RMME");
  return g_name;
}

//______________________________________________________________________________
inline GEF_UINT32
RMME::ReadRegister( GEF_UINT32 reg ) const
{
  return __bswap_32( *(m_offset+reg/GEF_VME_DWIDTH_D32) );
}

}

#endif
