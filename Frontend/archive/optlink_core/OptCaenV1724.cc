// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "OptCaenV1724.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace opt
{

//______________________________________________________________________________
CaenV1724::CaenV1724( int link_num, int conet_node, uint32_t base_addr )
  : OptlinkModule(link_num, conet_node, base_addr)
{
}

//______________________________________________________________________________
CaenV1724::~CaenV1724( void )
{
}

//______________________________________________________________________________
CAENComm_ErrorCode
CaenV1724::Open( void )
{
  return CAENComm_OpenDevice(CAENComm_OpticalLink, m_link_num, m_conet_node, 0, &m_handle);
}

//______________________________________________________________________________
CAENComm_ErrorCode
CaenV1724::Close( void )
{
  return CAENComm_CloseDevice( m_handle );
}

//______________________________________________________________________________
void
CaenV1724::WriteRegister( uint32_t addr, uint32_t reg )
{
  CAENComm_Write32(m_handle, addr, reg);
}

//______________________________________________________________________________
void
CaenV1724::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " ReadoutStatus   = " << ( ReadRegister( opt::CaenV1724::ReadoutStatus ) & 0x1ff )
	    << std::endl
	    << " ChEnMask        = " << ( ReadRegister( opt::CaenV1724::ChEnMask )      & 0xff )
	    << std::endl
	    << " BufOrg          ="  << ( ReadRegister( opt::CaenV1724::BufOrg )        & 0xf )
	    << std::endl
	    << " PostTrig        = " << ( ReadRegister( opt::CaenV1724::PostTrig )      & 0xffffffff )
	    << std::endl
	    << " BoardConf       = " << ( ReadRegister( opt::CaenV1724::BoardConf )     & 0xfffff )
	    << std::endl
	    << " MemoryAfullLv   = " << ( ReadRegister( opt::CaenV1724::MemoryAfullLv ) & 0x7ff )
	    << std::endl
	    << " CustomSize      = " << ( ReadRegister( opt::CaenV1724::CustomSize )    & 0xffffffff )
	    << std::endl
	    << " IOCtrl          = " << ( ReadRegister( opt::CaenV1724::IOCtrl )        & 0xff )
	    << std::endl
	    << " GPOEnMask       = " << ( ReadRegister( opt::CaenV1724::GPOEnMask )     & 0xffffffff )
	    << std::endl
	    << " AcqCtrl         = " << ( ReadRegister( opt::CaenV1724::AcqCtrl )       & 0xf )
	    << std::endl
	    << std::endl;

  std::cout << " Channel DC offset" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( opt::CaenV1724::DcOffset ) & 0xffff ) << std::endl;
  }// for(i)

  std::cout << " Zero suppression threshold" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( opt::CaenV1724::ZeroSuppThre ) & 0xffffffff ) << std::endl;
  }// for(i)

  std::cout << " Zero suppression sample" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( opt::CaenV1724::ZeroSuppSample ) & 0xffffffff ) << std::endl;
  }// for(i)

}

}
