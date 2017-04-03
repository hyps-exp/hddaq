// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_MODULE_HH
#define VME_MODULE_HH

#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <vector>

#include <bit3/vmeslib.h>

#include "PrintHelper.hh"

class NodeProp;

namespace vme
{

//______________________________________________________________________________
inline volatile uint16_t*
word16( VMEMAP *map_handle, uint16_t offset )
{
  return (volatile uint16_t*)((char*)(map_handle->base)+offset);
}

//______________________________________________________________________________
inline volatile uint32_t*
word32( VMEMAP *map_handle, uint32_t offset )
{
  return (volatile uint32_t*)((char*)(map_handle->base)+offset);
}

//______________________________________________________________________________
class VmeModule
{
public:
  VmeModule( uint32_t addr )
    : m_addr(addr)
  {}
  virtual ~VmeModule( void )
  {}

protected:
  uint32_t  m_addr;
  VMEMAP   *m_map_hdl;

public:
  virtual void Close( void ) = 0;
  virtual void Open( void ) = 0;
  virtual void Print( void ) const = 0;
  static const std::string& ClassName( void );

  uint32_t      Addr( void ) const { return m_addr; }
  std::string   AddrStr( void ) const;

};

//______________________________________________________________________________
inline const std::string&
VmeModule::ClassName( void )
{
  static std::string g_name("VmeModule");
  return g_name;
}

//______________________________________________________________________________
inline std::string
VmeModule::AddrStr( void ) const
{
  std::ostringstream oss;
  oss << "0x" << std::hex
      << std::setw(8) << std::setfill('0') << m_addr;
  return oss.str();
}

//______________________________________________________________________________
inline std::ostream&
operator <<( std::ostream& ost, const VmeModule& module )
{
  ost << module.AddrStr();
  return ost;
}

}

#endif
