// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "CaenV792.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include <bit3/vmeslib.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace vme
{

//______________________________________________________________________________
CaenV792::CaenV792( uint32_t addr )
: VmeModule(addr)
{
}

//______________________________________________________________________________
CaenV792::~CaenV792( void )
{
}

//______________________________________________________________________________
void
CaenV792::Close( void )
{
  vme_mapclose( m_map_hdl );
}

//______________________________________________________________________________
void
CaenV792::Open( void )
{
  m_map_hdl = vme_mapopen( VME_A32D32, m_addr, MapSize );
  if( !m_map_hdl ){

  }
  m_data_buf = word32( m_map_hdl, 0x0 );
}

//______________________________________________________________________________
uint32_t
CaenV792::DataBuf( void )
{
  return *m_data_buf;
}

//______________________________________________________________________________
void
CaenV792::WriteRegister( uint16_t reg, uint16_t val )
{
  *word16( m_map_hdl, reg ) = val;
}

//______________________________________________________________________________
void
CaenV792::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " GeoAddr   = " << ( ReadRegister( CaenV792::GeoAddr ) & 0x1f )
	    << std::endl
	    << " ChainAddr = " << ( ReadRegister( CaenV792::ChainAddr ) & 0xff )
	    << std::endl
	    << " BitSet1   ="
	    << " BErrFlag:"  << ( (ReadRegister( CaenV792::BitSet1 )>>3) & 0x1 )
	    << " SelAddr:"   << ( (ReadRegister( CaenV792::BitSet1 )>>4) & 0x1 )
	    << " SoftReset:" << ( (ReadRegister( CaenV792::BitSet1 )>>7) & 0x1 )
	    << std::endl
	    << " Str1      = " << ( ReadRegister( CaenV792::Str1 ) & 0xff )
	    << std::endl
	    << " ChainCtrl = " << ( ReadRegister( CaenV792::ChainCtrl ) & 0x3 )
	    << std::endl
	    << " BitSet2   = " << ( ReadRegister( CaenV792::BitSet2 ) & 0x7fff )
	    << std::endl
	    << " Iped      = " << ( ReadRegister( CaenV792::Iped ) & 0xff )
	    << std::endl
	    << std::endl;
}

}
