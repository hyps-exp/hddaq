#ifndef DAQTHREAD_INCLUDED
#define DAQTHREAD_INCLUDED

#include <kol/kolthread.h>

class NodeProp;
class DaqThread : public kol::Thread
{
public:
  DaqThread(NodeProp& nodeprop);
  ~DaqThread();
  int run();
  
private:
  NodeProp& m_nodeprop;
};

#endif
