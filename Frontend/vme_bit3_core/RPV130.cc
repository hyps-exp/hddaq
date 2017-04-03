// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "RPV130.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include <bit3/vmeslib.h>

#include "MessageHelper.h"

namespace vme
{

//______________________________________________________________________________
RPV130::RPV130( uint32_t addr )
  : VmeModule(addr)
{
}

//______________________________________________________________________________
RPV130::~RPV130( void )
{
}

//______________________________________________________________________________
void
RPV130::Close( void )
{
  vme_mapclose( m_map_hdl );
}

//______________________________________________________________________________
void
RPV130::Open( void )
{
  m_map_hdl = vme_mapopen( VME_A16D16, m_addr, MapSize );
  if( !m_map_hdl ){

  }
}

//______________________________________________________________________________
void
RPV130::WriteRegister( uint16_t reg, uint16_t val )
{
  *word16( m_map_hdl, reg ) = val;
}

//______________________________________________________________________________
void
RPV130::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " Csr1 = " << ( ReadRegister( RPV130::Csr1 ) & 0xff ) << std::endl
	    << " Csr2 = " << ( ReadRegister( RPV130::Csr2 ) & 0xff ) << std::endl
	    << std::endl;
}

}
