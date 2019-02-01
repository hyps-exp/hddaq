// -*- C++ -*-
/**
 *  @file   controlThread.h
 *  @brief
 *  @author
 *  @date
 *
 */
#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include "kol/kolthread.h"
#include "ControlThread/statableThread.h"
#include "Message/MessageClient.h"


//class ControlThread : public StatableThread
class ControlThread : public kol::Thread
{
public:
  ControlThread();
  virtual ~ControlThread();
  void setSlave(StatableThread *);
  virtual int ackStatus();
  virtual int sendEntry();
  virtual State getGeneralState();
  virtual int CommandStart();
  virtual int CommandStop();

protected:
  void simple_parse(std::string);
  int run();

  std::list<StatableThread *> m_slave_list;
  std::string m_buffer;

  Command m_command;
  int m_run_number;
  int m_max_event;
private:

};

extern bool g_VERBOSE;

#endif
