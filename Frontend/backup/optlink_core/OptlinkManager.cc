// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "OptlinkManager.hh"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

#include <CAENComm.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

#include "OptCaenV1724.hh"

namespace opt
{

namespace
{
  const std::string& class_name("OptlinkManager");
}

//______________________________________________________________________________
OptlinkManager::OptlinkManager( void )
{
}

//______________________________________________________________________________
OptlinkManager::~OptlinkManager( void )
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
OptlinkManager::Close( void )
{
  const std::string& func_name(m_nick_name+" ["+class_name+"::"+__func__+"()]");
  send_normal_message(func_name);
  
  MapIterator mitr = m_module_map.begin();
  MapIterator mend = m_module_map.end();
  for( ; mitr!=mend; ++mitr ){
    ListIterator litr = mitr->second.begin();
    ListIterator lend = mitr->second.end();
    for( ; litr!=lend; ++litr ){
      Check( (*litr)->Close() , std::string((*litr)->ClassName() + "->Open() Addr::" + (*litr)->AddrStr()));
    }
  }
}

//______________________________________________________________________________
void
OptlinkManager::Open( void )
{
  const std::string& func_name(m_nick_name+" ["+class_name+"::"+__func__+"()]");
  send_normal_message(func_name);

  MapIterator mitr = m_module_map.begin();
  MapIterator mend = m_module_map.end();
  for( ; mitr!=mend; ++mitr ){
    ListIterator litr = mitr->second.begin();
    ListIterator lend = mitr->second.end();
    for( ; litr!=lend; ++litr ){
      Check( (*litr)->Open(), std::string((*litr)->ClassName() + "->Open() Addr::" + (*litr)->AddrStr()));
    }
  }

  PrintModuleList();
}

//______________________________________________________________________________
void
OptlinkManager::PrintModuleList( void ) const
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
OptlinkManager::ReadBLT( int handle, uint32_t addr, int length)
{
  int n_of_word = 0;
  CAENComm_ErrorCode status;
  for(int i = 0; i<5; ++i){
    status = CAENComm_BLTRead( handle, addr, m_blt_buf, length, &n_of_word);
    if(status == CAENComm_Success) break;
  }// for(i)

  if(status != CAENComm_Success){
    Check( status, "CAENComm_BLTRead()" );
  }
}

//______________________________________________________________________________
void
OptlinkManager::Check( CAENComm_ErrorCode status, const std::string& name )
{
  static const std::size_t n = 30;
  if( status!=CAENComm_Success ){
    const std::size_t s = name.size();
    char CommErrMess[64];
    CAENComm_DecodeError(status, CommErrMess);
    std::ostringstream oss;
    oss << m_nick_name << " " << name << " ";
    for( std::size_t i=0; i<n-s; ++i ) oss << "-";
    oss << " failed " << CommErrMess << std::endl;
    send_fatal_message( oss.str() );
    std::exit(EXIT_FAILURE);
  }

#ifdef DebugPrint
  {
    const std::size_t s = name.size();
    std::ostringstream oss;
    oss << m_nick_name << " " << name << " ";
    for( std::size_t i=0; i<n-s; ++i ) oss << "-";
    oss << " ok ";
    send_normal_message( oss.str() );
  }
#endif

}

}
