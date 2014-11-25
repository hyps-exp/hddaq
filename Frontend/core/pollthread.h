#ifndef POLLER_H
#define POLLER_H

#include "kol/kolthread.h"
#include "nodeprop.h"

//class kol::TcpBuffer;

class PollThread
  : public kol::Thread
{
  
public:
  PollThread(kol::TcpBuffer& sock,
	     const std::string& nickname,
	     node_prop* nodeprop);
  virtual ~PollThread();

  virtual int run();
  void        set_event_number(int event_number);

private:
  kol::TcpBuffer& m_sock;
  std::string     m_nickname;
  int             m_event_number;
  struct node_prop *m_nodeprop;
};

//_____________________________________________________________________________
inline void
PollThread::set_event_number(int event_number)
{
  m_event_number = event_number;
  return;
}
#endif
