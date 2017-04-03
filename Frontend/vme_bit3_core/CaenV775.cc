// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "CaenV775.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include <bit3/vmeslib.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace vme
{

//______________________________________________________________________________
CaenV775::CaenV775( uint32_t addr )
: VmeModule(addr)
{
}

//______________________________________________________________________________
CaenV775::~CaenV775( void )
{
}

//______________________________________________________________________________
void
CaenV775::Close( void )
{
  vme_mapclose( m_map_hdl );
}

//______________________________________________________________________________
void
CaenV775::Open( void )
{
  m_map_hdl = vme_mapopen( VME_A32D32, m_addr, MapSize );
  if( !m_map_hdl ){

  }
  m_data_buf = word32( m_map_hdl, 0x0 );
}

//______________________________________________________________________________
uint32_t
CaenV775::DataBuf( void )
{
  return *m_data_buf;
}

//______________________________________________________________________________
void
CaenV775::WriteRegister( uint16_t reg, uint16_t val )
{
  *word16( m_map_hdl, reg ) = val;
}

//______________________________________________________________________________
void
CaenV775::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " GeoAddr   = " << ( ReadRegister( CaenV775::GeoAddr ) & 0x1f )
	    << std::endl
	    << " ChainAddr = " << ( ReadRegister( CaenV775::ChainAddr ) & 0xff )
	    << std::endl
	    << " BitSet1   ="
	    << " BErrFlag:"  << ( (ReadRegister( CaenV775::BitSet1 )>>3) & 0x1 )
	    << " SelAddr:"   << ( (ReadRegister( CaenV775::BitSet1 )>>4) & 0x1 )
	    << " SoftReset:" << ( (ReadRegister( CaenV775::BitSet1 )>>7) & 0x1 )
	    << std::endl
	    << " Str1      = " << ( ReadRegister( CaenV775::Str1 ) & 0xff )
	    << std::endl
	    << " ChainCtrl = " << ( ReadRegister( CaenV775::ChainCtrl ) & 0x3 )
	    << std::endl
	    << " BitSet2   = " << ( ReadRegister( CaenV775::BitSet2 ) & 0x7fff )
	    << std::endl
	    << " Range     = " << ( ReadRegister( CaenV775::Range ) & 0xff )
	    << std::endl
	    << std::endl;
}

}
