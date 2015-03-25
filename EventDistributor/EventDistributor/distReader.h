// -*- C++ -*-
/**
 *  @file   distReader.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: distReader.h,v 1.7 2012/04/16 11:23:05 igarashi Exp $
 *  $Log: distReader.h,v $
 *  Revision 1.7  2012/04/16 11:23:05  igarashi
 *  include Tomo's improvements
 *
 *  Revision 1.6  2010/07/02 12:13:54  igarashi
 *  change buffer modified flag from mutex to semaphore to against Ubuntu bug.
 *
 *  Revision 1.5  2009/06/29 02:44:26  igarashi
 *  debug Monitor data transport about buffer locking
 *
 *  Revision 1.4  2009/04/14 06:20:50  igarashi
 *  Debug for Monitor buffer relay sequence
 *  Debug startup threads sequence, wait reader threads until the ring buffer initialized
 *
 *  Revision 1.3  2009/04/10 09:25:04  igarashi
 *  add trywait interface in distReader
 *
 *  Revision 1.2  2008/06/27 15:30:18  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.3  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.2  2008/04/08 07:10:49  igarashi
 *  *** empty log message ***
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
#ifndef DIST_READER_THREAD_H
#define DIST_READER_THREAD_H

#include <iostream>
#include <exception>
#include "kol/kolthread.h"
#include "kol/koltcp.h"
#include "RingBuffer/RingBuffer.h"
#include "EventData/EventBuffer.h"
#include "EventData/EventParam.h"
#include "ControlThread/statableThread.h"


class DistReader : public StatableThread
{
  static const int HEADER_BYTE_SIZE  = 8;
  static const int EVENT_NO_OFFSET   = 8;

public:
  DistReader(int buflen, int quelen);
  virtual ~DistReader();
  void setHost(const char *host, int port);
  void initBuffer();
  EventBuffer * peekReadEventData();
  EventBuffer * peekWriteEventData();
  int           releaseReadEventData();
  int           releaseWriteEventData();
  int           leftEventData();
  int           getRingBufferDepth();
  int           trywaitRingBufferEmpty();
//   int setMonData(char *dest, char *src, size_t size);
  int setMonData(char *src, size_t size);
  EventBuffer * getMonData();
//   void setSemWaitMon();
//   void setSemPostMon();
  void releaseMonData(); //called by monDataSender
 
protected:
  virtual int active_loop();


private:
  int connect(kol::TcpClient& client);
  int readHeader(kol::TcpClient& client,
		 unsigned int* header);
  int updateEventData(kol::TcpClient& client,
		      unsigned int* header,
		      int trans_byte);
  
private:
  const char * m_host;
  int    m_port;
  int    m_node;
  bool   m_running;
  RingBuffer * m_dist_rb;
  EventBuffer * m_monData;
//  kol::Semaphore m_mondata_sem;
  kol::Semaphore m_mondata_modified;
//  kol::Mutex m_mondata_modified;
  kol::Mutex m_mondata_locker;
};

#endif
