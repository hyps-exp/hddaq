// -*- C++ -*-

#ifndef RECORD_BOOKMARKER_H
#define RECORD_BOOKMARKER_H

#include <fstream>
#include <string>

class Bookmarker
{

public:
  Bookmarker(int run_number,
	     std::string dir_name);
  ~Bookmarker();


public:
  void close();
  void operator +=(unsigned long long size);

private:
  int                m_run_number;
  std::string        m_filename;
  std::ofstream      m_bookmark;
  unsigned long long m_index;
};

#endif
