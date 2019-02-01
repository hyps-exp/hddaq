// -*- C++ -*-
/**
 *	@file	 statableThread.cc
 *	@brief
 *	@author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *	@date
 *
 *	$Id: statableThread.cc,v 1.3 2012/04/13 07:33:47 igarashi Exp $
 *
 *	$Log: statableThread.cc,v $
 *	Revision 1.3  2012/04/13 07:33:47  igarashi
 *	include Tomo's improvement
 *
 *	Revision 1.2  2009/12/15 08:16:50  igarashi
 *	minar change
 *
 *	Revision 1.1.1.1	2008/05/14 15:05:43	igarashi
 *	Network DAQ Software Prototype 1.5
 *
 *	Revision 1.4	2008/05/13 06:41:57	igarashi
 *	change control sequence
 *
 *	Revision 1.3	2008/02/27 16:24:46	igarashi
 *	Control sequence was introduced in ConsoleThread and Eventbuilder.
 *
 *	Revision 1.2	2008/02/25 13:24:20	igarashi
 *	Threads start timing was changed.
 *	Threads start as series, reader, builder, sender.
 *
 *	Revision 1.1.1.1	2008/01/30 12:33:33	igarashi
 *	Network DAQ Software Prototype 1.4
 *
 *	Revision 1.1.1.1	2007/09/21 08:50:48	igarashi
 *	prototype-1.3
 *
 *	Revision 1.1.1.1	2007/03/28 07:50:17	cvs
 *	prototype-1.2
 *
 *	Revision 1.2	2007/02/01 02:05:16	kensh
 *	Minor update.
 *
 *	Revision 1.1.1.1	2007/01/31 13:37:53	kensh
 *	Initial version.
 *
 *
 *
 *
 */
#include <iostream>
#include <sstream>

#include "ControlThread/statableThread.h"
#include "Message/GlobalMessageClient.h"

StatableThread::StatableThread()
  : kol::Thread(),
    m_command(STOP),
    m_state(IDLE),
    m_run_number(0),
    m_max_event(0),
    m_event_number(0),
    m_name(),
    m_mutex()
{
  //std::cerr << "StatableThread created\n";
  m_mutex.trylock();
}

StatableThread::~StatableThread()
{
  //std::cerr << "StatableThread deleted\n";
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  msock.sendString("StatbleThread destructed");
}

int StatableThread::run()
{
  // GlobalMessageClient& msock = GlobalMessageClient::getInstance();

  State myState = IDLE;
  m_is_running = true;

  while(m_is_running){
    //std::cerr <<"#D comm " << m_command << std::endl;
    switch(m_command) {
    case START: //command
      switch(myState) {
      case IDLE: //state
	reset_command();
	// std::cerr << "active_loop: started...\n";
	// trans(RUNNING); /// RUNNING ha active_loop no nakade yaru
	active_loop();
	// std::cerr << "active_loop: finished...\n";
	// trans(StatableThread::IDLE);
	trans(IDLE);
	// msock.sendString(m_name + " : active_loop() finished.");
	break;
      case RUNNING:
	break;
      case DISORDER:
	break;
      }
      break;

    case STOP: //command
      switch(myState) {
      case IDLE:
	break;
      case RUNNING: //state
	reset_command();
	//setSemPost();
	trans(IDLE);
	//std::cerr << "active: stopped...\n";
	break;
      case DISORDER:
	break;
      }
      break;

    case EXIT: //command
      switch(myState) {
      case IDLE: //state
	trans(IDLE);
	m_is_running = false;
	break;
      case RUNNING:
	break;
      case DISORDER:
	break;
      }
      break;

    case STAT: //command
      switch(myState) {
      case IDLE:
      case RUNNING: //state
	reset_command();
	//setCommand(StatableThread::STAT);
	setCommand(STAT);
	break;
      case DISORDER:
	break;
      }
      break;

    case MAXEVENT:
      break;
    case RUNNO:
      break;
    case NOCOMM:
      break;
    case ANYONE:
      break;
    }

    usleep(100000);
  }

  return 0;
}

int StatableThread::checkCommand()
{
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  int breakflag = 0;
  if (m_event_number == static_cast<unsigned int>(m_max_event)) {
    std::cerr << m_name  << ": Maximum Events "
	      << std::dec << " " << m_event_number
	      << " / " << m_max_event << std::endl;
    m_command = STOP;
    // 		m_event_number = 0;
    std::cerr << "Event_Num_Counter Init" << std::endl;
    msock.sendString(m_name + " : Maximum Events");
  }

  if (m_command == STOP) {// && m_state == RUNNING) {
    std::cerr << m_name << ": got STOP Command" << std::endl;
    m_state = IDLE;
    msock.sendString(m_name + " : got STOP Command");
    breakflag = 1;
  }
  return breakflag;
}

int StatableThread::getRunNumber()
{
  m_mutex.lock();
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  std::ostringstream msg;
  msg << m_name << " got RUN# = " << m_run_number;
  msock.sendString(msg);
  return m_run_number;
}

int StatableThread::setRunNumber(int run_number)
{
  m_run_number = run_number;
  m_mutex.unlock();
  return 0;
}


int StatableThread::setMaxEvent(int max_event)
{
  m_max_event = max_event;
  std::cerr << "Maximum Event No:" << m_max_event << std::endl;
  return 0;
}


size_t checkDataSize(unsigned int  max_bytes,
		     size_t& size,
		     const std::string& name)
{
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  if (size > max_bytes) {
    int ignore = size - max_bytes;
    size = max_bytes;
    std::ostringstream msg;
    msg << name << ": ERROR (Many data): " << size;
    msock.sendString(MT_WARNING, msg);
    return ignore;
  } else if (size<=0) {
    std::ostringstream msg;
    msg << name << ": ERROR (Invalid Total length): " << size;
    msock.sendString(MT_ERROR, msg);
    return -1;
  }
  return 0;
}

int checkHeader(unsigned int magic,
		const std::string& name)
{
  static const unsigned int EV_MAGIC = 0x45564e54;
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  if (magic!=EV_MAGIC) {
    std::ostringstream msg;
    msg << name << ": ERROR (Invalid Header): "
	<< std::hex << magic;
    std::cerr << "#E " << msg.str() << std::endl;
    msock.sendString(MT_ERROR, msg);
    return -1;
  }
  return 0;
}

int checkTcp(kol::TcpBuffer& tcp,
	     const std::string& name,
	     int port)
{
  std::ostringstream msg;
  msg << name;
  if (port>0) {
    msg << ":" << port;
  }
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  if (tcp.eof()) {
    msg << " : socket catch EOF";
    std::cerr << "#E " << msg.str() << std::endl;
    msock.sendString(MT_ERROR, msg);
    return -1;
  }
  if (tcp.gcount() == 0) {
    msg << " : socket gcount == 0";
    std::cerr << "#E " << msg.str() << std::endl;
    msock.sendString(MT_ERROR, msg);
    return -1;
  }
  if (!tcp.good()) {
    msg << " : socket not GOOD";
    std::cerr << "#E " << msg.str() << std::endl;
    msock.sendString(MT_ERROR, msg);
    return -1;
  }
  if (tcp.bad()) {
    msg << " : socket BAD";
    std::cerr << "#E " << msg.str() << std::endl;
    msock.sendString(MT_ERROR, msg);
    return -1;
  }
  return 0;
}

