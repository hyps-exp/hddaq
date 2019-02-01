// -*- C++ -*-
/**
 *  @file   RingBuffer.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: RingBuffer.cc,v 1.4 2012/04/13 08:39:18 igarashi Exp $
 *  $Log: RingBuffer.cc,v $
 *  Revision 1.4  2012/04/13 08:39:18  igarashi
 *  include Tomo's improvement, from EventBuffer array to EventBuffer vector
 *
 *  Revision 1.3  2009/06/29 02:52:11  igarashi
 *  clear buffer when the ring buffer is initialized
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

#include <iostream>
#include "RingBuffer/RingBuffer.h"

#define GLOBAL_LOCK

////
RingBuffer::RingBuffer(int buflen, int quelen)
  : m_quelen(quelen),
    m_buflen(buflen),
    m_write_ptr(0),
    m_read_ptr(0),
    m_len(0),
    m_empty(quelen),
    m_filled(0)
{
  try {
    m_buf.resize(m_quelen);
    for (int i=0; i<m_quelen; ++i) {
      m_buf[i] = new EventBuffer(m_buflen);
    }
  }
  catch (const std::bad_alloc& e) {
    std::cerr << "#E Can not allocate memory" << std::endl;
    m_buf.clear();
    throw;
  }
}

////
RingBuffer::~RingBuffer()
{
  for(int i=0; i<m_quelen; ++i) {
    delete m_buf[i];
    m_buf[i] = 0;
  }
  m_buf.clear();
  std::cerr << "Ringbuffer deleted\n";
}

////
void
RingBuffer::initBuffer()
{
  m_rwlock.lock();
  m_write_ptr = 0;
  m_read_ptr  = 0;
  m_len       = 0;
  m_empty     = m_quelen;
  m_filled    = 0;
  for(int i=0; i<m_quelen; i++)
    m_buf[i]->clear();
  m_rwlock.unlock();
}

////
EventBuffer*
RingBuffer::readBufPeek()
{
  //std::cerr << "readBufPeek()" << std::endl;
  m_filled.wait();
  return m_buf[m_read_ptr];
}

////
int
RingBuffer::readBufRelease() {
  //std::cerr << "readBufRelease()" << std::endl;
  m_rwlock.lock();
  m_read_ptr = (m_read_ptr + 1)%m_quelen;
  m_len = m_len - 1;
  m_rwlock.unlock();
  m_empty.post();
  return 0;
}

////
EventBuffer*
RingBuffer::writeBufPeek() {
  //std::cerr << "writeBufPeek()" << std::endl;
  m_empty.wait();
  return m_buf[m_write_ptr];
}

////
int
RingBuffer::writeBufRelease() {
  // std::cerr << "writeBufRelease()" << std::endl;
  m_rwlock.lock();
  m_write_ptr = (m_write_ptr + 1)%m_quelen;
  m_len = m_len + 1;
  m_rwlock.unlock();
  m_filled.post();
  return 0;
}

int RingBuffer::left()
{
  return m_len;
}

int RingBuffer::BufSize()
{
  return m_buflen;
}

int RingBuffer::depth()
{
  return m_quelen;
}

int RingBuffer::trywaitFill()
{
  return m_filled.trywait();
}

int RingBuffer::trywaitEmpty()
{
  return m_empty.trywait();
}
