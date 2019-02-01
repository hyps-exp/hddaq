// -*- C++ -*-
/**
 *  @file   msgd.cc
 *  @brief
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date
 *
 *  $Id: msgd.cc,v 1.6 2012/04/12 10:38:14 igarashi Exp $
 *  $Log: msgd.cc,v $
 *  Revision 1.6  2012/04/12 10:38:14  igarashi
 *  suppress debug message
 *
 *  Revision 1.5  2012/04/12 10:09:49  igarashi
 *  include Tomo's improvements.
 *  Reference was supported in Message interface.
 *  simplify msgd.
 *
 *  Revision 1.4  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
 *
 *  Revision 1.3  2010/06/15 10:30:04  igarashi
 *  minar fix
 *
 *  Revision 1.2  2009/12/10 10:37:29  igarashi
 *  Many bugs were fixed
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.1.1.1  2008/01/30 12:33:33  igarashi
 *  Network DAQ Software Prototype 1.4
 *
 *  Revision 1.1.1.1  2007/09/21 08:50:48  igarashi
 *  prototype-1.3
 *
 *  Revision 1.2  2007/04/11 15:56:44  igarashi
 *  *** empty log message ***
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
#include <csignal>
#include <cstdio>
#include <cstring>

#include <string>
#include <sstream>
#include <iostream>
#include <exception>
#include <list>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "Message/Message.h"
#include "Message/MessageSocket.h"

static bool g_verbose = false;

static int UPSIDE_PORT   = 8880;
static int DOWNSIDE_PORT = 8881;

static bool sw_reflection = false;

static kol::Mutex s_upstream_mutex;
kol::Mutex s_list_mutex; // mutex for std::list operation

void sigpipehandler(int signum)
{
  if (g_verbose) {
    fprintf(stderr, "Got SIGPIPE! %d\n", signum);
  }
}

int set_signal()
{
  struct sigaction act;

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = sigpipehandler;
  act.sa_flags |= SA_RESTART;

  if(sigaction(SIGPIPE, &act, NULL) != 0 ) {
    fprintf( stderr, "sigaction(2) error!\n" );
    return -1;
  }

  return 0;
}


class MessageUpstreamThread:public kol::Thread
{
public:
  MessageUpstreamThread (MessageSocket &up_msock,
			 MessageSocket &msock);
  ~MessageUpstreamThread ();
protected:
  int run ();
  MessageSocket &m_msock;
private:
  MessageSocket &m_up_msock;
};

MessageUpstreamThread::MessageUpstreamThread (
					      MessageSocket &up_msock, MessageSocket &msock)
  : m_msock(msock), m_up_msock(up_msock)
{
}

MessageUpstreamThread::~MessageUpstreamThread ()
{
  if (g_verbose) {
    std::cout << "#D MessageUpstreamThread destructed" << std::endl;
  }
}

int MessageUpstreamThread::run ()
{
  try {
    while (1) {
      Message msg;
      try {
	msg = m_msock.recvMessage ();
	if (msg.getHeader() != g_MESSAGE_MAGIC) {
	  std::cerr << "#E MessageUpstreamThread::run "
		    << "BROKEN MESSAGE." << std::endl;
	  break;
	}
      } catch (std::exception &e){
	std::cerr << "#E MessageUpstreamThread"
		  << " read from down error: " << e.what() << std::endl;
	break;
      }

      s_upstream_mutex.lock();
      try {
	m_up_msock.sendMessage(msg);
      } catch (std::exception &e) {
	std::cout
	  << "#E MessageUpstreamThread write error: "
	  << e.what() << std::endl;
      }
      s_upstream_mutex.unlock();
    }

  } catch (std::exception &e) {
    std::cout << "#E MessageUpstreamThread error: "
	      << e.what() << std::endl;
  }

  if (g_verbose) {
    std::cout << "#W MessageUpstreamThread terminate." << std::endl;
  }
  return 0;
}


class MessageDownstreamThread:public kol::Thread
{
public:
  MessageDownstreamThread (MessageSocket &up_msock,
			   std::list<MessageSocket> &msocklist);
  ~MessageDownstreamThread ();
protected:
  int run ();

private:
  MessageSocket &m_up_msock;
  std::list<MessageSocket> &m_msocklist;
};

MessageDownstreamThread::MessageDownstreamThread (
						  MessageSocket &up_msock, std::list<MessageSocket> &msocklist)
  : m_up_msock(up_msock), m_msocklist(msocklist)
{
}

MessageDownstreamThread::~MessageDownstreamThread ()
{
  if (g_verbose) {
    std::cout << "#D MessageDownstreamThread destruct" << std::endl;
  }
}

int MessageDownstreamThread::run ()
{

  kol::TcpServer up_server(DOWNSIDE_PORT);

  /********/
  while (1) {
    s_upstream_mutex.lock();
    m_up_msock = MessageSocket(up_server.accept());
    std::cout << "External Message Port Accepted" << std::endl;
    s_upstream_mutex.unlock();

    try {
      while (1) {
	Message msg;
	try {
	  msg = m_up_msock.recvMessage ();
	  if (msg.getHeader() != g_MESSAGE_MAGIC) {
	    std::cerr << "#E MessageDownstreamThread::run "
		      << "BROKEN MESSAGE." << std::endl;
	    break;
	  }
	} catch (std::exception &e) {
	  std::cout << "MessageDownstreamThread"
		    << " upstream read err. "
		    << e.what() << std::endl;
	  break;
	}

	if (sw_reflection) {
	  s_upstream_mutex.lock();
	  try {
	    m_up_msock.sendMessage(msg);
	  } catch (std::exception &e) {
	    std::cerr << "MessageDownstreamThread "
		      << "upstream write err. "
		      << e.what() << std::endl;
	  }
	  s_upstream_mutex.unlock();
	}

	s_list_mutex.lock();
	if (g_verbose) {
	  std::cerr << "#D num msock "
		    << m_msocklist.size() << std::endl;
	}
	for (std::list<MessageSocket>::iterator
	       it = m_msocklist.begin();
	     it != m_msocklist.end();) {
	  try {
	    int status = it->sendMessage(msg);
	    if (g_verbose) {
	      std::cerr << "#D send status :"
			<< status << std::endl;
	    }
	    ++it;
	  } catch (std::exception &e) {
	    std::cout << "#E MessageDownstreamThread "
		      << "downstream error: "
		      << e.what() << std::endl;
	    it = m_msocklist.erase(it);
	    if (g_verbose) {
	      std::cerr << "#W remove from list"
			<< std::endl;
	    }
	  }
	}
	s_list_mutex.unlock();
      }

      std::cerr << "MessageDownstreamThread upstream closed" << std::endl;
    } catch (std::exception &e) {
      std::cout << "MessageDownstreamThread unknown error : "
		<< e.what() << std::endl;
    }
  }
  /********/

  return 0;
}

int main (int argc, char **argv)
{
  std::string arg;
  for (int i = 1 ; i < argc ; ++i) {
    arg = argv[i];
    if ((arg == "--reflection") || (arg == "-r")) {
      sw_reflection = true;
      std::cout << "Downstream Reflection Mode" << std::endl;
    }
    std::string opt;
    if (arg.find(opt="--up=")==0) {
      std::istringstream iss(arg.substr(opt.size()));
      iss >> UPSIDE_PORT;
      std::cout << "UPSIDE_PORT = " << UPSIDE_PORT << std::endl;
    }
    if (arg.find(opt = "--down=")==0) {
      std::istringstream iss(arg.substr(opt.size()));
      iss >> DOWNSIDE_PORT;
      std::cout << "DOWNSIDE_PORT = " << DOWNSIDE_PORT << std::endl;
    }
  }

  set_signal();

  static MessageSocket up_msock;
  static std::list<MessageSocket> msocklist;

  try {
    kol::TcpServer server (UPSIDE_PORT);
    kol::TcpSocket sock;
    kol::TcpSocket up_sock;
    kol::ThreadController control;

    control.post(new MessageDownstreamThread(up_msock, msocklist));
    while ((sock = server.accept ())) {
      s_list_mutex.lock();
      msocklist.push_back(MessageSocket(sock));
      control.post (
		    new MessageUpstreamThread(
					      up_msock, msocklist.back()));
      s_list_mutex.unlock();

      std::cout << "Internal Message port Accepted"
		<< ":" << msocklist.size() << std::endl;
    }
  } catch (...) {
    std::cerr << "#E error at main" << std::endl;
  }
  if (g_verbose) {
    std::cerr << "#D main end\n" << std::endl;
  }

  return 0;
}
