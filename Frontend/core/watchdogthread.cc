#include <iostream>

#include "watchdogthread.h"
#include "nodeprop.h"
#include "unistd.h"

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
    sleep(3);
    m_nodeprop.ackStatus();
  }
  
  return 0;
}
