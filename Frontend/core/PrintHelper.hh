// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef PRINT_HELPER_HH
#define PRINT_HELPER_HH

#include <iomanip>
#include <iostream>
#include <string>

//______________________________________________________________________________
class PrintHelper
{
public:
  PrintHelper( std::size_t precision,
               std::ios::fmtflags flags=std::ios::dec|std::ios::left,
               std::ostream& ost=std::cout )
    : m_ostream(ost),
      m_precision(ost.precision()),
      m_flags(ost.flags())
  {
    m_ostream.precision(precision);
    m_ostream.flags(flags);
  }
  ~PrintHelper( void )
  {
    m_ostream.precision(m_precision);
    m_ostream.flags(m_flags);
  }

private:
  std::ostream&      m_ostream;
  std::size_t        m_precision;
  std::ios::fmtflags m_flags;

};

#endif
