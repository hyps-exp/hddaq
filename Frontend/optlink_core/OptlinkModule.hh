// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef OPT_MODULE_HH
#define OPT_MODULE_HH

#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <vector>

#include "CAENComm.h"

#include "PrintHelper.hh"

class NodeProp;

namespace opt
{

//______________________________________________________________________________
class OptlinkModule
{
public:
  OptlinkModule( int link_num, int conet_node, uint32_t addr )
    : m_link_num(link_num), m_conet_node(conet_node), m_addr(addr)
  {}
  virtual ~OptlinkModule( void )
  {}

protected:
  int        m_link_num;
  int        m_conet_node;
  uint32_t   m_addr;
  int        m_handle;

public:
  virtual CAENComm_ErrorCode Close( void ) = 0;
  virtual CAENComm_ErrorCode Open( void )  = 0;
  virtual void Print( void ) const = 0;
  static const std::string& ClassName( void );


  uint32_t      Addr( void ) const { return m_addr; }
  std::string   AddrStr( void ) const;

  int           ConetNode(void) const {return m_conet_node;}
  int           Handle(void)    const {return m_handle;}
  int           LinkNum(void)   const {return m_link_num;}
};

//______________________________________________________________________________
inline const std::string&
OptlinkModule::ClassName( void )
{
  static std::string g_name("OptlinkModule");
  return g_name;
}

//______________________________________________________________________________
inline std::string
OptlinkModule::AddrStr( void ) const
{
  std::ostringstream oss;
  oss << "0x" << std::hex
      << std::setw(8) << std::setfill('0') << m_addr;
  return oss.str();
}

//______________________________________________________________________________
inline std::ostream&
operator <<( std::ostream& ost, const OptlinkModule& module )
{
  ost << module.AddrStr();
  return ost;
}

}

#endif
