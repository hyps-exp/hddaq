// -*- C++ -*-
/**
 *  @file   msgd.cc
 *  @brief  
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date   
 *
 *  $Id: msgdtest.cc,v 1.2 2010/06/28 08:31:50 igarashi Exp $
 *  $Log: msgdtest.cc,v $
 *  Revision 1.2  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
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
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <exception>
#include <list>
#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "Message/Message.h"
#include "Message/MessageSocket.h"

#include <signal.h>


static const int UPSIDE_PORT = 8880;
static const int DOWNSIDE_PORT = 8881;

static bool sw_reflection = false;

static kol::Mutex s_upstream_mutex;

void sigpipehandler(int signum)
{
	fprintf(stderr, "Got SIGPIPE! %d\n", signum);
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
private:
	MessageSocket *m_up_msock_p;
public:
	MessageUpstreamThread (MessageSocket *up_msock_p,
		const MessageSocket& msock);
	~MessageUpstreamThread ();
protected:
	int run ();
	MessageSocket m_msock;
};


class MessageDownstreamThread:public kol::Thread
{
private:
	MessageSocket *m_up_msock_p;
	std::list<MessageSocket> *m_msocklist_p;
public:
	MessageDownstreamThread (MessageSocket *up_msock_p,
		std::list<MessageSocket> *msocklist_p);
	~MessageDownstreamThread ();
protected:
	int run ();
	MessageSocket m_msock;
};


MessageUpstreamThread::MessageUpstreamThread (
	MessageSocket *up_msock_p, const MessageSocket& msock)
	: m_msock(msock), m_up_msock_p(up_msock_p)
{
}

MessageUpstreamThread::~MessageUpstreamThread ()
{
	std::cout << "#D MessageUpstreamThread destructed" << std::endl;
}

int MessageUpstreamThread::run ()
{
	try {
	while (1) {
		if((!m_msock.getSock()->good()) || (!m_msock.IsGood())) break;
		Message msg;
		try {
			msg = m_msock.recvMessage ();
			//std::cout << "Int.[" << m_num_sock << "]: ";
			//std::cout << msg.getMessage ();
			msg.showAll();
			std::cout.flush ();
		} catch (std::exception &e){
			std::cerr
			<< "#E MessageUpstreamThread read from down error: "
			<< e.what() << std::endl;
			m_msock.getSock()->close ();
			m_msock.setIsGood(false);
			break;
		}

		if (m_up_msock_p->getSock() != NULL) {
			std::cerr << "#D upstream socket check : "
				<< (m_up_msock_p->getSock())->good() << " "
				<< m_up_msock_p->IsGood()
				<< std::endl;
			if ((m_up_msock_p->getSock())->good()
				&& m_up_msock_p->IsGood()) {

				s_upstream_mutex.lock();
				try {
					m_up_msock_p->sendMessage(msg);
				} catch (std::exception &e) {
					std::cout
					<< "#E MessageUpstreamThread write error: "
					<< e.what() << std::endl;
				}
				s_upstream_mutex.unlock();

			} else {
				std::cerr << "#E upstream is not exist!"
					<< std::endl;
			}
		}
	}

	m_msock.getSock()->close ();
	std::cout << "#W MessageUpstreamThread socket closeed: " << std::endl;
	} catch (std::exception &e) {
		std::cout << "#E MessageUpstreamThread error: "
			<< e.what() << std::endl;
	}

	std::cout << "#W MessageUpstreamThread terminate." << std::endl;

	return 0;
}


MessageDownstreamThread::MessageDownstreamThread (
	MessageSocket *up_msock_p, std::list<MessageSocket> *msocklist_p)
	: m_up_msock_p(up_msock_p), m_msocklist_p(msocklist_p)
{
}


MessageDownstreamThread::~MessageDownstreamThread ()
{
	std::cout << "#D MessageDownstreamThread destruct" << std::endl;
}

int MessageDownstreamThread::run ()
{

kol::TcpSocket up_sock;
kol::TcpServer up_server(DOWNSIDE_PORT);

/********/
while (1) {

	up_sock = up_server.accept();
	std::cout << "External Message Port Accepted" << std::endl;
	MessageSocket *msock_tmp2 = new MessageSocket ( up_sock );
	*m_up_msock_p = *msock_tmp2;
	delete msock_tmp2;
	m_up_msock_p->setIsGood(true);

	try {
	while (1) {
		if(!(m_up_msock_p->getSock())->good()) break;
		if(!(m_up_msock_p->IsGood())) break;
		Message msg;
		try {
			msg = m_up_msock_p->recvMessage ();
		} catch (std::exception &e) {
			std::cout << "MessageDownstreamThread upstream read err. "
				<< e.what() << std::endl;
			m_up_msock_p->getSock()->close ();
			m_up_msock_p->setIsGood(false);
			break;
		}
	 	std::cout << "Ext: " << msg.getMessage () << std::endl;
		msg.showAll();
		std::cout.flush ();

		if (sw_reflection) {
			s_upstream_mutex.lock();
			try {
				m_up_msock_p->sendMessage(msg);
			} catch (std::exception &e) {
				std::cerr << "MessageDownstreamThread "
					<< "upstream write err. "
					<< e.what() << std::endl;
				m_up_msock_p->getSock()->close ();
				m_up_msock_p->setIsGood(false);
			}
			s_upstream_mutex.unlock();
		}

		int status;
		std::cerr << "#D num msock " << m_msocklist_p->size() << std::endl;
		std::list<MessageSocket>::iterator it = m_msocklist_p->begin();
		while (it != m_msocklist_p->end()) {
			std::cerr << "#D down strem socket : "
				<< "sock->good() : " << it->getSock()->good() << " "
				<< "msock->IsGood() : " << it->IsGood() << " "
				<< std::endl;

			if ((it->getSock()->good()) && (it->IsGood())) {
				try {
					status = it->sendMessage(msg);
					std::cerr << "#D send status :"
						<< status << std::endl;
				} catch (std::exception &e) {
					std::cout << "#E MessageDownstreamThread "
					<< "downstream error: "
					<< e.what() << std::endl;
					try {
						it->getSock()->close();
						std::cerr << "#W downstream closed"
							<< std::endl;
					} catch (std::exception &e) {
						std::cerr
						<< "#E downstrem close fail :"
						<< e.what() << std::endl;
					}
					it->setIsGood(false);
				}
				++it;
			} else {
				std::cerr << std::endl;
				std::cerr << "#W downstrem connection had closed"
					<< std::endl;
				
				std::list<MessageSocket>::iterator ittemp = it;
				++it;
				m_msocklist_p->erase(ittemp);
				std::cerr << "#W remove from list" << std::endl;
			}
		}
	}

	std::cerr << "MessageDownstreamThread upstream closed" << std::endl;
	} catch (std::exception &e) {
		std::cout << "MessageDownstreamThread unknown error : "
			<< e.what() << std::endl;
			m_up_msock_p->getSock()->close ();
			m_up_msock_p->setIsGood(false);
	}

}
/********/

	return 0;
}



int main (int argc, char **argv)
{

	std::string arg;
	for (int i = 1 ; i < argc ; i++) {
		arg = argv[i];
		if ((arg == "--reflection") || (arg == "-r")) {
			sw_reflection = true;	
			std::cout << "Downstream Reflection Mode" << std::endl;
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

		control.post(new MessageDownstreamThread(&up_msock, &msocklist));

		while ((sock = server.accept ())) {

			std::cout << "Internal Message port Accepted" << std::endl;

			//msocklist.push_back(MessageSocket(sock));
			MessageSocket * msock_tmp = new MessageSocket (sock);
			msocklist.push_back(*msock_tmp);
			delete msock_tmp;
			control.post (
				new MessageUpstreamThread(
				&up_msock, msocklist.back()));
		}

	} catch (...) {
		std::cerr << "#E error at main" << std::endl;
	}

	std::cerr << "#D main end\n" << std::endl;

	return 0;
}
