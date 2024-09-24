// -*- C++ -*-

#include "FileSystem.hh"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "FuncName.hh"

namespace
{
  using std::chrono::duration_cast;
  using std::chrono::seconds;
}

//_____________________________________________________________________________
bool
FileSystem::Empty( const std::filesystem::path& file_path )
{
  return std::filesystem::file_size( file_path ) == 0;
}

//_____________________________________________________________________________
bool
FileSystem::Exists( const std::filesystem::path& file_path )
{
  return std::filesystem::exists( file_path );
}

//_____________________________________________________________________________
std::time_t
FileSystem::GetLastTime( const std::filesystem::path& file_path )
{
  return duration_cast<seconds>
    ( std::filesystem::last_write_time( file_path ).time_since_epoch() )
    .count();
}

//_____________________________________________________________________________
std::string
FileSystem::GetLatestRun( const std::string& dir_path )
{
  std::string latest_file;
  std::time_t latest_time = 0;
  for( const auto& d : std::filesystem::directory_iterator( dir_path ) ){
    auto s = duration_cast<seconds>( d.last_write_time().time_since_epoch() );
    std::time_t t = s.count();
    if( std::string( d.path() ).find( "/run_" ) == std::string::npos )
      continue;
    if( t > latest_time ){
      latest_time = t;
      latest_file = d.path();
#if 0
      const tm* lt = std::localtime( &t );
      std::cout << d.path() << " : " << t << " "
      		<< std::put_time( lt, "%c" ) << std::endl;
#endif
    }
  }
  return latest_file;
}

//_____________________________________________________________________________
std::string
FileSystem::GetRun( const std::string& dir_path,
		    const uint32_t run_number )
{
  std::stringstream ss;
  ss << "/run_" << std::setfill('0') << std::setw(4) << run_number
     << ".dat";
  for( const auto& d : std::filesystem::directory_iterator( dir_path ) ){
    if( std::string( d.path() ).find( ss.str() )
	!= std::string::npos ){
      return d.path();
    }
  }
  return "/dev/null";
}
