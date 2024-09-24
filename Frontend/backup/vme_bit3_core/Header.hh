// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_HEADER_HH
#define VME_HEADER_HH

#include <stdint.h>
#include <cstring>

namespace vme
{

//____________________________________________________________________________
/* VME */
struct MasterHeader
{
  uint32_t  m_magic;
  uint32_t  m_data_size;
  uint32_t  m_nblock;
};
const uint32_t  MasterMagic      = 0x00564d45U;
const std::size_t MasterHeaderSize = sizeof(MasterHeader)/sizeof(uint32_t);

//____________________________________________________________________________
/* VMEBOARD */
struct ModuleHeader
{
  uint64_t m_magic;
  uint64_t m_vme_address;
  uint64_t m_data_size;
  uint64_t m_n_times_read_device;
  uint64_t m_module_type[2];
  uint64_t m_tv_sec;
  uint64_t m_tv_nsec;
};
const uint64_t  ModuleMagic      = 0x766d65626f617264ULL;
const std::size_t ModuleHeaderSize = sizeof(ModuleHeader)/sizeof(uint32_t);

//____________________________________________________________________________
void
SetMasterHeader( uint32_t  data_size,
		 uint32_t  nblock,
		 uint32_t* position );
void
SetModuleHeader( uint64_t  vme_address,
		 uint64_t  data_size,
		 uint32_t* position );

}

#endif
