#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "kol/koltcp.h"

#include "nodeprop.h"
#include "MessageHelper.h"
#include "daqthread.h"
#include "pollthread.h"
#include "userdevice.h"

static const int EV_MAGIC = 0x45564e54;

struct event_header {
  unsigned int magic;
  unsigned int size;
  unsigned int event_number;
  unsigned int run_number;
  unsigned int node_id;
  unsigned int type;
  unsigned int nblock;
  unsigned int unixtime;
};


DaqThread::DaqThread(NodeProp& nodeprop)
  : m_nodeprop(nodeprop)
{
}

DaqThread::~DaqThread()
{
  std::cout << "DaqThread destruct" << std::endl;
}

int DaqThread::run()
{
  m_nodeprop.setStateAck(INITIAL);

  size_t event_header_size    = sizeof(struct event_header);
  const int event_header_len  = event_header_size/sizeof(unsigned int);
  size_t data_size            = get_maxdatasize();
  const int data_len          = data_size/sizeof(unsigned int);
  const int max_buf_len       = event_header_len + data_len;
  
  unsigned int *buf           = new unsigned int[max_buf_len];
  struct event_header *header = reinterpret_cast<struct event_header *>(buf);
  unsigned int *data          = buf + event_header_len;
  
  memset(header, 0, event_header_size);
  header->magic = EV_MAGIC;
  header->node_id = m_nodeprop.getNodeId();
  
 
  //Use open_devide
  open_device(m_nodeprop);
  m_nodeprop.setStateAck(IDLE);
  
  kol::TcpSocket dsock;
  kol::TcpServer server( m_nodeprop.getDataPort() );
  struct timeval tv={3,0};
  server.setsockopt(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  while( m_nodeprop.getState() == IDLE ){ 
    
    try{
      dsock = server.accept();
    }catch(...){
      std::cout << "#D waitting accept" << std::endl;
      continue;
    }
    
    std::cout << "#D server accepted" << std::endl;
    
    PollThread poller(m_nodeprop, dsock);
    poller.start();

    DaqMode daq_mode   = m_nodeprop.getDaqMode();
    header->type       = daq_mode;
    header->run_number = m_nodeprop.getRunNumber();
    m_nodeprop.setEventNumber( 0 );
 
    //User init_devide
    init_device(m_nodeprop);
    m_nodeprop.setStateAck(RUNNING);
      
    while(m_nodeprop.getState() == RUNNING) {
      
      //User wait_devide
      int status = wait_device(m_nodeprop);
      if(status==-1) continue; //TIMEOUT or Fast CLEAR
      
      time_t t = time(0);
      header->unixtime = (unsigned int)t; 
      
      //User read_devide
      int len;
      status = read_device(m_nodeprop, data, len);
      if(status==-1) continue;
      
      len += event_header_len;
      header->size = len;
      m_nodeprop.setEventSize(len);

      header->event_number = m_nodeprop.getEventNumber();
      
      int clen = len * sizeof(unsigned int);
	  
      try{
	dsock.send(buf, clen, MSG_NOSIGNAL);
	dsock.flush();
      }catch(...){
	send_fatal_message("data send failure");
	std::cout << "#E data send failed -> exit " << std::endl;
	exit(1);
      }

      m_nodeprop.setEventNumber( header->event_number+1 );
	    
    } //while( getState() == RUNNING )
    
    //User finalize_devide
    finalize_device(m_nodeprop);

    
    poller.join();
    dsock.close();

  } //while( getState() == IDLE ){ 
  
  //User close_devide
  close_device(m_nodeprop);
  delete buf;
  
  return 0;
}
