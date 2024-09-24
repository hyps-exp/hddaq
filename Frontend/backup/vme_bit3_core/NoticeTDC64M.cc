// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "NoticeTDC64M.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include <bit3/vmeslib.h>

#include "MessageHelper.h"

namespace vme
{

//______________________________________________________________________________
NoticeTDC64M::NoticeTDC64M( uint32_t addr )
  : VmeModule(addr)
{
}

//______________________________________________________________________________
NoticeTDC64M::~NoticeTDC64M( void )
{
}

//______________________________________________________________________________
void
NoticeTDC64M::Close( void )
{
  vme_mapclose( m_map_hdl );
}

//______________________________________________________________________________
void
NoticeTDC64M::Open( void )
{
  m_map_hdl = vme_mapopen( VME_A32D32, m_addr, MapSize );
  if( !m_map_hdl ){

  }
  m_data_buf = word32( m_map_hdl, 0x4000 );
}

//______________________________________________________________________________
uint32_t
NoticeTDC64M::DataBuf( uint32_t i ) const
{
  return *(m_data_buf+i);
}

//______________________________________________________________________________
void
NoticeTDC64M::WriteRegister( uint32_t reg, uint32_t val )
{
  *word32( m_map_hdl, reg ) = val;
}

//______________________________________________________________________________
void
NoticeTDC64M::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " Ctrl    ="
	    << " Reset:" << ( ( ReadRegister( NoticeTDC64M::Ctrl ) >> 0 ) & 0x1 )
	    << " Range:" << ( ( ReadRegister( NoticeTDC64M::Ctrl ) >> 1 ) & 0x7 )
	    << " Edge:"  << ( ( ReadRegister( NoticeTDC64M::Ctrl ) >> 4 ) & 0x1 )
	    << " MID:"   << ( ( ReadRegister( NoticeTDC64M::Ctrl ) >> 5 ) & 0x1f )
	    << std::endl
	    << " ClStat  ="
	    << " NWord:"  << ( ( ReadRegister( NoticeTDC64M::ClStat ) >> 0 ) & 0xfff )
	    << " DReady:" << ( ( ReadRegister( NoticeTDC64M::ClStat ) >> 12 ) & 0x1 )
	    << std::endl
	    << " Enable1 = " << ReadRegister( NoticeTDC64M::Enable1 ) << std::endl
	    << " Enable2 = " << ReadRegister( NoticeTDC64M::Enable2 ) << std::endl
	    << " Window  ="
	    << " Search:" << ( ( ReadRegister( NoticeTDC64M::Window ) >> 0 ) & 0xffff )
	    << " Mask:" << ( ( ReadRegister( NoticeTDC64M::Window ) >> 16 ) & 0xffff )
	    << std::endl
	    << " Event   = " << ( ReadRegister( NoticeTDC64M::Event ) & 0xff ) << std::endl
	    << std::endl;
}

}
