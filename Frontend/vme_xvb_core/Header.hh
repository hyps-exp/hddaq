// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_HEADER_HH
#define VME_HEADER_HH

#include <cstdio>
#include <cstdlib>
#include <gef/gefcmn_vme.h>
#include <cstring>
#include <byteswap.h>

namespace vme
{

//____________________________________________________________________________
/* VME */
struct MasterHeader
{
  GEF_UINT32  m_magic;
  GEF_UINT32  m_data_size;
  GEF_UINT32  m_nblock;
};
const GEF_UINT32  MasterMagic      = 0x00564d45U;
const std::size_t MasterHeaderSize = sizeof(MasterHeader)/sizeof(GEF_UINT32);

//____________________________________________________________________________
/* VMEBOARD */
struct ModuleHeader
{
  GEF_UINT64 m_magic;
  GEF_UINT64 m_vme_address;
  GEF_UINT64 m_data_size;
  GEF_UINT64 m_n_times_read_device;
  GEF_UINT64 m_module_type[2];
  GEF_UINT64 m_tv_sec;
  GEF_UINT64 m_tv_nsec;
};
const GEF_UINT64  ModuleMagic      = 0x766d65626f617264ULL;
const std::size_t ModuleHeaderSize = sizeof(ModuleHeader)/sizeof(GEF_UINT32);

//____________________________________________________________________________
void
SetMasterHeader( GEF_UINT32  data_size,
		 GEF_UINT32  nblock,
		 GEF_UINT32* position );
void
SetModuleHeader( GEF_UINT64  vme_address,
		 GEF_UINT64  data_size,
		 GEF_UINT32* position );

}

#endif
