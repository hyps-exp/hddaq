// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "RPV130.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "MessageHelper.h"

namespace vme
{

//______________________________________________________________________________
RPV130::RPV130( GEF_UINT32 addr )
  : VmeModule(addr)
{
}

//______________________________________________________________________________
RPV130::~RPV130( void )
{
}

//______________________________________________________________________________
void
RPV130::Open( void )
{
  VmeModule::m_addr_param.upper               = 0x00000000;
  VmeModule::m_addr_param.lower               = VmeModule::m_addr;
  VmeModule::m_addr_param.addr_space          = GEF_VME_ADDR_SPACE_A16;
  VmeModule::m_addr_param.vme_2esst_rate      = GEF_VME_2ESST_RATE_INVALID;
  VmeModule::m_addr_param.addr_mode           = GEF_VME_ADDR_MODE_DEFAULT;
  VmeModule::m_addr_param.transfer_mode       = GEF_VME_TRANSFER_MODE_SCT;
  VmeModule::m_addr_param.broadcast_id        = GEF_VME_BROADCAST_ID_DISABLE;
  VmeModule::m_addr_param.transfer_max_dwidth = GEF_VME_TRANSFER_MAX_DWIDTH_32;
  VmeModule::m_addr_param.flags               = GEF_VME_WND_EXCLUSIVE;
}

//______________________________________________________________________________
void
RPV130::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  m_offset = (GEF_UINT16*)ptr + MapSize/GEF_VME_DWIDTH_D16*index;
}

//______________________________________________________________________________
void
RPV130::WriteRegister( GEF_UINT16 reg, GEF_UINT16 val )
{
  *(m_offset+reg/GEF_VME_DWIDTH_D16) = __bswap_16( val );
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
