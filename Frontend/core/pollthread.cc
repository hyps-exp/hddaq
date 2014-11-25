/*
 *
 *
 */

#include <cstdlib>

#include <iostream>
#include <sstream>
#include <string>

#include "errno.h"

#include "kol/koltcp.h"
#include "Message/GlobalMessageClient.h"

#include "pollthread.h"

//______________________________________________________________________________
PollThread::PollThread(kol::TcpBuffer& sock,
		       const std::string& nickname,
		       node_prop* nodeprop)
  : kol::Thread(),
    m_sock(sock),
    m_nickname(nickname),
    m_event_number(0),
    m_nodeprop(nodeprop)
{
}
  
//______________________________________________________________________________
PollThread::~PollThread()
{
}

//______________________________________________________________________________
int
PollThread::run()
{
  GlobalMessageClient& g_sock = GlobalMessageClient::getInstance();
  while (RUNNING == m_nodeprop->getState())
    {
      char buf[1];
      try 
      	{
      std::cout << "m_sock.read" << std::endl;
      m_sock.read(buf, 1);
      std::cout << "m_sock.read done " <<  std::endl;
      if (m_sock.gcount() == 0)
	    {
	      std::ostringstream msg;
	      //	      std::cout  << m_nickname << ": #W PollThread: got gcount == 0. break\n";
	      msg << "#W " << m_nickname << ": data-path closed by peer"
		  << " (send " << m_event_number
		  << ((m_event_number<2) ? " event" : " events")
		  << "). break";
	      g_sock.sendString(MT_WARNING, msg.str());
	      std::cerr << msg.str() << std::endl;
	      m_nodeprop->setState((int)IDLE);
	      //	      g_sock.close();
	      m_sock.close();
	      //	      ::exit(1);
	      break;
	    }
	}
         catch (const std::exception& e)
	{
	  int errbuf = errno;
	  if(errbuf == EAGAIN){

	  }else{
	    std::cerr << "#E PollThread: " << e.what() 
		      << " errno is " << errbuf << std::endl;
	  }
	  //	  throw;
	}
	 catch (...)
	{
	     std::cerr << "#E PollThread: unknown error occurred. exit"  << std::endl;
	     throw;
	}
    }
  return 0;
}
