// -*- C++ -*-
/**
 *  @file   EbControl.cc
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
#include "EventBuilder/EbControl.h"
#include "EventBuilder/senderThread.h"
#include "EventBuilder/builderThread.h"
#include "EventBuilder/readerThread.h"


int EbControl::ackStatus()
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  std::string flags;
  std::ostringstream oss;

  GlobalInfo & gi = GlobalInfo::getInstance();
  SenderThread *sender = reinterpret_cast<SenderThread *>(gi.sender);
  BuilderThread *builder = reinterpret_cast<BuilderThread *>(gi.builder);

  flags = "";

  int f_running = 0;
  int f_idle = 0;

  flags = flags + "s:";
  if (sender->getState() == RUNNING) {
    f_running++;
    flags = flags + "R";
  } else {
    f_idle++;
    flags = flags + "I";
  }
  oss.str("");
  oss.clear();
  oss << sender->getEventNumber();
  flags = flags + ":" + oss.str() + " ";


  flags = flags + "b:";
  if (builder->getState() == RUNNING) {
    f_running++;
    flags = flags + "R";
  } else {
    f_idle++;
    flags = flags + "I";
  }
  oss.str("");
  oss.clear();
  oss << builder->getEventNumber();
  oss << "/" << builder->leftEventData();
  flags = flags + ":" + oss.str() + " ";

  flags = flags + "r( ";
  std::vector<StatableThread *>::iterator it;
  for (it = gi.readers.begin() ; it != gi.readers.end(); it++) {
    if ((*it)->getState() == RUNNING) {
      f_running++;
      flags = flags + "R";
    } else {
      f_idle++;
      flags = flags + "I";
    }
    oss.str("");
    oss.clear();
    oss << reinterpret_cast<ReaderThread *>(*it)->getEventNumber();
    oss << "/" << reinterpret_cast<ReaderThread *>(*it)->leftEventData();
    flags = flags + ":" + oss.str() + " ";
  }
  flags = flags + ")";

  if ((f_running > 0) && (f_idle == 0)) {
    gi.state = RUNNING;
  } else
    if ((f_running == 0) && (f_idle > 0)) {
      gi.state = IDLE;
    } else {
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

int EbControl::CommandStart()
{
#if 1
  GlobalInfo & gi = GlobalInfo::getInstance();
  //GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  std::vector<StatableThread *>::iterator it;
  for (it = gi.readers.begin() ; it != gi.readers.end(); it++) {
    (*it)->setCommand(START);
    //std::cerr << "#D CT Reader START" << std::endl;
  }
  gi.builder->setCommand(START);
  //std::cerr << "#D CT Builder START" << std::endl;
  gi.sender->setCommand(START);
  //std::cerr << "#D CT Sender START" << std::endl;
#endif

#if 0
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
#endif

  return 0;
}

int EbControl::CommandStop()
{
  GlobalInfo & gi = GlobalInfo::getInstance();
  //GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  //SenderThread *sender = reinterpret_cast<SenderThread *>(gi.sender);
  BuilderThread *builder = reinterpret_cast<BuilderThread *>(gi.builder);
  //std::vector <ReaderThread *> readers = gi.readers;


  std::cerr << "#D EBCONTROL SB left: "
	    << builder->leftEventData()
	    << " RB left: ";

  std::vector<StatableThread *>::iterator it;
  for (it = gi.readers.begin() ; it != gi.readers.end() ; ++it) {
    std::cerr << reinterpret_cast<ReaderThread *>(*it)->leftEventData()
	      << " ";
  }

  std::cerr << std::endl;

  int sb_left = builder->leftEventData();
  int sb_depth = builder->getRingBufferDepth();
  if (sb_left < 0.8 * sb_depth) {
    gi.sender->setCommand(STOP);
    usleep(10000);
    if (gi.sender->getState() != IDLE) usleep(50000);
    gi.builder->setCommand(STOP);
    usleep(10000);
    if (gi.builder->getState() != IDLE) usleep(50000);
    std::vector<StatableThread *>::iterator it;
    for (it = gi.readers.begin() ; it != gi.readers.end(); it++) {
      (*it)->setCommand(STOP);
    }
    /*
      std::list < StatableThread * >::iterator it;
      std::list < StatableThread * >::reverse_iterator rit;
      for (it = m_slave_list.begin();
      it != m_slave_list.end(); it++) {
      (*it)->setCommand(STOP);
      while ((*it)->getState() != IDLE) usleep(10000);
      std::cerr << "#D CT STOP" << std::endl;
      }
    */
  } else {
    std::cerr << "#D EbC STOP AT BUFFER FULL " << sb_left << std::endl;
    gi.sender->setCommand(STOP);
    gi.builder->setCommand(STOP);
    std::vector<StatableThread *>::iterator it;
    for (it = gi.readers.begin() ; it != gi.readers.end(); it++) {
      (*it)->setCommand(STOP);
    }
    /// release semaphore lock ///
    builder->peekReadMergData();
    builder->releaseReadMergData();
    ///
  }

  usleep(10000);
  /// release semaphore lock for Sender ///
  if (gi.sender->getState() != IDLE) {
    if (gi.builder->leftEventData() <= 0) {
      /// release semaphore lock ///
      builder->peekWriteMergData();
      builder->releaseWriteMergData();
      std::cerr << "#D EbC post Builder RB for S"<< std::endl;
    }
  }

  /// release semaphore lock for Builder ///
  if (gi.builder->getState() != IDLE) {
    for (it = gi.readers.begin() ; it != gi.readers.end(); it++) {
      ReaderThread *reader = reinterpret_cast<ReaderThread *>(*it);
      if (reader->leftEventData() <= 0) {
	reader->peekWriteFragData();
	reader->releaseWriteFragData();
	std::cerr << "#D EbC post Reader RB for B" << std::endl;
      }
    }
    //if (builder->leftEventData() > 0) {
    if (builder->leftEventData() >= builder->getRingBufferDepth()) {
      builder->peekReadMergData();
      builder->releaseReadMergData();
      std::cerr << "#D EbC post Seader RB for B" << std::endl;
    }
  }

  /// release semaphore lock for readers ///
  for (it = gi.readers.begin() ; it != gi.readers.end(); it++) {
    ReaderThread *reader = reinterpret_cast<ReaderThread *>(*it);
    if (reader->leftEventData() >= reader->getRingBufferDepth()) {
      reader->peekReadFragData();
      reader->releaseReadFragData();
      std::cerr << "#D EbC relase Reader RB" << std::endl;
    }
  }

  return 0;
}
