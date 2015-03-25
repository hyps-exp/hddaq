#ifndef RECORD_LOGGER_H
#define RECORD_LOGGER_H

#include <fstream>
#include <string>

class Logger
{

public:
  Logger(unsigned int& event_number,
	 int run_number,
	 const std::string& file);
  ~Logger();


public:
  void close();//unsigned int event_number);
  void operator+=(unsigned long long size);

private:
  unsigned int&      m_event_number_ref;
  int                m_run_number;
  std::string        m_filename;
  std::ofstream      m_log;
  unsigned long long m_size;
//   ::timesepc         m_ts;

};

#endif
