#include "VmeManager.hh"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <gef/gefcmn_vme.h>
#include <gef/gefcmn_vme_defs.h>
#include <gef/gefcmn_vme_errno.h>

#include "nodeprop.h"
#include "MessageHelper.h"
#include "PrintHelper.hh"

#include "CaenV775.hh"
#include "CaenV792.hh"
#include "RM.hh"
#include "Template.hh"

namespace vme
{

namespace
{
  const std::string& class_name("VmeManager");
}

//______________________________________________________________________________
VmeManager::VmeManager( void )
{
}

//______________________________________________________________________________
VmeManager::~VmeManager( void )
{
  DeleteModule( m_caen_v775 );
  DeleteModule( m_caen_v792 );
  DeleteModule( m_vme_rm );
}

//______________________________________________________________________________
void
VmeManager::Open( const NodeProp& nodeprop )
{
  const std::string& nick_name(nodeprop.getNickName());

#ifdef DebugPrint
  const std::string& func_name(nick_name+" ["+class_name+"::"+__func__+"()]");
  std::cout << func_name << std::endl;
#endif

  GEF_STATUS status;

  status = gefVmeOpen( &m_bus_hdl );
  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << "   gefVmeOpen() failed -- " << GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  send_normal_message(" gefVmeOpen() -- ok");
#endif

  status = gefVmeAllocDmaBuf( m_bus_hdl,
			      4*MaxDmaBufLen,
			      &m_dma_hdl,
			      (GEF_MAP_PTR*)&m_dma_buf );
  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << " gefVmeAllocDmaBuf() failed -- " << GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  send_normal_message(" gefVmeAllocDmaBuf() -- ok");
#endif

  {
    GEF_VME_ADDR addr_param = {
      0x00000000,                     //upoper
      0xAA000000,                     //lower
      GEF_VME_ADDR_SPACE_A32,         //addr_space
      GEF_VME_2ESST_RATE_INVALID,     //vme_2esst_rate
      GEF_VME_ADDR_MODE_DEFAULT,      //addr_mode
      GEF_VME_TRANSFER_MODE_BLT,      //transfer_mode
      GEF_VME_BROADCAST_ID_DISABLE,   //broadcast_id
      GEF_VME_TRANSFER_MAX_DWIDTH_32, //transfer_max_dwidth
      GEF_VME_DMA_DEFAULT|GEF_VME_DMA_PFAR //flags
    };
    m_dma_addr = addr_param;
  }

  ///// each VmeModule
  OpenModule( m_caen_v775 );
  OpenModule( m_caen_v792 );
  OpenModule( m_vme_rm );
  CreateMapWindow<CaenV775>(nodeprop);
  CreateMapWindow<CaenV792>(nodeprop);
  CreateMapWindow<RM>(nodeprop);

  PrintModuleList( nick_name );

}

//______________________________________________________________________________
void
VmeManager::Close( const NodeProp& nodeprop )
{
  const std::string& nick_name(nodeprop.getNickName());
  const std::string& func_name(nick_name+" ["+class_name+"::"+__func__+"()]");

  send_normal_message(func_name);

  GEF_STATUS status;

  status = gefVmeFreeDmaBuf( m_dma_hdl );
  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << " gefVmeFreeDmaBuf() failed -- " << GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  send_normal_message(" gefVmeFreeDmaBuf() -- ok");
#endif

  for( int i=0; i<m_hdl_num; ++i ){
    status = gefVmeUnmapMasterWindow( m_map_hdl[i] );
    if( status!=GEF_STATUS_SUCCESS ){
      std::ostringstream oss;
      oss << " gefVmeUnmapMasterWindow() failed -- " << GEF_GET_ERROR(status);
      send_fatal_message( oss.str() );
      std::exit(EXIT_FAILURE);
    }

#ifdef DebugPrint
    send_normal_message(" gefVmeUnmapMasterWindow() -- ok");
#endif

    status = gefVmeReleaseMasterWindow( m_mst_hdl[i] );
    if( status!=GEF_STATUS_SUCCESS ){
      std::ostringstream oss;
      oss << " gefVmeReleaseMasterWindow() failed -- " << GEF_GET_ERROR(status);
      send_fatal_message( oss.str() );
      std::exit(EXIT_FAILURE);
    }
#ifdef DebugPrint
    send_normal_message(" gefVmeReleaseMasterWindow() -- ok");
#endif
  }
  //close device
  status = gefVmeClose( m_bus_hdl );
  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << " gefVmeClose() failed -- " << GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  send_normal_message(" gefVmeClose() -- ok");
#endif

}

//______________________________________________________________________________
template <typename T>
void
VmeManager::CreateMapWindow( const NodeProp& nodeprop )
{
  const std::string& nick_name(nodeprop.getNickName());
  const std::string& func_name(nick_name+" ["+class_name+"::"+__func__+"()]");

  GEF_STATUS  status;
  GEF_MAP_PTR ptr;
  GEF_UINT32  w_size = GetMapSize<T>() * GetNumOfModule<T>();

  if( w_size==0 )
    return;

  VmeModule* module = GetModule<T>(0);
  if( !module )
    return;

  IncrementMasterHandle();

  status = gefVmeCreateMasterWindow( m_bus_hdl,
				     module->AddrParam(),
				     w_size,
				     &m_mst_hdl[m_hdl_num-1] );

  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << func_name << " gefVmeCreateMasterWindow() failed -- "
	<< GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  send_normal_message(" gefVmeCreateMasterWindow() --- ok");
#endif

  status = gefVmeMapMasterWindow( m_mst_hdl[m_hdl_num-1],
				  0,
				  w_size,
				  &m_map_hdl[m_hdl_num-1],
				  &ptr );

  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << func_name << " gefVmeMapMasterWindow() failed -- "
	<< GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  send_normal_message(" gefVmeMapMasterWindow() --- ok");
#endif

  for( int i=0, n=GetNumOfModule<T>(); i<n; ++i ){
    GetModule<T>(i)->InitRegister( ptr, i );
  }

}

//______________________________________________________________________________
void
VmeManager::IncrementMasterHandle( void )
{
  const std::string& func_name("["+class_name+"::"+__func__+"()]");

  if( m_hdl_num>=MaxHandleNum ){
    send_error_message(func_name+" too much Master Handle");
    return;
  }
  GEF_VME_MASTER_HDL mst_hdl;
  GEF_MAP_HDL        map_hdl;
  m_mst_hdl.push_back( mst_hdl );
  m_map_hdl.push_back( map_hdl );
  m_hdl_num++;
}

//______________________________________________________________________________
void
VmeManager::PrintModuleList( const std::string& arg ) const
{
  // PrintHelper helper( 0, std::ios::hex | std::ios::showbase | std::ios::left );
  // std::ostringstream oss;
  // ost << "["+class_name+"::"+__func__+"()]" << std::endl;

  ModuleIterator itr, end=m_module_map.end();
  for( itr=m_module_map.begin(); itr!=end; ++itr ){
    for( std::size_t i=0, n=itr->second.size(); i<n; ++i ){
      std::ostringstream oss;
      oss << arg << " : " << std::setw(8) << itr->first
	  << " [" << std::hex << std::showbase
	  << itr->second[i]->Addr() << "] joined";
#ifdef DebugPrint
      std::cout << oss.str() << std::endl;
#endif
      send_normal_message(oss.str());
    }
  }

}

//______________________________________________________________________________
void
VmeManager::ReadDmaBuf( GEF_UINT32 length )
{
  GEF_STATUS status = gefVmeReadDmaBuf( m_dma_hdl, &m_dma_addr, 0, length );
  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << "vme01: gefVmeReadDmaBuf() failed -- " << GEF_GET_ERROR( status );
    send_fatal_message( oss.str() );
    std::exit( EXIT_FAILURE );
  }
}

//______________________________________________________________________________
void
VmeManager::ReadDmaBuf( GEF_VME_ADDR *addr, GEF_UINT32 length )
{
  GEF_STATUS status = gefVmeReadDmaBuf( m_dma_hdl, addr, 0, length );
  if( status!=GEF_STATUS_SUCCESS ){
    std::ostringstream oss;
    oss << "vme01: gefVmeReadDmaBuf() failed -- " << GEF_GET_ERROR( status );
    send_fatal_message( oss.str() );
    std::exit( EXIT_FAILURE );
  }
}

}
