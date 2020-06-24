// -*- C++ -*-
/**
 *  @file   senderThread.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: senderThread.cc,v 1.5 2012/04/13 12:04:11 igarashi Exp $
 *  $Log: senderThread.cc,v $
 *  Revision 1.5  2012/04/13 12:04:11  igarashi
 *  include Tomo's improvements
 *    slowReader
 *
 *  Revision 1.4  2010/07/02 12:11:56  igarashi
 *  add to release a send buffer in builder thread when EB get stop command
 *
 *  Revision 1.3  2010/06/30 12:14:09  igarashi
 *  put Time-out in senderThread socket write
 *
 *  Revision 1.2  2009/04/07 09:34:08  igarashi
 *  sender thread re-run automatically without run control
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.3  2008/05/13 06:41:57  igarashi
 *  change control sequence
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
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */

#include <cerrno>
#include <sstream>

#include "EventBuilder/EventBuilder.h"
#include "EventBuilder/senderThread.h"
#include "Message/GlobalMessageClient.h"

SenderThread::SenderThread(int buflen, int quelen)
{
  m_command = STOP;
  m_event_number = 0;
}

SenderThread::~SenderThread()
{
  std::cerr << "Sender Deleted.\n";
}

void SenderThread::setBuilder(BuilderThread * builder)
{
  m_builder = builder;
}

void SenderThread::setSemPost()
{
  m_builder->releaseReadMergData();
}

int SenderThread::waitBuilder()
{
  while(m_builder->getState() != RUNNING) {
    ::usleep(10000);
  }
  return 0;
}

/* */
int SenderThread::run()
{
  active_loop();
  return 0;
}
/* */

int SenderThread::active_loop()
{
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  std::cerr << "== SenderThread: entered active_loop" << std::endl;

  while (true) {
    try {
      //int event_number = 0;
      //m_builder->initSendBuffer(); --> BuilderThread de yaru

      kol::TcpServer server(eventbuilder_port);

      m_state = IDLE;
      {
	std::stringstream msg;
	msg << "== SenderThread: waiting for connection from client port:"
	    << eventbuilder_port;
	// std::cerr << msg << std::endl;
	msock.sendString(MT_NORMAL, msg.str());
      }
      kol::TcpSocket sock = server.accept();
      {
	std::stringstream msg;
	msg << "== sender: accepted";
	// std::cerr << msg << std::endl;
	msock.sendString(MT_NORMAL, msg.str());
      }
      server.shutdown();
      server.close();

      struct timeval timeoutv;
      timeoutv.tv_sec = 10;
      timeoutv.tv_usec = 0;
      sock.setsockopt(SOL_SOCKET, SO_SNDTIMEO,
		      &timeoutv, sizeof(timeoutv));

      waitBuilder();

      m_command = NOCOMM;
      m_event_number = 0;
      while (true) {

	m_state = RUNNING;

	if (checkCommand()) break;

	EventBuffer *event = m_builder->peekReadMergData();

	if (checkHeader(event->getHeader(), m_name)<0) break;

	size_t trans_byte = (event->getLength()) * sizeof(unsigned int);
	if (checkDataSize(max_event_len, trans_byte, m_name)!=0) break;

	bool writeerr;
	bool retry;
	do {
	  writeerr = false;
	  retry    = false;
	  if (checkCommand()) break;
	  try {
	    if (sock.write(event->getBuf(), trans_byte) == 0) {
	      std::stringstream msg;
	      msg << "== SenderThread Error: write error occurred"
		  << std::endl;
	      msock.sendString(MT_ERROR, msg.str());
	      writeerr = true;
	      break;
	    }
	    sock.flush();
	  } catch(kol::SocketException &e) {
	    if (e.reason() == EWOULDBLOCK) {
	      std::stringstream msg;
	      msg << "== SenderThread::actrive_loop() socket time out!!"
		  << std::endl;
	      msock.sendString(MT_ERROR, msg.str());
	      sock.iostate_good();
	      retry = true;
	    } else {
	      std::stringstream msg;
	      msg << "== SenderThread::active_loop() write Err. : "
		  << e.what() << std::endl;
	      msock.sendString(MT_ERROR, msg.str());
	      writeerr = true;
	      break;
	    }
	  }
	} while (retry);

	if (writeerr)
	  break;

	m_builder->releaseReadMergData();
	m_event_number++;
      }

      // std::cerr << "== SenderThread event loop finished" << std::endl;
      ///temp////setSemPost();
      //sock.shutdown();
      sock.close();

    } catch(std::exception & e) {
      std::stringstream msg;
      msg << "== SenderThread Error: " << e.what()
	  << " Loop# " << m_builder->getEventNumber()
	  << std::endl;
      msock.sendString(MT_ERROR, msg.str());
    }

  }

  std::cerr << "senderThread: exited active_loop" << std::endl;

  return 0;
}
