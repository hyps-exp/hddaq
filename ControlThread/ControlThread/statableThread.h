// -*- C++ -*-
/**
 *  @file   statableThread.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: statableThread.h,v 1.5 2012/04/13 07:33:47 igarashi Exp $
 *  $Log: statableThread.h,v $
 *  Revision 1.5  2012/04/13 07:33:47  igarashi
 *  include Tomo's improvement
 *
 *  Revision 1.4  2009/12/15 08:16:50  igarashi
 *  minar change
 *
 *  Revision 1.3  2009/07/01 07:38:17  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/06/27 15:30:18  igarashi
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
 *  Revision 1.2  2007/02/19 23:51:55  nakayosi
 *  changed a member m_is_running "protected" to "private"
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#ifndef STATABLE_THREAD_H
#define STATABLE_THREAD_H

#include <string>
#include "kol/koltcp.h"
#include "kol/kolthread.h"


enum State {
  IDLE, RUNNING, DISORDER
};

enum Command {
  START, STOP, EXIT, STAT, ANYONE, MAXEVENT, RUNNO, NOCOMM
};

class StatableThread : public kol::Thread
{
public:
  StatableThread();
  virtual ~StatableThread();

public:

  //  enum Command {
  //    START, STOP, EXIT, STAT, MAXEVENT, RUNNO, NOCOMM
  //  };

  //  enum State {
  //    IDLE, RUNNING
  //  };

  enum Ack {
    DONE, YET, OK
  };

  void reset_command() { m_command = NOCOMM; }
  void setRunningFalse() { m_is_running = false; }
  //virtual void setSemPost() { return; }

  virtual int run();
  void setCommand(Command command) { m_command = command; };
  int getState() { return m_state; }
  int getCommand() const { return m_command; }
  int setRunNumber(int run_number);
  int getRunNumber() const { return m_run_number; }
  int setMaxEvent(int max_event);
  int getMaxEvent() const {return m_max_event;}
  int getEventNumber() const {return m_event_number;}
  virtual int leftEventData(){return 0;}
  void setName(const std::string& name) { m_name = name; }

protected:
  virtual int active_loop() { return 0; }
  virtual int checkCommand();
  int getRunNumber();
  void trans(State target) { m_state = target; }

protected:
  Command      m_command;
  State        m_state;
  int          m_run_number;
  unsigned     m_max_event;
  unsigned int m_event_number;
  std::string  m_name;
  kol::Mutex   m_mutex;

private:
  //Ack   m_ack;
  bool m_is_running;
};


size_t checkDataSize(unsigned int max_data_size,
		     size_t& size,
		     const std::string& name);
int checkHeader(unsigned int magic,
		const std::string& name);
int checkTcp(kol::TcpBuffer& tcp,
	     const std::string& name,
	     int port=-1);

#endif
