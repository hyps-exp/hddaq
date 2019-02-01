// -*- C++ -*-
/**
 *  @file   EdControl.cc
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
#include "EventDistributor/EdControl.h"
#include "EventDistributor/distReader.h"
#include "EventDistributor/dataSender.h"
#include "EventDistributor/monDataSender.h"


int EdControl::ackStatus()
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  GlobalInfo & gi = GlobalInfo::getInstance();
  DistReader *reader = reinterpret_cast<DistReader *>(gi.reader);
  DataSender *sender = reinterpret_cast<DataSender *>(gi.sender);
  MonDataSender *monsender = reinterpret_cast<MonDataSender *>(gi.monsender);

  std::string flags;
  std::ostringstream oss;

  flags = "";
  //gi.state = IDLE;

  flags = flags + "s:";
  if (sender->getState() == RUNNING) {
    //gi.state = RUNNING;
    flags = flags + "R";
  } else {
    flags = flags + "I";
  }
  oss.str("");
  oss.clear();
  oss << sender->getEventNumber();
  flags = flags + ":" + oss.str() + " ";

  flags = flags + "m";
  /*
    if (monsender->getState() == RUNNING) {
    gi.state = RUNNING;
    flags = flags + "R";
    } else {
    flags = flags + "I";
    }
  */
  oss.str("");
  oss.clear();
  oss << monsender->getEventNumber();
  flags = flags + ":" + oss.str() + " ";

  flags = flags + "r:";
  if (reader->getState() == RUNNING) {
    flags = flags + "R";
  } else {
    flags = flags + "I";
  }
  oss.str("");
  oss.clear();
  oss << reader->getEventNumber();
  oss << "/" << reader->leftEventData();
  flags = flags + ":" + oss.str() + " ";

  if ((sender->getState() == IDLE) && (reader->getState() == IDLE)) {
    gi.state = IDLE;
  } else
    if ((sender->getState() == RUNNING) && (reader->getState() == RUNNING)) {
      gi.state = RUNNING;
    } else {
      //if (sender->getState() != reader->getState()) {
      gi.state = DISORDER;
    }

#if 0
  std::list < StatableThread * >::iterator it;
  for (it = m_slave_list.begin(); it != m_slave_list.end(); it++) {
    if ((*it)->getState() == RUNNING) {
      gi.state = RUNNING;
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
#endif

  if (gi.state == IDLE) {
    flags = "IDLE " + flags;
    msock.sendString(MT_STATUS, &flags);
    if (g_VERBOSE) std::cout << flags << std::endl;
  } else if (gi.state == RUNNING) {
    flags = "RUNNING " + flags;
    msock.sendString(MT_STATUS, &flags);
    if (g_VERBOSE) std::cout << flags << std::endl;
  } else if (gi.state == DISORDER) {
    flags = "DISORDER " + flags;
    msock.sendString(MT_STATUS, &flags);
    if (g_VERBOSE) std::cout << flags << std::endl;
  } else {
    msock.sendString(MT_STATUS, "UNKNOWN");
  }

  return 0;
}

int EdControl::CommandStart()
{
  //GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  //GlobalInfo & gi = GlobalInfo::getInstance();
  //DistReader *reader = reinterpret_cast<DistReader *>(gi.reader);
  //DataSender *sender = reinterpret_cast<DataSender *>(gi.sender);

  std::list <StatableThread *>::iterator it;
  std::list <StatableThread *>::reverse_iterator rit;

  for (rit = m_slave_list.rbegin();
       rit != m_slave_list.rend(); ++rit) {
    (*rit)->setCommand(START);
    std::cerr << "#D CT START" << std::endl;
  }

  return 0;
}

int EdControl::CommandStop()
{
  GlobalInfo & gi = GlobalInfo::getInstance();
  //GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  DistReader *reader = reinterpret_cast<DistReader *>(gi.reader);
  DataSender *sender = reinterpret_cast<DataSender *>(gi.sender);

  std::list < StatableThread * >::iterator it;
  std::list < StatableThread * >::reverse_iterator rit;
  for (it = m_slave_list.begin();
       it != m_slave_list.end(); it++) {
    (*it)->setCommand(STOP);
    std::cerr << "#D EdC write STOP" << std::endl;
  }

  const int MAX_TRY = 100;
  int trycount = 0;
  while (
	 ((sender->getState() != IDLE)
	  || (reader->getState() != IDLE))
	 && (trycount++ < MAX_TRY)) {
    usleep(100000);
    std::cerr << "#D " << trycount
	      << " EdC: s:" << sender->getState()
	      << " r:" << reader->getState()
	      << " rl:" << reader->leftEventData()
	      << std::endl;
    //std::cerr << "#D EdC trywait : "
    //	<< reader->trywaitRingBufferEmpty() << std::endl;
    if ((sender->getState() != IDLE)
	&& (reader->getState() == IDLE)
	&& (reader->leftEventData() <= 0)
	//&& (reader->trywaitRingBufferEmpty() != 0)
	) {
      reader->peekWriteEventData();
      reader->releaseWriteEventData();

      std::cerr << "#D EdC put forward RingBuf for release lock."
		<< std::endl;
    }
    if ((sender->getState() == IDLE)
	&& (reader->getState() != IDLE)
	&& (reader->leftEventData() > 0)
	//&& (reader->trywaitRingBufferEmpty() != 0)
	) {
      reader->peekReadEventData();
      reader->releaseReadEventData();
    }

  }
  if (trycount >= MAX_TRY) {
    std::cerr << "#D EdC gives up to stop sender Thread."
	      << std::endl;
  }

  return 0;
}
