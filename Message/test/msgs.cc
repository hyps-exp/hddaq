// -*- C++ -*-
/**
 *	@file	 msgs.cc
 *	@brief	
 *	@author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *	@date	 
 *
 *	$Id: msgs.cc,v 1.2 2012/04/17 10:45:35 igarashi Exp $
 *	$Log: msgs.cc,v $
 *	Revision 1.2  2012/04/17 10:45:35  igarashi
 *	miner improvement
 *	
 *	Revision 1.1.1.1	2008/05/14 15:05:43	igarashi
 *	Network DAQ Software Prototype 1.5
 *
 *	Revision 1.1.1.1	2008/01/30 12:33:33	igarashi
 *	Network DAQ Software Prototype 1.4
 *
 *	Revision 1.1.1.1	2007/09/21 08:50:48	igarashi
 *	prototype-1.3
 *
 *	Revision 1.1.1.1	2007/03/28 07:50:17	cvs
 *	prototype-1.2
 *
 *	Revision 1.2	2007/02/14 15:38:31	igarashi
 *	minor debug
 *
 *	Revision 1.1.1.1	2007/01/31 13:37:53	kensh
 *	Initial version.
 *
 *
 *
 *
 */
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "Message/Message.h"
#include "Message/MessageClient.h"

#define	SERVER_NAME "localhost"
#define MSG_UP_PORT 8881


bool isRunning ;

class MessageClientSendThread:public kol::Thread
{
public:
	MessageClientSendThread (const MessageClient & sock):m_msock (sock)
	{
	}
protected:
	int run ()
	{
		std::string str;
		std::cout << "Input a message: " << std::endl;
		while (true) {
			getline (std::cin, str);
			if(str == "\0") continue;
			if(str == "000") break;
			Message *msg = new Message (&str);
			m_msock.sendMessage (*msg);
			delete msg;
		}
		isRunning = false;
		std::cerr << "#D2" << std::endl;
		m_msock.close ();

		return 0;
	}
private:
	MessageClient m_msock;
};

class MessageClientRecvThread:public kol::Thread
{
public:
	MessageClientRecvThread (const MessageClient & sock):m_sock (sock)
	{
	}
protected:
	int run ()
	{
		isRunning = true;
		try {
			while (isRunning) {
				if(!m_sock.good()) {
				std::cerr << "#D MessageClientRecvThread m_sock.good()" << std::endl;
				break;
			}
			Message rmsg = m_sock.recvMessage ();
			if (m_sock.gcount () > 0) {
				std::cout << "% " << rmsg.getMessage () << std::endl;
				std::cout.flush ();
			} else {
				std::cerr << "#D null message" << std::endl;
				//break;
			}
		}
	} catch (kol::SocketException & e) {
		std::cout << "error !! -- " << e.what () << std::endl;
	} catch (...) {
		std::cout << "error" << std::endl;
	}

	std::cerr << "#D 1" << std::endl;
	m_sock.close ();

	return 0;
	}
protected:
	MessageClient m_sock;
};

/*
int Message::m_header = 0x20060815;
int Message::m_src_id = 500;
*/

main (int argc, char **argv)
{
	int port = MSG_UP_PORT;

	std::vector<std::string> argvs(argv, argv + argc);
	//std::vector<std::string>::iterator itarg;
	for (std::vector<std::string>::iterator it = argvs.begin() ;
		it != argvs.end() ; ++it) {
		if (it->find("--port=") != std::string::npos ) {
			std::istringstream ss(it->substr(it->find("--port=") + 7));
			ss >> port;
		}
		if (it->find("--cmsgd") != std::string::npos ) {
			port = 8882;
		}
	}

	std::cout << "Port : " << port << std::endl;



	kol::ThreadController control;

	try {
		MessageClient msock (SERVER_NAME, port);
		// MessageClient msock ("redhat2.net.it-hiroshima.ac.jp", 8881);
		// MessageClient msock ("localhost", 8880);

		control.post (new MessageClientRecvThread (msock));
		control.post (new MessageClientSendThread (msock));
	} catch (std::exception &e) {
		std::cout << "Error : " << e.what() << std::endl;
		std::cout.flush();
	}

	while(1) {
		//std::cout << "num of thread :" << control.numrunning() << std::endl;
		//std::cout.flush();
		if (control.numrunning() != 2) {
			std::cerr << "#E msgs thread are dead !! num. of thread: "
				<< control.numrunning() << std::endl;
		}
		sleep(5);
	}

	return 0;
}
