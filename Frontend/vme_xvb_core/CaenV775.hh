
#ifndef VME_CAEN_V775_HH
#define VME_CAEN_V775_HH

#include <stdint.h>
#include <string>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class CaenV775 : public VmeModule
{
public:
  CaenV775( GEF_UINT64 addr );
  ~CaenV775( void );

private:
  CaenV775( const CaenV775& );
  CaenV775& operator =( const CaenV775& );

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
  volatile GEF_UINT16 *m_range;

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
      Range     = 0x1060U  // R/W [7:0]
    };

  static const GEF_UINT32 GetMapSize( void ) { return MapSize; }
  static const std::string& ClassName( void );
  void       Open( void );
  GEF_UINT32 DataBuf( void );
  void       InitRegister( const GEF_MAP_PTR& ptr, int index );
  GEF_UINT16 ReadRegister( GEF_UINT16 reg );
  void       WriteRegister( GEF_UINT16 reg, GEF_UINT16 val );
  void       Print( void );
};

//______________________________________________________________________________
inline const std::string&
CaenV775::ClassName( void )
{
  static std::string g_name("CaenV775");
  return g_name;
}

}

#endif
