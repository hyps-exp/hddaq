// -*- C++ -*-
/**
 *  @file   dataSender.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: dataSender.cc,v 1.10 2012/04/27 10:48:09 igarashi Exp $
 *  $Log: dataSender.cc,v $
 *  Revision 1.10  2012/04/27 10:48:09  igarashi
 *  debug mutex lock in EventDistributor
 *
 *  Revision 1.9  2012/04/17 06:47:12  igarashi
 *  miner refine
 *
 *  Revision 1.8  2012/04/16 11:23:06  igarashi
 *  include Tomo's improvements
 *
 *  Revision 1.6  2009/12/15 09:37:22  igarashi
 *  minar update
 *
 *  Revision 1.5  2009/06/29 02:44:26  igarashi
 *  debug Monitor data transport about buffer locking
 *
 *  Revision 1.4  2009/04/14 06:20:50  igarashi
 *  Debug for Monitor buffer relay sequence
 *  Debug startup threads sequence, wait reader threads until the ring buffer initialized
 *
 *  Revision 1.3  2008/06/27 15:30:18  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/05/18 15:50:36  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.5  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.4  2008/04/08 07:10:49  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.3  2008/02/27 16:24:46  igarashi
 *  Control sequence was introduced in ConsoleThread and Eventbuilder.
 *
 *  Revision 1.2  2008/02/25 13:24:20  igarashi
 *  Threads start timing was changed.
 *  Threads start as series, reader, builder, sender.
 *
 *  Revision 1.1.1.1  2008/01/30 12:33:33  igarashi
 *  Network DAQ Software Prototype 1.4
 *
 *  Revision 1.1.1.1  2007/09/21 08:50:48  igarashi
 *  prototype-1.3
 *
 *  Revision 1.1.1.1  2007/03/28 07:50:17  cvs
 *  prototype-1.2
 *
 *  Revision 1.2  2007/03/07 15:28:48  igarashi
 *  change MessageClient to GlobalMessageClient and little debug
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */

#include <cstdio>
#include <ctime>
#include <cerrno>

#include <exception>

#include "Message/GlobalMessageClient.h"
#include "EventDistributor/dataServer.h"
#include "EventDistributor/dataSender.h"


//______________________________________________________________________________
DataSender::DataSender(DistReader& reader)
  : StatableThread(),
    m_dist_reader(reader),
    m_sender_list(),
    m_list_mutex(),
    m_common_data(),
    m_controller(),
    m_timeoutv(0)
{
  m_command      = STOP;
  m_event_number = 0;
  std::cerr <<  m_name << " created\n";
}

//______________________________________________________________________________
DataSender::~DataSender()
{
  if (m_timeoutv)
    delete m_timeoutv;
  m_timeoutv = 0;

  for (std::list<SenderThread*>::iterator i=m_sender_list.begin();
       i!=m_sender_list.end(); ++i) {
    if (*i) delete *i;
    *i = 0;
  }
}

//______________________________________________________________________________
int DataSender::active_loop()
{
  std::cerr << m_name << " entered active_loop()" << std::endl;
  init();

  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  std::string msg_start="ED: " + m_name + " : start";
  msock.sendString(msg_start);
  while (true) {
    if (checkCommand()!=0) break;
    EventBuffer* srcBuf = read();
    char* srcBegin      = srcBuf->getBuf();
    char* srcEnd
      = srcBegin + srcBuf->getLength() * sizeof(unsigned int);

    m_common_data.clear();
    m_common_data.assign(srcBegin, srcEnd);
    releaseReader();

    if (checkCommand()!=0) break;
    notify();
    ++m_event_number;
  }
  releaseReader();

  std::cerr << m_name << " exited active_loop()" << std::endl;
  return 0;
}

//______________________________________________________________________________
void DataSender::add(SenderThread* t)
{
  m_list_mutex.lock();
  m_sender_list.push_back(t);
  m_list_mutex.unlock();
  m_controller.post(t);
  return;
}

//______________________________________________________________________________
int DataSender::getSenderNum() const
{
  return m_sender_list.size();
}

//______________________________________________________________________________
const std::string DataSender::getName() const
{
  return m_name;
}

//______________________________________________________________________________
const timeval* DataSender::getTimeout() const
{
  return m_timeoutv;
}

//______________________________________________________________________________
void DataSender::init()
{
  while (waitReader()!=0);
  m_event_number = 0;
  m_common_data.clear();

  m_list_mutex.lock();

  //std::cerr << "#d " << m_name << " init lock" << std::endl;

  for (std::list<SenderThread*>::iterator i=m_sender_list.begin();
       i!=m_sender_list.end(); ++i) {
    SenderThread* t = *i;
    if (t) t->clearBusy();
  }
  m_list_mutex.unlock();

  //std::cerr << "#d " << m_name << " init unlock" << std::endl;

  m_state = RUNNING;

  return;
}

//______________________________________________________________________________
void DataSender::notify()
{
  m_list_mutex.lock();

  //std::cerr << "#d " << m_name << " notify lock ";

  for (std::list<SenderThread*>::iterator
	 i = m_sender_list.begin(), iEnd = m_sender_list.end();
       i!=iEnd; ++i) {
    SenderThread* t  = *i;
    if (!t)
      continue;

    while (t->isBusy()) {
      //if (checkCommand()!=0) return;
      if (checkCommand()!=0) break;
      if (t->good()) continue;
      t=0;
      break;
    }

    if (!t) {
      //std::cerr << "#dd SenderThread destructed skip !!" << std::endl;
      continue;
    }
    //if (checkCommand()!=0) return;
    if (checkCommand()!=0) break;
    t->update(m_common_data);

  }
  m_list_mutex.unlock();

  //std::cerr << "#d " << m_name << " notify unlock" << std::endl;

  return;
}

//______________________________________________________________________________
void DataSender::remove(SenderThread* t)
{
  m_list_mutex.lock();
  m_sender_list.remove(t);
  m_list_mutex.unlock();
}

//______________________________________________________________________________
EventBuffer* DataSender::read()
{
  return m_dist_reader.peekReadEventData();
}

//______________________________________________________________________________
void DataSender::releaseReader()
{
  m_dist_reader.releaseReadEventData();
  return;
}

//______________________________________________________________________________
void DataSender::setTimeout(unsigned int tv_sec,
                            unsigned int tv_usec)
{
  m_timeoutv          = new timeval;
  m_timeoutv->tv_sec  = tv_sec;
  m_timeoutv->tv_usec = tv_usec;
  std::cout << m_name << " timeout value = "
	    << tv_sec << " sec " << tv_usec << " usec" << std::endl;
  return;
}

//______________________________________________________________________________
int DataSender::waitReader()
{
  int status = 0;
  for (int i = 0 ;; i++) {
    if (m_dist_reader.getState() == RUNNING) break;
    if (i >= 1000) {
      std::cerr << "#E " << m_name << " waitReader()"
		<< " Timeout !!" << std::endl;
      status = -1;
      break;
    }
    usleep(10000);
  }

  return status;
}

//______________________________________________________________________________
// class SenderThread
//______________________________________________________________________________
SenderThread::SenderThread(const kol::TcpSocket& socket,
                           DataSender& dataSender)
  : kol::Thread(),
    m_socket(socket),
    m_data_sender(dataSender),
    m_timeoutv(0),
    m_buffer(),
    m_locker()
{
  m_data_sender.add(this);
  m_timeoutv = m_data_sender.getTimeout();
}

//______________________________________________________________________________
SenderThread::~SenderThread()
{
  //std::cerr << "#d SenderThread destruct 1" << std::endl;
  m_data_sender.remove(this);
  //std::cerr << "#d SenderThread destruct 2" << std::endl;
}

//______________________________________________________________________________
void SenderThread::clearBusy()
{
  m_locker.clearBusy();
  return;
}

//______________________________________________________________________________
bool SenderThread::good() const
{
  return m_socket.good();
}

//______________________________________________________________________________
bool SenderThread::isBusy()
{
  return m_locker.isBusy();
}

//______________________________________________________________________________
int SenderThread::run()
{
  if (m_timeoutv)
    m_socket.setsockopt(SOL_SOCKET, SO_SNDTIMEO,
			m_timeoutv, sizeof(struct timeval));

  while (true) {
    m_locker.lockData();
    if (!send())
      //	break;
      {
	m_locker.clearBusy();
	break;
      }

    m_locker.clearBusy();
  }

  std::cout << " @@@ #D sender client exit run() " << std::endl;
  return 0;
}

//______________________________________________________________________________
bool SenderThread::send()
{
  const std::string& name = m_data_sender.getName();
  try {
    m_socket.write(&m_buffer[0], m_buffer.size());
    m_socket.flush();
  } catch(const kol::SocketException & e) {
    if (e.reason()==EWOULDBLOCK) {
      std::cerr << "#E " << name
		<< " write socket timeout " << std::endl;
    }
    std::cerr << "#E " << name << e.what() << std::endl;
    return false;
  }
  return true;
}

//______________________________________________________________________________
void SenderThread::update(const std::vector<char>& common_data)
{
  m_buffer.clear();
  m_buffer.assign(common_data.begin(), common_data.end());
  m_locker.unlockData();
  return;
}

