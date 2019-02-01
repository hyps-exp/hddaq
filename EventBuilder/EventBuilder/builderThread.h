// -*- C++ -*-
/**
 *  @file   builderThread.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: builderThread.h,v 1.5 2012/04/13 12:04:11 igarashi Exp $
 *  $Log: builderThread.h,v $
 *  Revision 1.5  2012/04/13 12:04:11  igarashi
 *  include Tomo's improvements
 *    slowReader
 *
 *  Revision 1.4  2010/07/02 12:11:56  igarashi
 *  add to release a send buffer in builder thread when EB get stop command
 *
 *  Revision 1.3  2009/12/15 08:27:30  igarashi
 *  minar update for error message and trigger rate display
 *
 *  Revision 1.2  2008/05/16 06:29:22  igarashi
 *  update eventbuilder control sequence
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.4  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.3  2008/02/27 16:24:46  igarashi
 *  Control sequence was introduced in ConsoleThread and Eventbuilder.
 *
 *  Revision 1.2  2008/02/25 13:24:20  igarashi
 *  Threads start timing was changed.
 *  Threads start as series, reader, builder, sender.
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
 *  Revision 1.2  2007/02/20 00:02:25  nakayosi
 *  changed all members "protected" to "private"
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#ifndef BUILDER_THREAD_H
#define BUILDER_THREAD_H

#include <iostream>
#include <iomanip>
#include "kol/kolthread.h"
#include "RingBuffer/RingBuffer.h"
#include "EventData/EventBuffer.h"
#include "EventData/EventParam.h"
#include "EventBuilder/readerThread.h"
#include "ControlThread/statableThread.h"

class BuilderThread : public StatableThread
{
  //static const int SEND_RB_BUFLEN   = 20;

public:
  BuilderThread(int buflen, int quelen);
  virtual ~BuilderThread();
  void setAllReaders(ReaderThread **readers, int node_num);

  void initAllNodeBuffers();
  void initSendBuffer();
  EventBuffer * peekReadMergData();
  int releaseReadMergData();
  EventBuffer * peekWriteMergData();
  int releaseWriteMergData();
  int leftEventData();
  int getRingBufferDepth();
  void setSemPost();

  void setDebugPrint(int d_print);
  void setParaFd(int fd_para);
  void getOneShot();

protected:
  int    active_loop();
  bool   checkNodeRB();
  double checkTrigRate(int ntimes);
  int    checkEventNumber();
  int    waitReaders();

private:
  int m_node_num;
  int m_fd_para;
  int m_debug_print;

  ReaderThread** m_readers;
  EventBuffer*   m_event_f[max_node_num];
  RingBuffer*    m_send_rb;
};

#endif
