// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "VmeManager.hh"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <bit3/vmeslib.h>

#include "MessageHelper.h"
#include "PrintHelper.hh"

#include "CaenV775.hh"
#include "CaenV792.hh"
#include "NoticeTDC64M.hh"
#include "RM.hh"
#include "RPV130.hh"
#include "EMC.hh"

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

  // each VmeModule
  MapIterator mitr = m_module_map.begin();
  MapIterator mend = m_module_map.end();
  for( ; mitr!=mend; ++mitr ){
    ListIterator litr = mitr->second.begin();
    ListIterator lend = mitr->second.end();
    for( ; litr!=lend; ++litr ){
      (*litr)->Close();
    }
  }
}

//______________________________________________________________________________
void
VmeManager::Open( void )
{
  const std::string& func_name(m_nick_name+" ["+class_name+"::"+__func__+"()]");

  send_normal_message(func_name);

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

}
