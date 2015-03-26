// -*- C++ -*-
/**
 *  @file   RingBuffer.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date   
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: RingBuffer.h,v 1.3 2012/04/13 08:39:18 igarashi Exp $
 *  $Log: RingBuffer.h,v $
 *  Revision 1.3  2012/04/13 08:39:18  igarashi
 *  include Tomo's improvement, from EventBuffer array to EventBuffer vector
 *
 *  Revision 1.2  2009/04/07 09:24:23  igarashi
 *  add read write mutex for buffer length value
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.2  2008/05/13 06:41:57  igarashi
 *  change control sequence
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
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <iostream>
#include <vector>

#include "kol/kolthread.h"
#include "EventData/EventBuffer.h"

////
class RingBuffer {
public:
  // constructor/destructor
  RingBuffer(int buflen, int quelen=20);
  ~RingBuffer();

  // extructor
  EventBuffer* readBufPeek();
  EventBuffer* writeBufPeek();

  // method
  void initBuffer();
  int  readBufRelease();
  int  writeBufRelease();
  int  left();
  int  depth();
  int  BufSize();
  int  trywaitFill();
  int  trywaitEmpty();

protected:
  int m_quelen;
  int m_buflen;
  int m_write_ptr;
  int m_read_ptr;
  int m_len;
  kol::Semaphore m_empty;
  kol::Semaphore m_filled;
  kol::Mutex m_rwlock;
//   EventBuffer **m_buf;
  std::vector<EventBuffer*> m_buf;

  
};

#endif
