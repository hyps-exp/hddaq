// -*- C++ -*-
/**
 *  @file   consoleThread.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: consoleThread.h,v 1.2 2009/07/01 07:38:17 igarashi Exp $
 *  $Log: consoleThread.h,v $
 *  Revision 1.2  2009/07/01 07:38:17  igarashi
 *  *** empty log message ***
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
#ifndef CONSOLE_THREAD_H
#define CONSOLE_THREAD_H

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include "kol/kolthread.h"
#include "ControlThread/statableThread.h"
#include "Message/MessageClient.h"


class ConsoleThread : public StatableThread
{
public:
  //ConsoleThread(int thread_num);
  ConsoleThread();
  virtual ~ConsoleThread();
  void setSlave(StatableThread *slave);
  virtual int ack_status();
  virtual int send_entry();
  virtual State get_gstate();
  virtual int CommandStart();
  virtual int CommandStop();

protected: 
  void simple_parse(std::string smessage);
  int run();

private:
  std::list<StatableThread *> m_slave_list;
  std::string m_buffer;
};

#endif
