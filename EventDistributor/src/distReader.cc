// -*- C++ -*-
/**
 *  @file   distReader.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: distReader.cc,v 1.10 2012/04/17 06:47:12 igarashi Exp $
 *  $Log: distReader.cc,v $
 *  Revision 1.10  2012/04/17 06:47:12  igarashi
 *  miner refine
 *
 *  Revision 1.9  2012/04/16 11:23:06  igarashi
 *  include Tomo's improvements
 *
 *  Revision 1.8  2010/07/02 12:13:54  igarashi
 *  change buffer modified flag from mutex to semaphore to against Ubuntu bug.
 *
 *  Revision 1.7  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
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
 *  Revision 1.3  2009/04/10 09:25:04  igarashi
 *  add trywait interface in distReader
 *
 *  Revision 1.2  2008/06/27 15:30:18  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.4  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.3  2008/04/08 07:10:49  igarashi
 *  *** empty log message ***
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
 *  Revision 1.4  2007/03/07 15:28:48  igarashi
 *  change MessageClient to GlobalMessageClient and little debug
 *
 *  Revision 1.3  2007/02/16 05:32:23  nakayosi
 *  fixed "event-header error neglect" bug
 *
 *  Revision 1.2  2007/02/14 11:19:06  igarashi
 *  debug semaphore unlock
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */

#include <cstdio>
#include <cerrno>
#include <cstring>
// #include <sys/time.h>
#include <ctime>

#include <iostream>
#include <iomanip>

#include "ControlThread/GlobalInfo.h"
#include "EventDistributor/distReader.h"
#include "Message/GlobalMessageClient.h"


DistReader::DistReader(int buflen, int quelen)
  :m_running(true),
   //   m_mondata_sem(0),
   m_mondata_modified(0),
   //   m_mondata_modified(),
   m_mondata_locker()
{
  m_dist_rb = new RingBuffer(buflen, quelen);
  m_monData = new EventBuffer(max_event_len);
  m_command = STOP;
  m_event_number = 0;
  std::cerr << "distReader created\n";
  //-//m_mondata_modified.lock();
}

DistReader::~DistReader()
{
  delete m_dist_rb;
  delete m_monData;
  std::cerr << "distReader deleted\n";
}

void DistReader::setHost(const char *host, int port)
{
  m_host = host;
  m_port = port;
}

void DistReader::initBuffer()
{
  m_monData->clear();
  //-//m_mondata_modified.trylock();
  while(m_mondata_modified.trywait() == 0);
  releaseMonData();
  m_dist_rb->initBuffer();
}

EventBuffer *DistReader::peekReadEventData()
{
  //std::cerr << "DistReader::peekReadEventData()" << std::endl;
  return m_dist_rb->readBufPeek();

}

EventBuffer *DistReader::peekWriteEventData()
{
  return m_dist_rb->writeBufPeek();

}

int DistReader::releaseReadEventData()
{
  //std::cerr << "DistReader::releaseReadEventData()" << std::endl;
  return m_dist_rb->readBufRelease();
}

int DistReader::releaseWriteEventData()
{
  //std::cerr << "DistReader::releaseReadEventData()" << std::endl;
  return m_dist_rb->writeBufRelease();
}

int DistReader::leftEventData()
{
  return m_dist_rb->left();
}

int DistReader::getRingBufferDepth()
{
  return m_dist_rb->depth();
}

int DistReader::trywaitRingBufferEmpty()
{
  return m_dist_rb->trywaitEmpty();
}


// int DistReader::setMonData(char *dest, char *src, size_t size)
int DistReader::setMonData(char *src, size_t size)
{
  //      danger
  //  	if (0 != m_mondata_sem.trywait())
  //   	{
  //  		memcpy(dest, src, size);
  //  	}
  //  	m_mondata_sem.post();


  //      safe
  //        if (0 == m_mondata_locker.trylock())
  //        {
  //                memcpy(dest, src, size);
  //                m_mondata_locker.unlock();
  //                if (0 != m_mondata_modified.trywait())
  //                        m_mondata_modified.post();
  //        }

  if (0 == m_mondata_locker.trylock())
    {
      //  		m_monData->clear();
      //  		m_monData->resize(size);
      memcpy(m_monData->getBuf(), src, size);
      m_mondata_locker.unlock();
      //-//m_mondata_modified.unlock();
      m_mondata_modified.trywait();
      m_mondata_modified.post();
    }

  return 0;
}

EventBuffer *DistReader::getMonData()
{

  // danger
  //  	m_mondata_sem.wait();

  // safe
  //        m_mondata_modified.wait();

  //-//m_mondata_modified.lock();
  m_mondata_modified.wait();
  m_mondata_locker.lock();
  return m_monData;
}

// void DistReader::setSemWaitMon()
// {
// 	//std::cerr << "== distReader sem wait\n";
// 	m_mondata_sem.wait();
// 	//std::cerr << "== distReader sem wait done\n";
// }

// void DistReader::setSemPostMon()
// {
// 	if (m_mondata_sem.trywait() != 0) {
// 		m_mondata_sem.post();
// 		//std::cerr << "=== distReader sem post done\n";
// 	}
// }

void DistReader::releaseMonData()
{
  m_mondata_locker.unlock();
}

int DistReader::connect(kol::TcpClient& client)
{
  for (int numtry = 0;; ++numtry) {
    try {
      std::cout << "m_host = " << m_host
		<< " m_port = " << std::dec << m_port
		<< std::endl;
      client.Start(m_host, m_port);
      std::cout << "#D client desc. "
		<< client.getDescriptor() << std::endl;
      break;
    } catch(std::exception & e) {
      std::cerr << "#E DistReader::active_loop()"
		<< "TcpClient Fail! / " << e.what()
		<< std::endl;
      if (numtry > 5) {
	std::cerr <<
	  "#E DistReader::active_loop() "
		  << "TcpClinet connection GIVE UP !!"
		  << std::endl;
	return -1;
      } else {
	sleep(1);
      }
    }
  }
  return 0;
}

int DistReader::readHeader(kol::TcpClient& client,
			   unsigned int* header)
{
  while (true) {
    if (checkCommand()!=0) break;
    if (!client.good()) {
      std::cerr << "Network connection broken"
		<< std::endl;
      break;
    }
    try {
      if (!client.read(reinterpret_cast<char *>(header),
		       HEADER_BYTE_SIZE)) {
	std::cerr << "#E DR: ERROR: acvitve_loop"
		  << " header read " << std::endl;
	break;
      }
    } catch(kol::SocketException & e) {
      if (e.reason() == EWOULDBLOCK) {
	std::cerr <<"#DR1 Data socket timeout. retry"
		  << std::endl;
	client.iostate_good();
	continue;
      } else {
	std::cerr
	  << "#W DistReader::active_loop"
	  << " read fail"
	  << std::endl;
	perror("DistReader::active_loop");
      }
      if (checkTcp(client, "#E DistReader::active_loop()")!=0)
	break;
    }
    return 0;
  }
  return 1;
}

int DistReader::updateEventData(kol::TcpClient& client,
				unsigned int* header,
				int trans_byte)
{
  //GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  EventBuffer *event_m = peekWriteEventData();
  // 	event_m->clear();
  // 	event_m->resize(trans_byte);
  char* event_buf      = event_m->getBuf();
  memcpy(event_buf, header, HEADER_BYTE_SIZE);
  int status = -1;

  while (true) {
    if (checkCommand()!=0) break;
    try {
      if (!client.read(event_buf + HEADER_BYTE_SIZE,
		       trans_byte)) {
	std::cerr << "== DR client.read failed"
		  << std::endl;
	break;
      }
      status = 0;
      break;
    } catch (kol::SocketException &e) {
      if (e.reason() == EWOULDBLOCK) {
	std::cerr
	  << "#DR2 Data socket timeout. retry"
	  << std::endl;
	client.iostate_good();
	continue;
      }
      std::cerr << "#E DR data reading err.: "
		<< e.what() << std::endl;
      break;
    }
  }

  if (status==0) {
    setMonData(event_buf, trans_byte + HEADER_BYTE_SIZE);
    if (releaseWriteEventData()!= 0) {
      std::cerr
	<< "ERROR: m_node_rb.writeBufRelease()"
	<< std::endl;
    }
  }
  return status;
}

int DistReader::active_loop()
{
  const int debugprint = 10000;

  //         m_mondata_modified.trylock();
  while (GlobalInfo::getInstance().state!=IDLE)
    usleep(1);

  m_event_number = 0;
  initBuffer();

  GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  std::cerr << "%% DistReader entered active_loop "
	    << m_host << "port:" << m_port << std::endl;

  m_state = RUNNING;
  msock.sendString("ED: READER start");

  try {
    kol::TcpClient client;
    struct timeval timeoutv;
    timeoutv.tv_sec = 10;
    timeoutv.tv_usec = 0;
    client.setsockopt(SOL_SOCKET, SO_RCVTIMEO,
		      &timeoutv, sizeof(timeoutv));

    if (connect(client)!=0) return -1;

    while (true) {
      unsigned header[2];

      memset(header, 0, HEADER_BYTE_SIZE);
      if (readHeader(client, header)!=0) break;

      if (checkHeader(header[0], m_name)<0) continue;

      if (m_event_number % debugprint == 0) {
	std::cout << "== Reader Loop = "
		  << m_event_number
		  << " header[0] = "
		  << std::hex << header[0]
		  << " header[1] = "
		  << std::dec << header[1] << std::endl;
	std::cout.flush();
      }
      //std::cerr << "#D DS wait data left:" << leftEventData()
      // << std::endl;
      size_t trans_byte = (header[1]-2)*sizeof(unsigned int);
      if (checkDataSize(max_event_len,
			trans_byte, m_name)<0)
	continue;

      if (updateEventData(client, header, trans_byte)!=0)
	break;

      ++m_event_number;
    }

    client.close();
  } catch(std::exception & e) {
    std::cerr << "ERROR: eventDistributor: reader:"
	      << e.what() << std::endl;
  }

  std::cerr << "%% DistReader exited active_loop: " << m_host
	    << std::endl;
  m_state = IDLE;
  return 0;
}
