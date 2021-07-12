// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "VmeManager.hh"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <gef/gefcmn_vme.h>
#include <gef/gefcmn_vme_defs.h>
#include <gef/gefcmn_vme_errno.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

#include "CaenV775.hh"
#include "CaenV792.hh"
#include "CaenV830.hh"
#include "NoticeTDC64M.hh"
#include "RM.hh"
#include "RMME.hh"
#include "RPV130.hh"

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
  MapIterator mitr = m_module_map.begin();
  MapIterator mend = m_module_map.end();
  for( ; mitr!=mend; ++mitr ){
    ListIterator litr = mitr->second.begin();
    ListIterator lend = mitr->second.end();
    for( ; litr!=lend; ++litr ){
      delete *litr;
    }
  }
}

//______________________________________________________________________________
void
VmeManager::Close( void )
{
  const std::string& func_name(m_nick_name+" ["+class_name+"::"+__func__+"()]");

  send_normal_message(func_name);

  Check( gefVmeFreeDmaBuf( m_dma_hdl ), "gefVmeFreeDmaBuf()" );

  for( int i=0; i<m_hdl_num; ++i ){
    Check( gefVmeUnmapMasterWindow( m_map_hdl[i] ),
	   "gefVmeUnmapMasterWindow()" );
    Check( gefVmeReleaseMasterWindow( m_mst_hdl[i] ),
	   "gefVmeReleaseMasterWindow()" );
  }

  Check( gefVmeClose( m_bus_hdl ), "gefVmeClose()" );
}

//______________________________________________________________________________
void
VmeManager::Open( void )
{
  const std::string& func_name(m_nick_name+" ["+class_name+"::"+__func__+"()]");

  send_normal_message(func_name);

  Check( gefVmeOpen( &m_bus_hdl ), "gefVmeOpen()" );

  {
    char *tmp = (char*)&m_dma_buf; // type-punned pointer
    Check( gefVmeAllocDmaBuf( m_bus_hdl,
			      4*MaxDmaBufLen,
			      &m_dma_hdl,
			      (GEF_MAP_PTR*)tmp ),
	   "gefVmeAllocDmaBuf()" );
  }

  // each VmeModule
  MapIterator mitr = m_module_map.begin();
  MapIterator mend = m_module_map.end();
  for( ; mitr!=mend; ++mitr ){
    ListIterator litr = mitr->second.begin();
    ListIterator lend = mitr->second.end();
    for( ; litr!=lend; ++litr ){
      (*litr)->Open();
    }
  }

  // each type of VmeModule
  CreateMapWindow<CaenV775>();
  CreateMapWindow<CaenV792>();
  CreateMapWindow<CaenV830>();
  CreateMapWindow<CaenV1724>();
  CreateMapWindow<NoticeTDC64M>();
  CreateMapWindow<RM>();
  CreateMapWindow<RMME>();
  CreateMapWindow<RPV130>();

  PrintModuleList();
}

//______________________________________________________________________________
void
VmeManager::PrintModuleList( void ) const
{
  MapIterator mitr, mend=m_module_map.end();
  for( mitr=m_module_map.begin(); mitr!=mend; ++mitr ){
    ListIterator litr, lend=mitr->second.end();
    for( litr=mitr->second.begin(); litr!=lend; ++litr ){
      std::ostringstream oss;
      oss.flags( std::ios::left );
      oss << m_nick_name << " : " << std::setw(14) << mitr->first
	  << " [" << (*litr)->AddrStr() << "] joined";
      send_normal_message(oss.str());
    }
  }

}

//______________________________________________________________________________
void
VmeManager::ReadDmaBuf( GEF_UINT32 length, GEF_UINT32 offset )
{
  Check( gefVmeReadDmaBuf( m_dma_hdl, &m_dma_addr, offset, length ),
	 "gefVmeReadDmaBuf()" );
}

//______________________________________________________________________________
void
VmeManager::ReadDmaBuf( GEF_VME_ADDR *addr, GEF_UINT32 length, GEF_UINT32 offset )
{
  Check( gefVmeReadDmaBuf( m_dma_hdl, addr, offset, length ),
	 "gefVmeReadDmaBuf()" );
}

//______________________________________________________________________________
void
VmeManager::SetDmaAddress( GEF_UINT32 addr )
{
  GEF_VME_ADDR addr_param = {
    0x00000000,                          // upper
    addr,                                // lower
    GEF_VME_ADDR_SPACE_A32,              // addr_space
    GEF_VME_2ESST_RATE_INVALID,          // vme_2esst_rate
    GEF_VME_ADDR_MODE_DEFAULT,           // addr_mode
    GEF_VME_TRANSFER_MODE_BLT,           // transfer_mode
    GEF_VME_BROADCAST_ID_DISABLE,        // broadcast_id
    GEF_VME_TRANSFER_MAX_DWIDTH_32,      // transfer_max_dwidth
    GEF_VME_DMA_DEFAULT|GEF_VME_DMA_PFAR // flags
  };
  m_dma_addr = addr_param;
}

//______________________________________________________________________________
void
VmeManager::Check( GEF_STATUS status, const std::string& name )
{
  static const std::size_t n = 30;
  if( status!=GEF_STATUS_SUCCESS ){
    const std::size_t s = name.size();
    std::ostringstream oss;
    oss << m_nick_name << " " << name << " ";
    for( std::size_t i=0; i<n-s; ++i )
      oss << "-";
    oss << " failed " << GEF_GET_ERROR(status);
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  {
    const std::size_t s = name.size();
    std::ostringstream oss;
    oss << m_nick_name << " " << name << " ";
    for( std::size_t i=0; i<n-s; ++i )
      oss << "-";
    oss << " ok ";
    send_normal_message( oss.str() );
  }
#endif

}

//______________________________________________________________________________
void
VmeManager::IncrementMasterHandle( void )
{
  const std::string& func_name("["+class_name+"::"+__func__+"()]");

  if( m_hdl_num>=MaxHandleNum ){
    send_fatal_message(func_name+" too much Master Handle");
    std::exit(EXIT_FAILURE);
  }
  GEF_VME_MASTER_HDL mst_hdl;
  GEF_MAP_HDL        map_hdl;
  m_mst_hdl.push_back( mst_hdl );
  m_map_hdl.push_back( map_hdl );
  m_hdl_num++;
}

//______________________________________________________________________________
template <typename T>
void
VmeManager::CreateMapWindow( void )
{
  GEF_MAP_PTR ptr;
  GEF_UINT32  window_size = GetMapSize<T>() * GetNumOfModule<T>();

  if( window_size==0 )
    return;

  VmeModule* first_module = GetModule<T>(0);

  if( !first_module )
    return;

  IncrementMasterHandle();

  Check( gefVmeCreateMasterWindow( m_bus_hdl,
				   first_module->AddrParam(),
				   window_size,
				   &m_mst_hdl[m_hdl_num-1] ),
	 "gefVmeCreateMasterWindow()" );

  Check( gefVmeMapMasterWindow( m_mst_hdl[m_hdl_num-1],
				0,
				window_size,
				&m_map_hdl[m_hdl_num-1],
				&ptr ),
	 "gefVmeMapMasterWindow()" );

  for( int i=0, n=GetNumOfModule<T>(); i<n; ++i ){
    GetModule<T>(i)->InitRegister( ptr, i );
  }

}

}
