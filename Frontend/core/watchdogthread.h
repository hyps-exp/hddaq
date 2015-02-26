#ifndef WATCHDOGTHREAD_INCLUDED
#define WATCHDOGTHREAD_INCLUDED

#include "kol/kolthread.h"

class NodeProp;
class WatchdogThread : public kol::Thread
{
 public:
  WatchdogThread(NodeProp& nodeprop);
  ~WatchdogThread();
  int run();
  
 private:
  NodeProp& m_nodeprop;
};

#endif
