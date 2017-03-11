
#ifndef VME_MODULE_HH
#define VME_MODULE_HH

#include <iomanip>
#include <ostream>
#include <stdint.h>
#include <vector>
#include <gef/gefcmn_vme.h>

#include "PrintHelper.hh"

class NodeProp;

namespace vme
{

//______________________________________________________________________________
class VmeModule
{
public:
  VmeModule( GEF_UINT64 addr )
    : m_addr(addr)
  {}
  virtual ~VmeModule( void )
  {}

protected:
  GEF_UINT64   m_addr;
  GEF_VME_ADDR m_addr_param;

public:
  virtual void  Open( void ) = 0;
  static const std::string& ClassName( void );
  GEF_UINT64    Addr( void ) const { return m_addr; }
  GEF_VME_ADDR* AddrParam( void ) { return &m_addr_param; }

};

//______________________________________________________________________________
inline const std::string&
VmeModule::ClassName( void )
{
  static std::string g_name("VmeModule");
  return g_name;
}

//______________________________________________________________________________
inline std::ostream&
operator <<( std::ostream& ost, const VmeModule& module )
{
  ost << module.Addr();
  return ost;
}

}

#endif
