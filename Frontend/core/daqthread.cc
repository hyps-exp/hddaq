// -*- C++ -*-
/*
 *
 *
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "Message/GlobalMessageClient.h"

#include "nodeprop.h"
#include "daqthread.h"
#include "controlthread.h"
#include "userdevice.h"
#include "dummydevice.h"
#include "pollthread.h"

DaqThread::DaqThread(struct node_prop *nodeprop)
	: kol::Thread(),
	  m_state(INITIAL),
	  m_daqmode(DM_NORMAL),
	  m_event_number(0),
	  m_event_size(0),
          m_run_number(-1),
	  m_nodeprop(nodeprop)
{
}

DaqThread::~DaqThread()
{
	std::cerr << "DaqThread destruct" << std::endl;
}

int DaqThread::getState()
{
	return m_state;
}
int DaqThread::getDaqMode()
{
	return m_daqmode;
}
int DaqThread::setDaqMode(int mode)
{
	if (m_state == IDLE) {
		m_daqmode = mode;
		return 1;
	} else {
		return 0;
	}
}


int DaqThread::run()
{
  int port = m_nodeprop->data_port;
  const std::string nickname = m_nodeprop->nickname;
  unsigned int *buf = new unsigned int[get_maxdatasize()/sizeof(unsigned int)];
  unsigned int *data;
  struct event_header *header;
  int status;
  
  header = reinterpret_cast<struct event_header *>(buf);
  data = buf + sizeof(struct event_header)/sizeof(unsigned int);
  
  memset(header, 0, sizeof(struct event_header));
  header->magic = EV_MAGIC;
  header->node_id = m_nodeprop->node_id;
  
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  
  ControlThread *controller  = reinterpret_cast<ControlThread *>(m_nodeprop->controller);
  m_state=INITIAL;
  controller->ackStatus();
  
  status = open_device();
  
  while(m_nodeprop->getState() != END){ 
    
    kol::TcpSocket sock;
  
    m_state=IDLE;
    controller->ackStatus();
  
    try {
      kol::TcpServer server(port);
      sock = server.accept();
      server.close();
      
      timeval timeoutv;
      timeoutv.tv_sec  = 6;
      timeoutv.tv_usec = 0;
      sock.setsockopt(SOL_SOCKET, SO_RCVTIMEO,
		      (char*)&timeoutv, sizeof(timeoutv));
      
      PollThread poller(sock, nickname, m_nodeprop);
      poller.start();
      
      //std::cerr << "#D server accepted" << std::endl;
      if (m_daqmode == DM_NORMAL) header->type = ET_NORMAL;
      if (m_daqmode == DM_DUMMY) header->type = ET_DUMMY;
      
      m_run_number = m_nodeprop->getRunNumber();
      header->run_number = m_run_number;
      {
	std::ostringstream msg;
	msg << m_nodeprop->nickname << " got RUN# = "
	    << m_run_number;
	msock.sendString(msg);
      }
      
      if (m_daqmode == DM_NORMAL) status = init_device();
      
      m_state = RUNNING;
      m_event_number = 0;
      controller->ackStatus();
      
      while(m_nodeprop->getState() == RUNNING) {
	int len;
	poller.set_event_number(m_event_number);
	if (m_daqmode == DM_NORMAL) {
	  status = wait_device();
	  if(status<0) continue; //TIMEOUT or Fast CLEAR
	  
	  status = read_device(data, &len, &m_event_number, m_run_number);
	  if (status <= 0) {
	    msock.sendString(MT_ERROR,
			     "Read device err. (Timeout?)");
	    std::cerr << "#E read device error ! "
		      << status << std::endl;
	  }
	} else {
	  status = wait_dummy();
	  status = read_dummy(data, &len, &m_event_number);
	}
	if (status == 1) {
	  len = len + sizeof(struct event_header)/sizeof(unsigned int);
	  header->size = len;
	  header->event_number = m_event_number;
	  int clen = len * sizeof(unsigned int);
	  m_event_size = len;
	  
	  sock.write(buf, clen);
	  sock.flush();
	  
	  ++m_event_number;
	}else if( status==2 ){
	  // do not send data to event builder.
	}else{
	  printf("daqthread: undefined status \n");
	}
      }//while( State() == RUNNING )
      
      poller.join();
      finalize_device();

    } catch (std::exception &e) {
      std::ostringstream msg;
      msg << "#D DaqThread::run main loop: "
	  << e.what() << ", errno = " 
	  << errno;
      std::cerr << msg.str() << std::endl;
      msock.sendString(MT_ERROR, msg);
      sock.close();
    }

  } //while(m_nodeprop->getState() != END){ 
  
  close_device();
  delete buf;
  
  {
    std::ostringstream msg;
    msg << m_nodeprop->nickname << " RUN#"
	<< header->run_number
        << " DaqThread end";
    msock.sendString(msg);
  }

  return 0;
}
