// -*- C++ -*-
/**
 *  @file   EventBuffer.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date   
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: EventBuffer.h,v 1.2 2009/06/29 02:32:12 igarashi Exp $
 *  $Log: EventBuffer.h,v $
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
#ifndef EVENT_BUFFER_H
#define EVENT_BUFFER_H

#include <iostream>
#include <assert.h>

class EventBuffer
{
public:
  // constructor/destructor
  EventBuffer(int len);
  ~EventBuffer();

  EventBuffer(const EventBuffer& from);

  void clear();

  // operator
  EventBuffer& operator=(const EventBuffer &from);

  // extructor
  char* getBuf() { return m_buf; }
  int   getLen() { return m_len; }

  unsigned getHeader() { return * (unsigned *)m_buf; }
  unsigned getLength() { return * ((unsigned *)m_buf + 1); }
  unsigned getEventNo() { return * ((unsigned *)m_buf + 2); }

private:
  char *m_buf;
  int m_len;

};

#endif
