#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "kol/koltcp.h"

#include "nodeprop.h"
#include "pollthread.h"

PollThread::PollThread(NodeProp& nodeprop)
  : m_nodeprop(nodeprop)
{
}
  
PollThread::~PollThread()
{
  std::cout << "PollThread destructed" << std::endl;
}

int PollThread::run()
{
  char buf[1];

  while(1){
    
    if( m_nodeprop.getState() != RUNNING ){
      sleep(3);
      continue;
    }
      
    kol::TcpSocket& dsock = m_nodeprop.getDataSocket();
      
    while (m_nodeprop.getState() == RUNNING) {
      
      try
	{
	  dsock.read(buf, 1);
	  
	  if(m_nodeprop.getState() != RUNNING ) break;
	  
	  if (dsock.gcount() == 0){
	    m_nodeprop.sendErrorMessage("data-path closed by peer");
	    std::cout << "#E data-path closed by peer -> exit" << std::endl;
	    exit(1);
	  }
	}
      catch(...)
	{
	  std::cout << "#D polling data socket" << std::endl;
	}
      
    }

  } // while(1)

  return 0;
}
