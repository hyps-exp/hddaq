#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>

#include "controlthread.h"
#include "nodeprop.h"

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
  m_nodeprop.sendEntry();

  while (1) {
    std::string messageline = m_nodeprop.recvMessage();
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
      ;
    }

    if (messageline == "stop") {
#ifdef END_AT_STOP
      m_nodeprop.setStateAck(END);
      std::cout << "#D end by stop" <<std::endl;
      m_nodeprop.sendNormalMessage("end by stop");
#else
      m_nodeprop.setStateAck(IDLE);
#endif
    }

    if (messageline == "fe_end") {
      m_nodeprop.setStateAck(END);
      std::cout << "#D end by fe_end" <<std::endl;
      m_nodeprop.sendNormalMessage("end by fe_end");
    }

    if (messageline == "fe_exit") {
      std::cout << "#D exit by fe_exit" <<std::endl;
      m_nodeprop.sendErrorMessage("exit by fe_exit");
      exit(1);
    }
    
    int runno;
    if (sscanf(messageline.c_str(), "run %d", &runno) == 1) {
      m_nodeprop.setRunNumber(runno);
    }

  }// while(1)
  
  return 0;
}
