#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "kol/koltcp.h"

#include "nodeprop.h"
#include "MessageHelper.h"
#include "pollthread.h"

PollThread::PollThread(NodeProp& nodeprop, kol::TcpSocket& sock)
  : m_nodeprop(nodeprop),
    m_sock(sock)
{
}

PollThread::~PollThread()
{
  std::cout << "PollThread destructed" << std::endl;
}

int PollThread::run()
{
  char buf[1];
  while (m_nodeprop.getState() == RUNNING) {
    try
      {
	m_sock.read(buf, 1);

	if(m_nodeprop.getState() != RUNNING ) break;

	if (m_sock.gcount() == 0){
	  send_error_message("data-path closed by peer -> end");
	  std::cout << "#E data-path closed by peer -> end" << std::endl;
	  m_nodeprop.setStateAck(END);
	  break;
	}
      }
    catch(...)
      {
	// std::cout << "#D polling data socket" << std::endl;
      }
  }

  return 0;
}
