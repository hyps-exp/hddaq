/*
 *
 *
 */

#include <iostream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"
//#include "Message/GlobalMessageClient.h"

#include "watchdogthread.h"
//#include "daqthread.h"
#include "controlthread.h"
#include "nodeprop.h"


WatchdogThread::WatchdogThread(struct node_prop *nodeprop)
  : m_nodeprop(nodeprop)
{
}

WatchdogThread::~WatchdogThread()
{
  std::cerr << "WatchdogThread destructed" << std::endl;
}


int WatchdogThread::run()
{
  //DaqThread *daqthread  = reinterpret_cast<DaqThread *>(m_nodeprop->daq_thread);
  ControlThread *controller  = reinterpret_cast<ControlThread *>(m_nodeprop->controller);
  while (true) {
    sleep(3);
    controller->ackStatus();
    //std::cerr << "WAN: s:" << daqthread->getState()
    //	 << " m:" << daqthread->getDaqMode() << std::endl;
  }
  
  return 0;
}
