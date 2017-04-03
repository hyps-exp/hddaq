// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "RM.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include <bit3/vmeslib.h>

#include "MessageHelper.h"

namespace vme
{

//______________________________________________________________________________
RM::RM( uint32_t addr )
  : VmeModule(addr)
{
}

//______________________________________________________________________________
RM::~RM( void )
{
}

//______________________________________________________________________________
void
RM::Close( void )
{
  vme_mapclose( m_map_hdl );
}

//______________________________________________________________________________
void
RM::Open( void )
{
  m_map_hdl = vme_mapopen( VME_A32D32, m_addr, MapSize );
  if( !m_map_hdl ){
  }

}

//______________________________________________________________________________
void
RM::WriteRegister( uint32_t reg, uint32_t val )
{
  *word16( m_map_hdl, reg ) = val;
}

//______________________________________________________________________________
void
RM::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " Event  = " << ReadRegister( RM::Event )  << std::endl
	    << " Spill  = " << ReadRegister( RM::Spill )  << std::endl
	    << " Serial = " << ReadRegister( RM::Serial ) << std::endl
	    << " Dummy  = " << ReadRegister( RM::Dummy )  << std::endl
	    << " Input  = " << ReadRegister( RM::Input )  << std::endl
	    // << " Time   = " << ReadRegister( RM::Time )   << std::endl
	    << std::endl;
}

}
