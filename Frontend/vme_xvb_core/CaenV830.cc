// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "CaenV830.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace vme
{

//______________________________________________________________________________
CaenV830::CaenV830( GEF_UINT32 addr )
: VmeModule(addr)
{
}

//______________________________________________________________________________
CaenV830::~CaenV830( void )
{
}

//______________________________________________________________________________
void
CaenV830::Open( void )
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
CaenV830::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  m_counter  = ( (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index
		 + Counter/GEF_VME_DWIDTH_D32 );
  m_enable   = ( (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index
		 + ChEnable/GEF_VME_DWIDTH_D32 );
  m_offset16 = (GEF_UINT16*)ptr + MapSize/GEF_VME_DWIDTH_D16*index;
  m_offset32 = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index;
}

//______________________________________________________________________________
// GEF_UINT32
// CaenV830::DataBuf( void )
// {
//   return __bswap_32( *m_data_buf );
// }

//______________________________________________________________________________
GEF_UINT32
CaenV830::ReadCounter( GEF_UINT32 channel ) const
{
  return __bswap_32( *(m_counter+channel*0x4/GEF_VME_DWIDTH_D32) );
}

//______________________________________________________________________________
void
CaenV830::WriteRegister16( GEF_UINT16 reg, GEF_UINT16 val )
{
  *(m_offset16+reg/GEF_VME_DWIDTH_D16) = __bswap_16( val );
}

//______________________________________________________________________________
void
CaenV830::WriteRegister32( GEF_UINT32 reg, GEF_UINT32 val )
{
  *(m_offset32+reg/GEF_VME_DWIDTH_D32) = __bswap_32( val );
}

//______________________________________________________________________________
void
CaenV830::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " Ctrl      = " << ( ReadRegister16( CaenV830::Ctrl ) & 0xff )
	    << std::endl
	    << " GeoAddr   = " << ( ReadRegister16( CaenV830::GeoAddr ) & 0x1f )
	    << std::endl
	    << " EventNum  = " << ( ReadRegister16( CaenV830::EventNum ) & 0xff )
	    << std::endl
	    << std::endl;
}

}
