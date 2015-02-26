#include <iostream>

#include "watchdogthread.h"
#include "nodeprop.h"


WatchdogThread::WatchdogThread(NodeProp& nodeprop)
  : m_nodeprop(nodeprop)
{
}

WatchdogThread::~WatchdogThread()
{
  std::cout << "WatchdogThread destructed" << std::endl;
}


int WatchdogThread::run()
{
  while (1) {
    m_nodeprop.ackStatus();
    sleep(3);
  }
  
  return 0;
}
