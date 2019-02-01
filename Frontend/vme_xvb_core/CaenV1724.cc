// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "CaenV1724.hh"

#include <cstdlib>
#include <sstream>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace vme
{

//______________________________________________________________________________
CaenV1724::CaenV1724( GEF_UINT32 addr )
: VmeModule(addr)
{
}

//______________________________________________________________________________
CaenV1724::~CaenV1724( void )
{
}

//______________________________________________________________________________
void
CaenV1724::Open( void )
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
CaenV1724::InitRegister( const GEF_MAP_PTR& ptr, int index )
{
  m_data_buf = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index;
  m_offset   = (GEF_UINT32*)ptr + MapSize/GEF_VME_DWIDTH_D32*index;
}

//______________________________________________________________________________
GEF_UINT32
CaenV1724::DataBuf( void )
{
  return __bswap_32( *m_data_buf );
}

//______________________________________________________________________________
void
CaenV1724::WriteRegister( GEF_UINT32 reg, GEF_UINT32 val )
{
  *(m_offset+reg/GEF_VME_DWIDTH_D32) = __bswap_32( val );
}

//______________________________________________________________________________
void
CaenV1724::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " ReadoutStatus   = " << ( ReadRegister( CaenV1724::ReadoutStatus ) & 0x1ff )
	    << std::endl
	    << " ChEnMask        = " << ( ReadRegister( CaenV1724::ChEnMask )      & 0xff )
	    << std::endl
	    << " BufOrg          ="  << ( ReadRegister( CaenV1724::BufOrg )        & 0xf )
	    << std::endl
	    << " PostTrig        = " << ( ReadRegister( CaenV1724::PostTrig )      & 0xffffffff )
	    << std::endl
	    << " BoardConf       = " << ( ReadRegister( CaenV1724::BoardConf )     & 0xfffff )
	    << std::endl
	    << " MemoryAfullLv   = " << ( ReadRegister( CaenV1724::MemoryAfullLv ) & 0x7ff )
	    << std::endl
	    << " CustomSize      = " << ( ReadRegister( CaenV1724::CustomSize )    & 0xffffffff )
	    << std::endl
	    << " IOCtrl          = " << ( ReadRegister( CaenV1724::IOCtrl )        & 0xff )
	    << std::endl
	    << " GPOEnMask       = " << ( ReadRegister( CaenV1724::GPOEnMask )     & 0xffffffff )
	    << std::endl
	    << " AcqCtrl         = " << ( ReadRegister( CaenV1724::AcqCtrl )       & 0xf )
	    << std::endl
	    << std::endl;

  std::cout << " Channel DC offset" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( CaenV1724::DcOffset ) & 0xffff ) << std::endl;
  }// for(i)

  std::cout << " Zero suppression threshold" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( CaenV1724::ZeroSuppThre ) & 0xffffffff ) << std::endl;
  }// for(i)

  std::cout << " Zero suppression sample" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( CaenV1724::ZeroSuppSample ) & 0xffffffff ) << std::endl;
  }// for(i)

}

}
