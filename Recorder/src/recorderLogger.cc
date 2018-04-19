// -*- C++ -*-

#include <iostream>
#include <iomanip>
#include <vector>
#include <sys/stat.h>
#include <ctime>

#include "Recorder/recorderLogger.hh"

//______________________________________________________________________________
Logger::Logger(unsigned int& event_number,
	       int run_number,
	       const std::string& filename)
  : m_event_number_ref(event_number),
    m_run_number(run_number),
    m_filename(filename),
    m_log(),
    m_size(0)
{
  std::ifstream log_exists(m_filename.c_str());
  if (log_exists.good())
    {
      ::chmod(m_filename.c_str(),
	      S_IRUSR | S_IWUSR |
	      S_IRGRP | S_IWGRP |
	      S_IROTH);
      log_exists.close();
    }
  m_log.open(m_filename.c_str(), std::ios::out | std::ios::app);
  if (m_log.fail())
    std::cerr << "#E failed to open file: " << m_filename << std::endl;
  m_log << "\nRUN ";
  //   m_log.fill('0');
  m_log << std::setw(5) << m_run_number;
  //   m_log.fill(' ');
  m_log << " : ";
  std::time_t t;
  std::tm     timer;
  std::vector<char> v(0x100);
  std::time(&t);
  //   ::clock_gettime(CLOCK_REALTIME, &m_ts);
  ::tzset();
  ::localtime_r(&t, &timer);
  //   ::localtime_r(&m_ts.tv_sec, &timer);
  std::strftime(&(v[0]), 0xff, "%c", &timer);
  std::string str = &v[0];
  m_log << std::setw(26) << str << "  -";
  m_log.flush();
}

//______________________________________________________________________________
Logger::~Logger()
{
  if (m_log.is_open())
    {
      close();
    }
}

//______________________________________________________________________________
void
Logger::operator+=(unsigned long long size)
{
  m_size += size;
  return;
}

//______________________________________________________________________________
void
Logger::close()
{
  std::time_t t;
  std::tm     timer;
  std::vector<char> v(0x100);
  std::time(&t);
  //   unsigned long long sec  = m_ts.tv_sec;
  //   unsigned long long nsec = m_ts.tv_nsec;
  //   ::clock_gettime(CLOCK_REALTIME, &m_ts);
  ::tzset();
  ::localtime_r(&t, &timer);
  //   ::localtime_r(&m_ts.tv_sec, &timer);
  std::strftime(&(v[0]), 0xff, "%c", &timer);
  std::string str = &v[0];
  m_log << std::setw(26) << str;

  m_log << " : " << std::setw(10) << m_event_number_ref;
  m_log << " events : " << std::setw(14) << m_size << " bytes";
  m_log.flush();
  m_log.close();
  ::chmod(m_filename.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  return;
}
