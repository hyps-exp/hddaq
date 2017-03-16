// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "RM.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "MessageHelper.h"

namespace vme
{

//______________________________________________________________________________
RM::RM( GEF_UINT32 addr )
  : VmeModule(addr)
{
}

//______________________________________________________________________________
RM::~RM( void )
{
}

//______________________________________________________________________________
void
RM::Open( void )
{
  GEF_VME_ADDR addr_param = {
    0x00000000,                     // upper
    VmeModule::m_addr,              // lower
    GEF_VME_ADDR_SPACE_A32,         // addr_space
    GEF_VME_2ESST_RATE_INVALID,     // vme_2esst_rate
    GEF_VME_ADDR_MODE_DEFAULT,      // addr_mode
    GEF_VME_TRANSFER_MODE_SCT,      // transfer_mode
    GEF_VME_BROADCAST_ID_DISABLE,   // broadcast_id
    GEF_VME_TRANSFER_MAX_DWIDTH_32, // transfer_max_dwidth
    GEF_VME_WND_EXCLUSIVE           // flags
  };
  VmeModule::m_addr_param = addr_param;
}


//______________________________________________________________________________
void
RM::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  GEF_UINT32 offset32 = MapSize/GEF_VME_DWIDTH_D32*index;
  m_offset = (GEF_UINT32*)ptr +offset32;
}

//______________________________________________________________________________
void
RM::WriteRegister( GEF_UINT32 reg, GEF_UINT32 val )
{
  *(m_offset+reg/GEF_VME_DWIDTH_D32) = __bswap_32( val );
}

//______________________________________________________________________________
void
RM::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << Addr() << std::endl
	    << " Event  = " << ReadRegister( RM::Event )  << std::endl
	    << " Spill  = " << ReadRegister( RM::Spill )  << std::endl
	    << " Serial = " << ReadRegister( RM::Serial ) << std::endl
	    << " Dummy  = " << ReadRegister( RM::Dummy )  << std::endl
	    << " Input  = " << ReadRegister( RM::Input )  << std::endl
	    << " Time   = " << ReadRegister( RM::Time )   << std::endl
	    << std::endl;
}

}
