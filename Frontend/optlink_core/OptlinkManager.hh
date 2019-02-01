// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef OPT_MANAGER_HH
#define OPT_MANAGER_HH

#include <byteswap.h>
#include <map>
#include <string>
#include <vector>
#include <stdint.h>

#include "CAENComm.h"

//______________________________________________________________________________
namespace opt
{
class OptlinkModule;

typedef std::vector<OptlinkModule*>       ModuleList;
typedef ModuleList::const_iterator        ListIterator;

typedef std::map<std::string, ModuleList> ModuleMap;
typedef ModuleMap::const_iterator         MapIterator;

//______________________________________________________________________________
class OptlinkManager
{
public:
  static OptlinkManager& GetInstance( void );
  ~OptlinkManager( void );

private:
  OptlinkManager( void );
  OptlinkManager( const OptlinkManager& );
  OptlinkManager operator =( const OptlinkManager& );

private:
  static const int MaxBltBufLen = 4096;
  std::string                      m_nick_name;
  uint32_t                         m_blt_buf[MaxBltBufLen];
  ModuleMap                        m_module_map;
  std::vector<std::string>         m_module_type_list;

public:
  static const int   BltBufLen( void ) { return MaxBltBufLen; }
  void               Close( void );
  uint32_t           GetBltBuf( int i ) const { return m_blt_buf[i]; }
  const std::string& GetNickName( void ) const { return m_nick_name; }
  void               Open( void );
  void               PrintModuleList( void ) const;
  void               ReadBLT( int handle, uint32_t addr, int length);
  void               SetNickName( const std::string& n ) { m_nick_name = n; }

private:
  void               Check( CAENComm_ErrorCode status, const std::string& name );

  // template for each type of OptlinkModule
public:
  template <typename T> void AddModule( T* module );
  template <typename T> T*   GetModule( int i ) const;
  template <typename T> int  GetNumOfModule( void ) const;

};

//______________________________________________________________________________
inline OptlinkManager&
OptlinkManager::GetInstance( void )
{
  static OptlinkManager g_instance;
  return g_instance;
}

//______________________________________________________________________________
template <typename T>
inline void
OptlinkManager::AddModule( T* module )
{
  m_module_map[T::ClassName()].push_back(module);
}

//______________________________________________________________________________
template <typename T>
inline T*
OptlinkManager::GetModule( int i ) const
{
  MapIterator itr = m_module_map.find(T::ClassName());
  if( itr != m_module_map.end() )
    return dynamic_cast<T*>( (itr->second)[i] );
  else
    return 0;
}

//______________________________________________________________________________
template <typename T>
inline int
OptlinkManager::GetNumOfModule( void ) const
{
  MapIterator itr = m_module_map.find(T::ClassName());
  if( itr != m_module_map.end() )
    return (itr->second).size();
  else
    return 0;
}

}

#endif
