// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "EMC.hh"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

#include "MessageHelper.h"

namespace vme
{

namespace
{
  const std::string& EmcFile("/tmp/daq.txt");
  const std::string& EmcParamMagic("fin");
}

//______________________________________________________________________________
EMC::EMC( uint32_t addr )
  : VmeModule(addr),
    m_data(NDataType)
{
}

//______________________________________________________________________________
EMC::~EMC( void )
{
}

//______________________________________________________________________________
void
EMC::Close( void )
{
}

//______________________________________________________________________________
void
EMC::Open( void )
{
}

//______________________________________________________________________________
bool
EMC::ReadFile( void )
{
  bool ifs_flag = false;
  std::ifstream emc_ifs( EmcFile.c_str() );
  if( emc_ifs.fail() )
    return false;

  uint32_t serial = 0xffffffffU;
  uint32_t xpos   = 0xffffffffU;
  uint32_t ypos   = 0xffffffffU;
  uint32_t state  = 0xffffffffU;
  uint64_t time   = 0xffffffffU;
  uint32_t utime  = 0xffffffffU;
  uint32_t ltime  = 0xffffffffU;

  std::string line;
  while( emc_ifs.good() && std::getline( emc_ifs, line ) ){
    if( line.empty() )
      continue;

    std::istringstream input_line( line );
    std::istream_iterator<std::string> line_begin( input_line );
    std::istream_iterator<std::string> line_end;
    std::vector<std::string> emc_param( line_begin, line_end );

    if( emc_param.size() != EMC::kNFileParam )
      continue;
    if( emc_param[EMC::kMagic] != EmcParamMagic )
      continue;

    serial = std::strtoul( emc_param[kSerial].c_str(), NULL, 0 );
    xpos   = (uint32_t)std::strtod( emc_param[kXpos].c_str(), NULL ) + OffsetX;
    ypos   = (uint32_t)std::strtod( emc_param[kYpos].c_str(), NULL ) + OffsetY;
    state  = std::strtoul( emc_param[kState].c_str(), NULL, 0 );
    time   = std::strtoull( emc_param[kTime].c_str(), NULL, 0 );
    utime  = (time>>36) & kDataMask;
    ltime  = (time>>8) & kDataMask;
    ifs_flag = true;
  }

  if( !ifs_flag ) return false;

  m_data[Header] = ( ifs_flag & kDataMask ) | ( kHeaderMagic << kWordTypeShift );
  m_data[Serial] = ( serial & kDataMask ) | ( kSerialMagic << kWordTypeShift );
  m_data[Xpos]   = ( xpos & kDataMask ) | ( kXposMagic << kWordTypeShift );
  m_data[Ypos]   = ( ypos & kDataMask ) | ( kYposMagic << kWordTypeShift );
  m_data[State]  = ( state & kDataMask ) | ( kStateMagic << kWordTypeShift );
  m_data[Utime]  = ( utime & kDataMask ) | ( kUtimeMagic << kWordTypeShift );
  m_data[Ltime]  = ( ltime & kDataMask ) | ( kLtimeMagic << kWordTypeShift );
  m_data[Footer] = ( 0x0 & kDataMask ) | ( kFooterMagic << kWordTypeShift );

  return true;
}

//______________________________________________________________________________
void
EMC::Print( void ) const
{
  PrintHelper helper( 0, std::ios::hex | std::ios::right | std::ios::showbase );

  std::cout << "["+ClassName()+"::"+__func__+"()] " << AddrStr() << std::endl
	    << " Serial = " << ReadData( EMC::Serial ) << std::endl
	    << " Xpos   = " << ReadData( EMC::Xpos ) << std::endl
	    << " Ypos   = " << ReadData( EMC::Ypos ) << std::endl
	    << " State  = " << ReadData( EMC::State ) << std::endl
	    << " Utime  = " << ReadData( EMC::Utime ) << std::endl
	    << " Ltime  = " << ReadData( EMC::Ltime ) << std::endl
	    << std::endl;
}

}
