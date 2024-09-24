// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "userdevice.h"

#include <sstream>
#include <sys/time.h>
#include <iomanip>
#include <iostream>

#include "EventReader.hh"
#include "FileSystem.hh"

namespace
{
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100;         //maximum count to check data ready
  const int max_data_size = 1000000; //maximum datasize by byte unit
  DaqMode     g_daq_mode = DM_NORMAL;
  int         g_cobo_id = -1;
  EventReader g_event_reader;
  std::string g_data_path;
  std::char_traits<char>::pos_type g_seek_pos = 0;
}

//____________________________________________________________________________
int
get_maxdatasize( void )
{
  return max_data_size;
}

//____________________________________________________________________________
void
open_device( NodeProp& nodeprop )
{
  g_daq_mode = nodeprop.getDaqMode();
  const int argc = nodeprop.getArgc();
  for( int i=0; i<argc; ++i ){
    const std::string arg = nodeprop.getArgv(i);
    std::istringstream iss;
    if( arg.substr(0,9) == "--coboid=" ){
      iss.str( arg.substr(9) );
      iss >> g_cobo_id;
    }
  }
  std::ostringstream oss;
  oss.flags( std::ios::left );
  if( g_cobo_id >= 0 ){
    oss << nodeprop.getNickName() << " set cobo id " << g_cobo_id;
    send_normal_message( oss.str() );
  } else {
    oss << nodeprop.getNickName() << " found invalid cobo id ("
	<< g_cobo_id << ")";
    send_fatal_message( oss.str() );
    std::exit( EXIT_FAILURE );
  }
  return;
}

//____________________________________________________________________________
void
init_device( NodeProp& nodeprop )
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int run_number = nodeprop.getRunNumber();
      std::ostringstream oss;
      // oss << "/raid/getdata/bench_e42_2021may/cobo" << g_cobo_id
      oss << "/raid/getdata/e42_2021may/cobo" << g_cobo_id
      // oss << "/raid/getdata/e03_2021feb/cobo" << g_cobo_id
      //oss << "/raid/getdata/bench_e03_2021feb/cobo" << g_cobo_id
	 << "/run_" << std::setfill('0') << std::setw(4) << run_number
	 << ".dat";
      // g_data_path = FileSystem::GetRun( ss.str(), run_number );
      g_data_path = oss.str();
      g_event_reader.Clear();
      // g_event_reader.SetPrintCycle( 1 );
      g_seek_pos = 0;
      oss.str("");
      oss << nodeprop.getNickName() << " set data path : " << g_data_path;
      send_normal_message( oss.str() );
      return;
    }
  case DM_DUMMY:
    {
      return;
    }
  default:
    return;
  }
}

//____________________________________________________________________________
void
finalize_device( NodeProp& nodeprop )
{
  // erase raw data
  return;
}

//____________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{
  return;
}

//____________________________________________________________________________
int
wait_device( NodeProp& nodeprop )
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      if( !FileSystem::Exists( g_data_path ) ||
	  FileSystem::Empty( g_data_path ) ){
	::usleep(100000);
	return -1;
      }
      // static std::time_t last_time = 0;
      // std::time_t curr_time = FileSystem::GetLastTime( g_data_path );
      // if( last_time < curr_time ){
	// last_time = curr_time;
      // ::sleep(1);
      return 0;
      // } else {
      // 	last_time = curr_time;
      // 	::usleep(100000);
      // 	return -1;
      // }
    }
  case DM_DUMMY:
    {
      ::usleep(200000);
      return 0;
    }
  default:
    return 0;
  }
}

//____________________________________________________________________________
int
read_device( NodeProp& nodeprop, unsigned int* data, int& len )
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      uint32_t ndata = 0;

      const uint32_t event_number = nodeprop.getEventNumber();

      g_event_reader.SetStream( g_data_path );
      g_event_reader.seekg( g_seek_pos );

      bool next = ( g_event_reader.Next() &&
		    g_event_reader.HasEventBuf( event_number ) );
      if( next ){
	const auto& head = g_event_reader.CoBoHeader();
	data[ndata++] = head->m_magic;
	data[ndata++] = head->m_cobo_id;
	data[ndata++] = head->m_data_size;
	data[ndata++] = head->m_nblock;
	const auto& buf = g_event_reader.EventBuf( event_number );
	for( const auto& asad : buf ){
	  for( const auto& b : asad ){
	    if( ndata == max_data_size/4 ){
	      std::cerr << "too much data!!!" << std::endl;
	      std::exit( EXIT_FAILURE );
	    }
	    data[ndata++] = b;
	  }
	}
	// if( ndata == 0 ){
	//   std::cerr << "wrong event number? ndata=0" << std::endl;
	//   std::exit( EXIT_FAILURE );
	// }
	g_seek_pos = g_event_reader.tellg();
      } else {
	::usleep(10);
      }
      len = ndata;
      return next ? 0 : -1;
    }
  case DM_DUMMY:
    {
      len = 0;
      return 0;
    }
  default:
    len = 0;
    return 0;
  }
}
