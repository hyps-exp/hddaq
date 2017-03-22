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
  VmeModule::m_addr_param.upper               = 0x00000000;
  VmeModule::m_addr_param.lower               = VmeModule::m_addr;
  VmeModule::m_addr_param.addr_space          = GEF_VME_ADDR_SPACE_A32;
  VmeModule::m_addr_param.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  VmeModule::m_addr_param.addr_mode           = GEF_VME_ADDR_MODE_DEFAULT;
  VmeModule::m_addr_param.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  VmeModule::m_addr_param.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  VmeModule::m_addr_param.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  VmeModule::m_addr_param.flags               = GEF_VME_WND_EXCLUSIVE;
}

//______________________________________________________________________________
void
RM::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  m_offset = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index;
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
