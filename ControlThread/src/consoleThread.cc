// -*- C++ -*-
/**
 *  @file   consoleThread.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: consoleThread.cc,v 1.2 2009/07/01 07:38:17 igarashi Exp $
 *  $Log: consoleThread.cc,v $
 *  Revision 1.2  2009/07/01 07:38:17  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.3  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.2  2008/02/27 16:24:46  igarashi
 *  Control sequence was introduced in ConsoleThread and Eventbuilder.
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
 *  Revision 1.2  2007/03/07 15:25:56  igarashi
 *  change MessageClient to GlobalMessageClient
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#include <iostream>
#include <sstream>
#include <string>
#include "kol/kolthread.h"
#include "ControlThread/consoleThread.h"
#include "ControlThread/statableThread.h"
#include "ControlThread/GlobalInfo.h"
#include "Message/GlobalMessageClient.h"


//ConsoleThread::ConsoleThread(int slave_num)
ConsoleThread::ConsoleThread()
{
  //	m_state = IDLE;
  //	m_command = STOP;
  std::cerr << "ConsoleThread created m_command " << m_command << std::endl;
}

ConsoleThread::~ConsoleThread()
{
  std::cerr << "ConsoleThread destructed" << std::endl;
}

void ConsoleThread::setSlave(StatableThread * slave)
{
  m_slave_list.push_back(slave);
}

State ConsoleThread::get_gstate()
{
  State gstate;
  gstate = IDLE;
  std::list < StatableThread * >::iterator it;
  for (it = m_slave_list.begin(); it != m_slave_list.end(); ++it) {
    if ((*it)->getState() == RUNNING) {
      gstate = RUNNING;
    }
  }
  return gstate;
}

int ConsoleThread::ack_status()
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  State gstate;
  std::string flags;
  std::ostringstream oss;

  flags = "";
  gstate = IDLE;
  std::list < StatableThread * >::iterator it;
  for (it = m_slave_list.begin(); it != m_slave_list.end(); it++) {
    if ((*it)->getState() == RUNNING) {
      gstate = RUNNING;
      flags = flags + "R";
    } else {
      flags = flags + "I";
    }
    oss.str("");
    oss.clear();
    oss << (*it)->getEventNumber();
    //oss << "/" << (*it)->leftEventData();
    flags = flags + ":" + oss.str() + " ";
  }

  if (gstate == IDLE) {
    flags = "IDLE " + flags;
    msock.sendString(MT_STATUS, &flags);
    std::cout << flags << std::endl;
  } else if (gstate == RUNNING) {
    flags = "RUNNING " + flags;
    msock.sendString(MT_STATUS, &flags);
    std::cout << flags << std::endl;
  } else {
    msock.sendString(MT_STATUS, "UNKNOWN");
  }

  return 0;
}

int ConsoleThread::send_entry()
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  GlobalInfo & gi = GlobalInfo::getInstance();

  std::string mstring = "ENTRY " + gi.nickname;
  msock.sendString(MT_STATUS, &mstring);

  return 0;
}

int ConsoleThread::CommandStart()
{
  std::list < StatableThread * >::iterator it;
  std::list < StatableThread * >::reverse_iterator rit;

  for (rit = m_slave_list.rbegin();
       rit != m_slave_list.rend(); ++rit) {
    (*rit)->setCommand(START);
    std::cerr << "#D CT START" << std::endl;
    /*
      std::cerr << "#D CT STA";
      while((*rit)->getState() != RUNNING) {
      std::cerr << (*rit)->getState();
      usleep(10000);
      }
      std::cerr << "RT:" << m_command << std::endl;
    */
  }

  return 0;
}

int ConsoleThread::CommandStop()
{
  std::list < StatableThread * >::iterator it;
  std::list < StatableThread * >::reverse_iterator rit;

  for (it = m_slave_list.begin();
       it != m_slave_list.end(); it++) {
    (*it)->setCommand(STOP);
    while ((*it)->getState() != IDLE) usleep(10000);
    std::cerr << "#D CT STOP" << std::endl;
  }

  return 0;
}


void ConsoleThread::simple_parse(std::string smessage)
{
  std::string comm;
  int val;
  std::istringstream ss(m_buffer);
  ss >> comm;
  ss >> val;

  if (comm == "start" || comm == "START")
    m_command = START;
  else if (comm == "stop" || comm == "STOP")
    m_command = STOP;
  else if (comm == "exit" || comm == "EXIT")
    m_command = EXIT;
  else if (comm == "stat" || comm == "STAT")
    m_command = STAT;
  else if (comm == "status" || comm == "STATUS")
    m_command = STAT;
  else if (comm == "anyone" || comm == "ANYONE")
    m_command = ANYONE;
  else if (comm == "runno" || comm == "RUNNO" || comm == "run") {
    m_command = RUNNO;
    m_run_number = val;
    std::cerr << "$$ RUN NO:" << val << std::endl;
  } else if (comm == "maxevent" || comm == "MAXEVENT" || comm == "max") {
    m_command = MAXEVENT;
    m_max_event = val;
    std::cerr << "$$ MAX EVENT:" << val << std::endl;
  } else
    m_command = NOCOMM;
}

int ConsoleThread::run()
{
  std::cerr << "ConsoleThread::run() " << std::endl;

  GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  while (true) {

    Message rmsg = msock.recvMessage();

    if (msock.gcount() > 0) {
      m_buffer = rmsg.getMessage();
      simple_parse(m_buffer);
      std::cout << "ConsoleThread::run() m_buffer: "
		<< m_buffer << std::endl;
      std::cout.flush();
    } else {
      break;
    }
    /*
      std::list <StatableThread *>::iterator it;
      for(it=m_slave_list.begin(); it!=m_slave_list.end();it++){
      (*it)->setCommand(m_command);
      if(m_command == MAXEVENT)
      (*it)->setMaxEvent(m_max_event);
      if(m_command == RUNNO)
      (*it)->setRunNumber(m_run_number);
      }
    */

    std::list < StatableThread * >::iterator it;
    std::list < StatableThread * >::reverse_iterator rit;
    switch (m_command) {
    case START:
#if 0
      for (rit = m_slave_list.rbegin();
	   rit != m_slave_list.rend(); ++rit) {
	(*rit)->setCommand(m_command);
	std::cerr << "#D CT START:" << m_command
		  << std::endl;
	/*
	  std::cerr << "#D CT STA";
	  while((*rit)->getState() != RUNNING) {
	  std::cerr << (*rit)->getState();
	  usleep(10000);
	  }
	  std::cerr << "RT:" << m_command << std::endl;
	*/
      }
#endif
      CommandStart();
      break;
    case STOP:
#if 0
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setCommand(m_command);
	while ((*it)->getState() != IDLE) usleep(10000);
	std::cerr << "#D CT STOP:" << m_command
		  << std::endl;
      }
#endif
      CommandStop();
      break;
    case MAXEVENT:
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setMaxEvent(m_max_event);
      }
      break;
    case RUNNO:
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setRunNumber(m_run_number);
      }
      break;
    case STAT:
      ack_status();
      break;
    case ANYONE:
      send_entry();
      break;
    default:
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setCommand(m_command);
      }
    }


  }
  std::cerr << "ConsoleThread finished" << std::endl;

  return 0;
}
