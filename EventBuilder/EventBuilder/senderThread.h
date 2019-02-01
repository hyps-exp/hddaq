// -*- C++ -*-
/**
 *  @file   senderThread.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: senderThread.h,v 1.4 2012/04/13 12:04:11 igarashi Exp $
 *  $Log: senderThread.h,v $
 *  Revision 1.4  2012/04/13 12:04:11  igarashi
 *  include Tomo's improvements
 *    slowReader
 *
 *  Revision 1.3  2010/06/30 12:14:09  igarashi
 *  put Time-out in senderThread socket write
 *
 *  Revision 1.2  2009/04/07 09:34:08  igarashi
 *  sender thread re-run automatically without run control
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.3  2008/05/13 06:41:57  igarashi
 *  change control sequence
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
 *  Revision 1.2  2007/02/20 00:04:02  nakayosi
 *  changed all members "protected" to "private"
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#ifndef SENDER_THREAD_H
#define SENDER_THREAD_H

#include <iostream>
#include "kol/kolthread.h"
#include "kol/koltcp.h"
#include "ControlThread/statableThread.h"
#include "RingBuffer/RingBuffer.h"
#include "EventBuilder/builderThread.h"
#include "EventBuilder/EventBuilder.h"


class SenderThread : public StatableThread
{
public:
  SenderThread(int buflen, int quelen);
  virtual ~SenderThread();
  void setBuilder(BuilderThread *builder);
  void setSemPost();

protected:
  int active_loop();
  int run();
  int waitBuilder();

private:
  BuilderThread * m_builder;
};

#endif
