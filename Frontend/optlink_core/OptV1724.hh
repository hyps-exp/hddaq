// -*- C++ -*-

// Author: Yoshiyuki Nakada

#ifndef OPT_CAEN_V1724_HH
#define OPT_CAEN_V1724_HH

#include <byteswap.h>
#include <stdint.h>
#include <string>
#include <vector>

#include <CAENComm.h>

#include "OptlinkModule.hh"

namespace opt
{

//______________________________________________________________________________
class OptV1724 : public OptlinkModule
{
public:
  OptV1724( int link_num, int conet_node, uint32_t base_addr );
  ~OptV1724( void );

private:
  OptV1724( const OptV1724& );
  OptV1724& operator =( const OptV1724& );

public:
  // Registers are defined in VmeV1724.hh
  
  static const std::string& ClassName( void );
  CAENComm_ErrorCode Close( void );
  CAENComm_ErrorCode Open( void );
  void               Print( void ) const;
  uint32_t           ReadRegister( uint32_t addr ) const;
  void               WriteRegister( uint32_t addr, uint32_t reg );
};

//______________________________________________________________________________
inline const std::string&
OptV1724::ClassName( void )
{
  static std::string g_name("OptV1724");
  return g_name;
}

//______________________________________________________________________________
inline uint32_t
OptV1724::ReadRegister( uint32_t addr ) const
{
  uint32_t reg;
  return CAENComm_Read32(m_handle, addr, &reg);
}

}

#endif
