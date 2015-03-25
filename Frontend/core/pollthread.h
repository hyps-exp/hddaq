#ifndef POLLER_H
#define POLLER_H

#include "kol/kolthread.h"

namespace kol { class TcpSocket;}
class NodeProp;

class PollThread : public kol::Thread
{
 public:
  PollThread(NodeProp& nodeprop, kol::TcpSocket& sock);
  ~PollThread();
  int run();
 
private:
  NodeProp& m_nodeprop;
  kol::TcpSocket& m_sock;
};

#endif
