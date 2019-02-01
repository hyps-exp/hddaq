// -*- C++ -*-
/**
 *  @file   EventBuffer.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: EventBuffer.cc,v 1.3 2010/06/28 08:31:50 igarashi Exp $
 *  $Log: EventBuffer.cc,v $
 *  Revision 1.3  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
 *
 *  Revision 1.2  2009/06/29 02:32:12  igarashi
 *  add clear method in EventBuffer
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.1.1.1  2008/01/30 12:33:33  igarashi
 *  Network DAQ Software Prototype 1.4
 *
 *  Revision 1.1.1.1  2007/09/21 08:50:48  igarashi
 *  prototype-1.3
 *
 *  Revision 1.1.1.1  2007/03/28 07:50:17  cvs
 *  prototype-1.2
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */

#include <string.h>

#include <iostream>

#include "EventData/EventBuffer.h"

EventBuffer::EventBuffer(int len)
{
  assert(len > 0);
  if(len > 0) {
    m_buf = new char[len];
    m_len = len;
    memset(m_buf, 0, len);
  }
  else {
    m_buf = 0;
    m_len = 0;
  }
}

EventBuffer::~EventBuffer()
{
  assert(m_len > 0);
  if(m_buf)
    delete[] m_buf;
  m_buf = 0;
}

EventBuffer::EventBuffer(const EventBuffer &from)
{
  if(from.m_len > 0) {
    m_buf = new char[from.m_len];
    memcpy(m_buf, from.m_buf, from.m_len);
    m_len = from.m_len;
  }
  else {
    m_buf = 0;
    m_len = 0;
  }
}

////
void
EventBuffer::clear()
{
  memset(m_buf, 0, m_len);
}

////
EventBuffer&
EventBuffer::operator=(const EventBuffer &from)
{
  if (&from == this)
    return *this;

  delete[] m_buf;
  m_buf = 0;
  if(from.m_len > 0) {
    m_buf = new char[from.m_len];
    memcpy(m_buf, from.m_buf, from.m_len);
    m_len = from.m_len;
    return *this;
  }
  else {
    m_buf = 0;
    m_len = 0;
    return *this;
  }
}

