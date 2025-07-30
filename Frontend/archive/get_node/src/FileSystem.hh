// -*- C++ -*-

#ifndef FILE_SYSTEM_HH
#define FILE_SYSTEM_HH

#include <cstdint>
#include <filesystem>
#include <string>

//_____________________________________________________________________________
class FileSystem
{
public:
  static std::string& ClassName( void );
  static bool         Empty( const std::filesystem::path& file_path );
  static bool         Exists( const std::filesystem::path& file_path );
  static std::time_t  GetLastTime( const std::filesystem::path& file_path );
  static std::string  GetLatestRun( const std::string& dir_path );
  static std::string  GetRun( const std::string& dir_path,
			      const uint32_t run_number );
};

//_____________________________________________________________________________
inline std::string&
FileSystem::ClassName( void )
{
  static std::string s_name( "FileSystem" );
  return s_name;
}

#endif
