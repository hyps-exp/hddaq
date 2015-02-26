#ifndef CONTROLTHREAD_INCLUDED
#define CONTROLTHREAD_INCLUDED

#include "kol/kolthread.h"

class NodeProp;
class ControlThread : public kol::Thread
{
 public:
  ControlThread(NodeProp& nodeprop);
  ~ControlThread();
  int run();
  
 private:
  NodeProp& m_nodeprop;
};

#endif
