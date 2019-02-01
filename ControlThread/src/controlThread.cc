// -*- C++ -*-
/**
 *  @file   controlThread.cc
 *  @brief
 *  @author
 *  @date
 *
 */
#include <iostream>
#include <sstream>
#include <string>
#include "kol/kolthread.h"
#include "ControlThread/statableThread.h"
#include "ControlThread/controlThread.h"
#include "ControlThread/GlobalInfo.h"
#include "Message/GlobalMessageClient.h"


ControlThread::ControlThread()
  : m_command(NOCOMM)
{
  std::cerr << "ControlThread created" << std::endl;
}

ControlThread::~ControlThread()
{
  std::cerr << "ControlThread destructed" << std::endl;
}

void ControlThread::setSlave(StatableThread * slave)
{
  m_slave_list.push_back(slave);
}

State ControlThread::getGeneralState()
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

int ControlThread::ackStatus()
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
    if (g_VERBOSE) std::cout << flags << std::endl;
  } else if (gstate == RUNNING) {
    flags = "RUNNING " + flags;
    msock.sendString(MT_STATUS, &flags);
    if (g_VERBOSE) std::cout << flags << std::endl;
  } else {
    msock.sendString(MT_STATUS, "UNKNOWN");
  }

  return 0;
}

int ControlThread::sendEntry()
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  GlobalInfo & gi = GlobalInfo::getInstance();

  std::string mstring = "ENTRY " + gi.nickname;
  msock.sendString(MT_STATUS, &mstring);

  return 0;
}

int ControlThread::CommandStart()
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

int ControlThread::CommandStop()
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


void ControlThread::simple_parse(std::string smessage)
{
  std::string comm;
  int val;
  std::istringstream ss(m_buffer);
  ss >> comm;
  ss >> val;

  if (comm == "start" || comm == "START") {
    m_command = START;
  } else if (comm == "stop" || comm == "STOP") {
    m_command = STOP;
  } else if (comm == "exit" || comm == "EXIT") {
    m_command = EXIT;
  } else if (comm == "stat" || comm == "STAT") {
    m_command = STAT;
  } else if (comm == "status" || comm == "STATUS") {
    m_command = STAT;
  } else if (comm == "anyone" || comm == "ANYONE") {
    m_command = ANYONE;
  } else if (comm == "runno" || comm == "RUNNO" || comm == "run") {
    m_command = RUNNO;
    m_run_number = val;
    std::cerr << "CT RUN NO:" << val << std::endl;
  } else if (comm == "maxevent" || comm == "MAXEVENT" || comm == "max") {
    m_command = MAXEVENT;
    m_max_event = val;
    std::cerr << "CT MAX EVENT:" << val << std::endl;
  } else
    m_command = NOCOMM;
}

int ControlThread::run()
{
  std::cerr << "ControlThread::run() " << std::endl;

  GlobalInfo& gi = GlobalInfo::getInstance();
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  while (true) {

    Message rmsg = msock.recvMessage();

    if (msock.gcount() > 0) {
      m_buffer = rmsg.getMessage();
      simple_parse(m_buffer);
      std::cout << "ControlThread::run() m_buffer: "
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
      gi.state = RUNNING;
      CommandStart();
      break;
    case STOP:
      std::cerr << "#D ControlThread got STOP" << std::endl;
      gi.state = IDLE;
      CommandStop();
      std::cerr << "#D ControlThread done STOP" << std::endl;
      break;
    case MAXEVENT:
      gi.max_event = m_max_event;
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setMaxEvent(m_max_event);
      }
      break;
    case RUNNO:
      gi.run_number = m_run_number;
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setRunNumber(m_run_number);
      }
      break;
    case STAT:
      ackStatus();
      break;
    case ANYONE:
      sendEntry();
      break;
    default:
      for (it = m_slave_list.begin();
	   it != m_slave_list.end(); it++) {
	(*it)->setCommand(m_command);
      }
    }

  }
  std::cerr << "ControlThread finished" << std::endl;

  return 0;
}
