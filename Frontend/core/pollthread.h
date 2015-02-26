#ifndef POLLER_H
#define POLLER_H

#include "kol/kolthread.h"

class NodeProp;
class PollThread : public kol::Thread
{
 public:
  PollThread(NodeProp& nodeprop);
  ~PollThread();
  int run();
 
private:
  NodeProp& m_nodeprop;
};

#endif
