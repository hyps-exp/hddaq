// -*- C++ -*-
/*
 *
 *
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <string>
#include <sstream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "Message/GlobalMessageClient.h"

#include "controlthread.h"
#include "nodeprop.h"
#include "daqthread.h"

ControlThread::ControlThread(struct node_prop *nodeprop)
  :m_gstate_mutex(),
   m_nodeprop(nodeprop)
{
}

ControlThread::~ControlThread()
{
	std::cerr << "ControlThread destructed" << std::endl;
}

int ControlThread::ackStatus()
{
	DaqThread *daqthread  = reinterpret_cast<DaqThread *>(m_nodeprop->daq_thread);
	GlobalMessageClient& msock = GlobalMessageClient::getInstance();

	std::ostringstream oss; 
	if (daqthread->getState() == IDLE) {
		oss << "IDLE ";
	} else if (daqthread->getState() == RUNNING) {
		oss << "RUNNING ";
        } else if (daqthread->getState() == INITIAL) {
		oss << "INITIAL ";
	} else if (daqthread->getState() == END) {
		oss << "END ";
	} else {
		oss << "UNKNOWN ";
	}
	if (daqthread->getDaqMode() == DM_NORMAL) {
		oss << "NORMAL ";
	} else if (daqthread->getDaqMode() == DM_DUMMY) {
		oss << "DUMMY ";
	} else {
		oss << "UNKNOWN ";
	}
	oss << daqthread->getEventNumber();
	oss << " size:" << daqthread->getEventSize();

	std::string smessage = oss.str();
	msock.sendString(MT_STATUS, smessage);

	return 0;
}

int ControlThread::sendEntry()
{
	GlobalMessageClient & msock = GlobalMessageClient::getInstance();

	std::string nickname(m_nodeprop->nickname);
	std::string message = "ENTRY " + nickname;

	msock.sendString(MT_STATUS, message);

	return 0;
}


int ControlThread::run()
{
	DaqThread *daqthread = reinterpret_cast<DaqThread *>(m_nodeprop->daq_thread);
	GlobalMessageClient& msock = GlobalMessageClient::getInstance();

	sendEntry();

	while (m_nodeprop->getState() != END) {
		std::string messageline;
		Message rmessage = msock.recvMessage();
		if (msock.gcount() > 0) {
		  messageline = rmessage.getMessage();
		}

		std::cerr << "#D Message : " << messageline << std::endl;
		
		if (messageline == "status") {
			ackStatus();
		}
		if (messageline == "dummy_mode") {
			if(daqthread->setDaqMode(DM_DUMMY)) {
				ackStatus();
				std::cerr << "DUMMY MODE" << std::endl;
			} else {
				std::ostringstream oss; 
				oss << "Mode change fail : " << daqthread->getDaqMode();
				std::string smessage = oss.str();
				msock.sendString(MT_ERROR, smessage);

				std::cerr << "Mode change Fail "
					<< daqthread->getDaqMode() << std::endl;
				
			}
		}
		if (messageline == "normal_mode") {
			if (daqthread->setDaqMode(DM_NORMAL)) {
				ackStatus();
				std::cerr << "NORMAL MODE" << std::endl;
			} else {
				std::ostringstream oss;
				oss << "Mode change fail : " << daqthread->getDaqMode();
				std::string smessage = oss.str();
				msock.sendString(MT_ERROR, smessage);

				std::cerr << "Mode change Fail "
					<< daqthread->getDaqMode() << std::endl;
			}
		}
		
		int runno;
		if (sscanf(messageline.c_str(), "run %d", &runno) == 1) {
			m_nodeprop->setRunNumber(runno);
		}
		int maxevent;
		if (sscanf(messageline.c_str(), "maxevent %d", &maxevent) == 1) {
			m_nodeprop->max_event = maxevent;
		}
		if (messageline == "start") {
		  m_nodeprop->setState(RUNNING);
		}
		if (messageline == "stop") {
		  m_nodeprop->setState(IDLE);
		}

		if (messageline == "anyone") {
			sendEntry();
		}
		if (messageline == "fe_end") {
		  m_nodeprop->setState(END);
		}
		if (messageline == "fe_exit") {
		  fprintf(stderr, "#D exit by fe_exit command\n");
		  exit(1);
		}
	}

	return 0;
}
