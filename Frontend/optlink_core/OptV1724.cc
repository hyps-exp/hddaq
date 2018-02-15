// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "OptV1724.hh"
#include "CaenV1724.hh"

#include <cstdlib>
#include <sstream>
#include <vector>

#include "MessageHelper.h"
#include "PrintHelper.hh"

namespace opt
{

//______________________________________________________________________________
OptV1724::OptV1724( int link_num, int conet_node, uint32_t base_addr )
  : OptlinkModule(link_num, conet_node, base_addr)
{
}

//______________________________________________________________________________
OptV1724::~OptV1724( void )
{
}

//______________________________________________________________________________
CAENComm_ErrorCode
OptV1724::Open( void )
{
  return CAENComm_OpenDevice(CAENComm_OpticalLink, m_link_num, m_conet_node, &m_handle);
}

//______________________________________________________________________________
CAENComm_ErrorCode
OptV1724::Close( void )
{
  return CAENComm_CloseDevice( m_handle );
}

//______________________________________________________________________________
void
OptV1724::WriteRegister( uint32_t addr, uint32_t reg )
{
  CAENComm_Write32(m_handle, addr, reg);
}

//______________________________________________________________________________
void
OptV1724::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " ReadoutStatus   = " << ( ReadRegister( vme::CaenV1724::ReadoutStatus ) & 0x1ff )
	    << std::endl
	    << " ChEnMask        = " << ( ReadRegister( vme::CaenV1724::ChEnMask )      & 0xff )
	    << std::endl
	    << " BufOrg          ="  << ( ReadRegister( vme::CaenV1724::BufOrg )        & 0xf )
	    << std::endl
	    << " PostTrig        = " << ( ReadRegister( vme::CaenV1724::PostTrig )      & 0xffffffff )
	    << std::endl
	    << " BoardConf       = " << ( ReadRegister( vme::CaenV1724::BoardConf )     & 0xfffff )
	    << std::endl
	    << " MemoryAfullLv   = " << ( ReadRegister( vme::CaenV1724::MemoryAfullLv ) & 0x7ff )
	    << std::endl
	    << " CustomSize      = " << ( ReadRegister( vme::CaenV1724::CustomSize )    & 0xffffffff )
	    << std::endl
	    << " IOCtrl          = " << ( ReadRegister( vme::CaenV1724::IOCtrl )        & 0xff )
	    << std::endl
	    << " GPOEnMask       = " << ( ReadRegister( vme::CaenV1724::GPOEnMask )     & 0xffffffff )
	    << std::endl
	    << " AcqCtrl         = " << ( ReadRegister( vme::CaenV1724::AcqCtrl )       & 0xf )
	    << std::endl
	    << std::endl;

  std::cout << " Channel DC offset" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( vme::CaenV1724::DcOffset ) & 0xffff ) << std::endl;
  }// for(i)

  std::cout << " Zero suppression threshold" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( vme::CaenV1724::ZeroSuppThre ) & 0xffffffff ) << std::endl;
  }// for(i)

  std::cout << " Zero suppression sample" << std::endl;
  for(int i = 0; i<NofCh; ++i){
    std::cout << "  Ch" << i << "    = " << ( ReadRegister( vme::CaenV1724::ZeroSuppSample ) & 0xffffffff ) << std::endl;
  }// for(i)

}

}
