// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "CaenV792.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace vme
{

//______________________________________________________________________________
CaenV792::CaenV792( GEF_UINT32 addr )
: VmeModule(addr)
{
}

//______________________________________________________________________________
CaenV792::~CaenV792( void )
{
}

//______________________________________________________________________________
void
CaenV792::Open( void )
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
CaenV792::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  m_data_buf = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index;
  m_offset   = (GEF_UINT16*)ptr + MapSize/GEF_VME_DWIDTH_D16*index;
}

//______________________________________________________________________________
GEF_UINT32
CaenV792::DataBuf( void )
{
  return __bswap_32( *m_data_buf );
}

//______________________________________________________________________________
void
CaenV792::WriteRegister( GEF_UINT16 reg, GEF_UINT16 val )
{
  *(m_offset+reg/GEF_VME_DWIDTH_D16) = __bswap_16( val );
}

//______________________________________________________________________________
void
CaenV792::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << Addr() << std::endl
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
