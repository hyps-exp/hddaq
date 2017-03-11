
#ifndef VME_CAEN_V792_HH
#define VME_CAEN_V792_HH

#include <stdint.h>
#include <string>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class CaenV792 : public VmeModule
{
public:
  CaenV792( GEF_UINT64 addr );
  ~CaenV792( void );

private:
  CaenV792( const CaenV792& );
  CaenV792& operator =( const CaenV792& );

private:
  static const GEF_UINT32 MapSize = 0x10000;
  volatile GEF_UINT32 *m_data_buf;
  volatile GEF_UINT16 *m_offset;
  volatile GEF_UINT16 *m_geo_addr;
  volatile GEF_UINT16 *m_chain_addr;
  volatile GEF_UINT16 *m_bitset1;
  volatile GEF_UINT16 *m_bitclr1;
  volatile GEF_UINT16 *m_str1;
  volatile GEF_UINT16 *m_chain_ctrl;
  volatile GEF_UINT16 *m_bitset2;
  volatile GEF_UINT16 *m_bitclr2;
  volatile GEF_UINT16 *m_iped;

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
      Iped      = 0x1060U  // R/W [7:0]
    };

  static const GEF_UINT32 GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void       Open( void );
  GEF_UINT32 DataBuf( void );
  void       InitRegister( const GEF_MAP_PTR& ptr, int index );
  GEF_UINT16 ReadRegister( GEF_UINT16 reg ) const;
  void       WriteRegister( GEF_UINT16 reg, GEF_UINT16 val );
  void       Print( void ) const;
};

//______________________________________________________________________________
inline const std::string&
CaenV792::ClassName( void )
{
  static std::string g_name("CaenV792");
  return g_name;
}

}

#endif
