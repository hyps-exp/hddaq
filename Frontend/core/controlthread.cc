#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>

#include "controlthread.h"
#include "nodeprop.h"
#include "MessageHelper.h"

ControlThread::ControlThread(NodeProp& nodeprop)
  : m_nodeprop(nodeprop)
{
}

ControlThread::~ControlThread()
{
  std::cout << "ControlThread destructed" << std::endl;
}

int ControlThread::run()
{
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  m_nodeprop.sendEntry();
  
  while (1) {
    std::string messageline;
    Message rmessage = msock.recvMessage();
    if (msock.gcount() > 0) {
      messageline = rmessage.getMessage();
    }
    std::cout << "#D Message : " << messageline << std::endl;
    
    if (messageline == "status") {
      m_nodeprop.ackStatus();
    }

    if (messageline == "anyone") {
      m_nodeprop.sendEntry();
    }

    if (messageline == "dummy_mode") {
      m_nodeprop.setDaqMode(DM_DUMMY);
      m_nodeprop.ackStatus();
    }

    if (messageline == "normal_mode") {
      m_nodeprop.setDaqMode(DM_NORMAL);
      m_nodeprop.ackStatus();
    }

    if (messageline == "start") {
      m_nodeprop.setUpdate(true);
    }

    if (messageline == "stop") {
#ifdef END_AT_STOP
      m_nodeprop.setStateAck(END);
      std::cout << "#D end by stop" <<std::endl;
      send_normal_message("end by stop");
#else
      m_nodeprop.setStateAck(IDLE);
#endif
    }

    if (messageline == "fe_end") {
      m_nodeprop.setStateAck(END);
      std::cout << "#D end by fe_end" <<std::endl;
      send_normal_message("end by fe_end");
    }
    
    if (messageline == "fe_exit") {
      m_nodeprop.setStateAck(END);
      std::cout << "#D exit by fe_exit" <<std::endl;
      send_normal_message("exit by fe_exit");
      exit(1);
    }
    
    int runno;
    if (sscanf(messageline.c_str(), "run %d", &runno) == 1) {
      m_nodeprop.setRunNumber(runno);
    }

  }// while(1)
  
  return 0;
}
