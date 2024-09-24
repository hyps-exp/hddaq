// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_MANAGER_HH
#define VME_MANAGER_HH

#include <map>
#include <string>
#include <vector>
#include <bit3/vmeslib.h>

#include "CaenV775.hh"
#include "CaenV792.hh"
#include "NoticeTDC64M.hh"
#include "RM.hh"
#include "RPV130.hh"
#include "EMC.hh"

class VmeModule;

//______________________________________________________________________________
namespace vme
{

/** Base address setting for multi modules
 *
 * if,
 *  base_address = 0x12340000 (first module)
 *  map_offset   = 0x00010000 (LSB of address space)
 *
 * then,
 *  module[0]:base_address = 0x12340000
 *  module[1]:base_address = 0x12350000 (=module[0]+map_offset)
 *  module[2]:base_address = 0x12360000 (=module[1]+map_offset)
 *
 * Base address should be set in serial order by map_offset increment.
 * This design is because of limitation of number of map handle.
 *
 *
 ** A new type Vme Module "NewType"
 *
 * one line is necessary as follows,
 *
 *  CreateMapWindow<NewType>();
 *
 * in VmeManager::Open() function.
 *
 */

typedef std::vector<VmeModule*>           ModuleList;
typedef ModuleList::const_iterator        ListIterator;

typedef std::map<std::string, ModuleList> ModuleMap;
typedef ModuleMap::const_iterator         MapIterator;

//______________________________________________________________________________
class VmeManager
{
public:
  static VmeManager& GetInstance( void );
  ~VmeManager( void );

private:
  VmeManager( void );
  VmeManager( const VmeManager& );
  VmeManager operator =( const VmeManager& );

private:
  static const int MaxHandleNum = 8;
  std::string                      m_nick_name;
  int                              m_hdl_num;
  ModuleMap                        m_module_map;
  std::vector<std::string>         m_module_type_list;

public:
  void               Close( void );
  const std::string& GetNickName( void ) const { return m_nick_name; }
  void               Open( void );
  void               PrintModuleList( void ) const;
  void               SetNickName( const std::string& n ) { m_nick_name = n; }

  // template for each type of VmeModule
public:
  template <typename T> void AddModule( T* module );
  template <typename T> int  GetMapSize( void ) const;
  template <typename T> T*   GetModule( int i ) const;
  template <typename T> int  GetNumOfModule( void ) const;

};

//______________________________________________________________________________
inline VmeManager&
VmeManager::GetInstance( void )
{
  static VmeManager g_instance;
  return g_instance;
}

//______________________________________________________________________________
template <typename T>
inline void
VmeManager::AddModule( T* module )
{
  m_module_map[T::ClassName()].push_back(module);
}

//______________________________________________________________________________
template <typename T>
inline int
VmeManager::GetMapSize( void ) const
{
  return T::GetMapSize();
}

//______________________________________________________________________________
template <typename T>
inline T*
VmeManager::GetModule( int i ) const
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
VmeManager::GetNumOfModule( void ) const
{
  MapIterator itr = m_module_map.find(T::ClassName());
  if( itr != m_module_map.end() )
    return (itr->second).size();
  else
    return 0;
}

}

#endif
