// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <cstdio>
#include <cstdlib>

#include "Header.hh"

namespace vme
{

//____________________________________________________________________________
void
SetMasterHeader( GEF_UINT32  data_size,
		 GEF_UINT32  nblock,
		 GEF_UINT32* position )
{
  MasterHeader vme_master_header;
  vme_master_header.m_magic     = MasterMagic;
  vme_master_header.m_data_size = data_size;
  vme_master_header.m_nblock    = nblock;
  std::memcpy( position, &vme_master_header, MasterHeaderSize*4 );
}

//____________________________________________________________________________
void
SetModuleHeader( GEF_UINT64  vme_address,
		 GEF_UINT64  data_size,
		 GEF_UINT32* ptr )
{
  ModuleHeader vme_module_header;
  vme_module_header.m_magic               = ModuleMagic;
  vme_module_header.m_vme_address         = vme_address;
  vme_module_header.m_data_size           = data_size;
  vme_module_header.m_n_times_read_device = 0;
  vme_module_header.m_module_type[0]      = 0;
  vme_module_header.m_module_type[1]      = 0;
  vme_module_header.m_tv_sec              = 0;
  vme_module_header.m_tv_nsec             = 0;
  std::memcpy( ptr, &vme_module_header, ModuleHeaderSize*4 );
}

}