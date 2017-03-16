// -*- C++ -*-

#include "FPGAModule.hh"

#include <iostream>

#include "UDPRBCP.hh"

//______________________________________________________________________________
FPGAModule::FPGAModule( char*        ipAddr,
			unsigned int port,
			rbcp_header* sendHeader,
			int          disp_mode )
  : ipAddr_(ipAddr),
    port_(port),
    sendHeader_(sendHeader),
    disp_mode_(disp_mode)
{
}

//______________________________________________________________________________
FPGAModule::~FPGAModule( void )
{
}

//______________________________________________________________________________
int
FPGAModule::WriteModule( unsigned int module_id,
			 unsigned int local_address,
			 unsigned int write_data )
{
  unsigned int udp_addr
    = ((module_id & module_id_mask) << module_id_shift)
    + ((local_address & address_mask) << address_shift)
    + ((write_data & exdata_mask) >> exdata_shift);

  char udp_wd = static_cast<char>(write_data & data_mask);

  UDPRBCP udpMan(ipAddr_, port_, sendHeader_,
		 static_cast<UDPRBCP::rbcp_debug_mode>(disp_mode_));
  udpMan.SetWD(udp_addr, 1, &udp_wd);
  return udpMan.DoRBCP();
}

//______________________________________________________________________________
unsigned int
FPGAModule::ReadModule( unsigned int module_id,
			unsigned int local_address,
			int          nCycle )
{
  if(nCycle > 4){
    std::cerr << "#E :FPGAModule::ReadModule, too many cycle "
	      << nCycle << std::endl;
    return 0xeeeeeeee;
  }

  unsigned int data = 0;
  for(int i = 0; i<nCycle; ++i){
    if( this->ReadModule_nByte(module_id, local_address+i, 1) > -1){
      unsigned int tmp = (unsigned int)rd_data_[0];
      data += (tmp & 0xff) << 8*i;
    }else{
      return 0xeeeeeeee;
    }
  }

  rd_word_ = data;
  return rd_word_;
}

//______________________________________________________________________________
int FPGAModule::ReadModule_nByte( unsigned int module_id,
				  unsigned int local_address,
				  unsigned int length )
{
  rd_data_.clear();
  unsigned int udp_addr
    = ((module_id & module_id_mask) << module_id_shift)
    + ((local_address & address_mask) << address_shift);

  UDPRBCP udpMan(ipAddr_, port_, sendHeader_,
		 static_cast<UDPRBCP::rbcp_debug_mode>(disp_mode_));
  udpMan.SetRD(udp_addr, length);
  int ret;
  if((ret = udpMan.DoRBCP()) > -1){ udpMan.CopyRD(rd_data_); }

  return ret;
}