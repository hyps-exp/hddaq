#ifndef VME_MANAGER_HH
#define VME_MANAGER_HH

#include <byteswap.h>
#include <map>
#include <string>
#include <vector>
#include <gef/gefcmn_vme_defs.h>

#include "CaenV775.hh"
#include "CaenV792.hh"
#include "RM.hh"

class NodeProp;
class VmeModule;

//______________________________________________________________________________
namespace vme
{

/*  base address setting for multi modules
if,
base_address = 0x12340000 (first module)
map_offset   = 0x00010000 (LSB of address space)

then,
module[0]:base_address = 0x12340000
module[1]:base_address = 0x12350000 (=module[0]+map_offset)
module[2]:base_address = 0x12360000 (=module[1]+map_offset)
:
(Module base address should be set in serial order by map_offset increment.)
*/

typedef std::vector<VmeModule*>           ModuleList;
typedef std::map<std::string, ModuleList> ModuleMap;
typedef ModuleMap::const_iterator         ModuleIterator;

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
  static const int MaxDmaBufLen = 1000;
  int                              m_hdl_num;
  GEF_VME_BUS_HDL                  m_bus_hdl;
  std::vector<GEF_VME_MASTER_HDL>  m_mst_hdl;
  std::vector<GEF_MAP_HDL>         m_map_hdl;
  GEF_VME_DMA_HDL                  m_dma_hdl;
  GEF_UINT32                      *m_dma_buf;
  GEF_VME_ADDR                     m_dma_addr;
  ModuleMap                        m_module_map;
  ModuleList                       m_vme_rm;
  ModuleList                       m_caen_v775;
  ModuleList                       m_caen_v792;

public:
  static const int DmaBufLen( void ) { return MaxDmaBufLen; }
  GEF_VME_BUS_HDL GetBusHandle( void ) const { return m_bus_hdl; }
  GEF_UINT32      GetDmaBuf( int i ) const { return __bswap_32( m_dma_buf[i] ); }
  void            IncrementMasterHandle( void );
  void            Open( const NodeProp& nodeprop );
  void            ReadDmaBuf( GEF_UINT32 length );
  void            ReadDmaBuf( GEF_VME_ADDR *addr, GEF_UINT32 length );
  void            Close( const NodeProp& nodeprop );

  // template for each VmeModule
  template <typename T>
  void            AddModule( T* module );
  template <typename T>
  void            CreateMapWindow( const NodeProp& nodeprop );
  template <typename T>
  int             GetMapSize( void ) const;
  template <typename T>
  T*              GetModule( int i ) const;
  template <typename T>
  int             GetNumOfModule( void ) const;

  void            PrintModuleList( const std::string& arg ) const;
};

//______________________________________________________________________________
inline VmeManager&
VmeManager::GetInstance( void )
{
  static VmeManager g_instance;
  return g_instance;
}

//______________________________________________________________________________
// template <typename T>
// inline void
// VmeManager::AddModule( T* module )
// {
//   m_module_map[T::ClassName()].push_back(module);
// }

//______________________________________________________________________________
template <>
inline void
VmeManager::AddModule<RM>( RM* module )
{
  m_module_map[RM::ClassName()].push_back(module);
  m_vme_rm.push_back( module );
}

//______________________________________________________________________________
template <>
inline void
VmeManager::AddModule<CaenV775>( CaenV775* module )
{
  m_module_map[CaenV775::ClassName()].push_back(module);
  m_caen_v775.push_back( module );
}

//______________________________________________________________________________
template <>
inline void
VmeManager::AddModule<CaenV792>( CaenV792* module )
{
  m_module_map[CaenV792::ClassName()].push_back(module);
  m_caen_v792.push_back( module );
}

//______________________________________________________________________________
template <typename T>
inline int
VmeManager::GetMapSize( void ) const
{
  return T::GetMapSize();
}

//______________________________________________________________________________
template <>
inline RM*
VmeManager::GetModule<RM>( int i ) const
{
  return dynamic_cast<RM*>(m_vme_rm[i]);
}

//______________________________________________________________________________
template <>
inline CaenV775*
VmeManager::GetModule<CaenV775>( int i ) const
{
  return dynamic_cast<CaenV775*>(m_caen_v775[i]);
}

//______________________________________________________________________________
template <>
inline CaenV792*
VmeManager::GetModule<CaenV792>( int i ) const
{
  return dynamic_cast<CaenV792*>(m_caen_v792[i]);
}

//______________________________________________________________________________
template <>
inline int
VmeManager::GetNumOfModule<RM>( void ) const
{
  return m_vme_rm.size();
}

//______________________________________________________________________________
template <>
inline int
VmeManager::GetNumOfModule<CaenV775>( void ) const
{
  return m_caen_v775.size();
}

//______________________________________________________________________________
template <>
inline int
VmeManager::GetNumOfModule<CaenV792>( void ) const
{
  return m_caen_v792.size();
}

}

#endif
