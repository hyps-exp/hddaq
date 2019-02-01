// -*- C++ -*-


#include <cstdio>
#include <cerrno>
#include <ctime>
#include <cstring>


#include <iostream>
#include <string>
#include <sstream>


#include "Message/GlobalMessageClient.h"
#include "ControlThread/GlobalInfo.h"
#include "EventBuilder/slowReaderThread.h"

//______________________________________________________________________________
SlowReaderThread::SlowReaderThread(int buflen,
				   int quelen)
  : ReaderThread(buflen, quelen),
    m_mutex(),
    m_entries(0),
    m_buf(buflen),
    m_null(sizeof(event_header))
{
  std::cout << "#D " << __func__ << "()" << std::endl;
}

//______________________________________________________________________________
SlowReaderThread::~SlowReaderThread()
{
}

//______________________________________________________________________________
int
SlowReaderThread::getRingBufferDepth()
{
  return 1;
}

//______________________________________________________________________________
void
SlowReaderThread::initBuffer()
{
  //   while (m_mutex.trylock()!=0)
  //     {
  //       usleep(1);
  //     }
  //   m_mutex.lock();
  m_entries = 0;
  m_buf.clear();
  m_null.clear();

  m_mutex.unlock();
  return;
}

//______________________________________________________________________________
int
SlowReaderThread::leftEventData()
{
  return m_entries;
}

//______________________________________________________________________________
int
SlowReaderThread::releaseReadFragData()
{

  m_entries = 0;
  return m_mutex.unlock();
}

//______________________________________________________________________________
int
SlowReaderThread::releaseWriteFragData()
{
  m_entries = 1;
  return m_mutex.unlock();
}

//______________________________________________________________________________
EventBuffer*
SlowReaderThread::peekReadFragData()
{
  while (m_mutex.trylock()!=0)
    {
      usleep(100000);
    }

  //   m_mutex.lock();

  if (m_entries==0)
    {
      event_header* ev = reinterpret_cast<event_header*>(m_null.getBuf());
      ev->magic        = g_EVENT_MAGIC;
      ev->size         = sizeof(event_header)/sizeof(unsigned int);
      ev->event_number = m_event_number;
      ev->run_number   = 0;
      ev->node_id      = 0;
      ev->type         = g_EVENT_TYPE_NORMAL;
      ev->nblock       = 0;
      ev->reserve      = 0;
      ++m_event_number;
      return &m_null;
    }
  else
    {
      return &m_buf;
    }
}

//______________________________________________________________________________
EventBuffer*
SlowReaderThread::peekWriteFragData()
{
  while (m_entries>0)
    {
      ::timespec req;
      req.tv_sec  = 0;
      req.tv_nsec = 1;
      ::nanosleep(&req, 0);
    }

  while (m_mutex.trylock()!=0)
    {
      if (checkCommand()!=0)
	return &m_null;
      ::timespec req;
      req.tv_sec  = 0;
      req.tv_nsec = 1;
      ::nanosleep(&req, 0);
    }

  //   m_mutex.lock();
  return &m_buf;
}

//______________________________________________________________________________
int
SlowReaderThread::updateEventData(kol::TcpClient& client,
				  unsigned int* header,
				  int trans_byte,
				  int rest_byte)
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  EventBuffer* event_f = peekWriteFragData();
  if (event_f!=&m_buf)
    return -2;

  char* event_buf      = event_f->getBuf();
  memcpy(event_buf, header, HEADER_BYTE_SIZE);
  int status = -1;

  while (true) {
    if (checkCommand()) break;
    try {
      if (rest_byte>0) {
	if (!client.read(event_buf + HEADER_BYTE_SIZE,
			 trans_byte)) break;
	if (!client.ignore(rest_byte)) break;
      } else if (!client.read(event_buf + HEADER_BYTE_SIZE,
			      trans_byte)) break;
      status = 0;
      break;
    } catch (kol::SocketException& e) {
      if (e.reason() ==  EWOULDBLOCK) {
	// std::cerr << "#D2 Data socket timeout. retry" << std::endl;
	client.iostate_good();
	continue;
      }
      std::ostringstream msg;
      msg << "EB: Reader data reading error: "
	  << e.what()
	  << " host: " << m_host;
      msock.sendString(MT_ERROR, msg);
      std::cerr << msg.str() << std::endl;
      status = -1;
      break;
    }
  }

  if (status==0) {
    event_header* ev = reinterpret_cast<event_header*>(event_buf);
    ev->event_number = m_event_number;
    if (releaseWriteFragData() != 0) {
      std::cerr << "ERROR: m_node_rb.writeBufRelease()" << std::endl;
    }
  }

  return status;
}
