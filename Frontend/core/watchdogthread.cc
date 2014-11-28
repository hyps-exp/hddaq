/*
 *
 *
 */

#include <iostream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "watchdogthread.h"
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
  ControlThread *controller  = reinterpret_cast<ControlThread *>(m_nodeprop->controller);
  while (1) {
    sleep(3);
    controller->ackStatus();
  }
  
  return 0;
}
