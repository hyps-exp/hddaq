// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "NoticeTDC64M.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "MessageHelper.h"

namespace vme
{

//______________________________________________________________________________
NoticeTDC64M::NoticeTDC64M( GEF_UINT32 addr )
  : VmeModule(addr)
{
}

//______________________________________________________________________________
NoticeTDC64M::~NoticeTDC64M( void )
{
}

//______________________________________________________________________________
void
NoticeTDC64M::Open( void )
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
NoticeTDC64M::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  m_data_buf = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index
    + Data/GEF_VME_DWIDTH_D32;
  m_offset   = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index;
}

//______________________________________________________________________________
GEF_UINT32
NoticeTDC64M::DataBuf( GEF_UINT32 i ) const
{
  return __bswap_32( *(m_data_buf+i) );
}

//______________________________________________________________________________
void
NoticeTDC64M::WriteRegister( GEF_UINT32 reg, GEF_UINT32 val )
{
  *(m_offset+reg/GEF_VME_DWIDTH_D32) = __bswap_32( val );
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
