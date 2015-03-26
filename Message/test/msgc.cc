// msgc.cc

#include <iostream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "Message/Message.h"
#include "Message/MessageClient.h"

#define  SERVER_NAME "localhost"


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

	Message *msg = new Message (&str);
	std::cout << "Input a message: " << std::endl;
	while (getline (std::cin, str)) {
	    //Message *msg = new Message (&str);
	    //m_msock.sendMessage (*msg);
	    //delete msg;
	    //Message msg(&str);
	    //m_msock.sendMessage(msg);
	    msg->setMessage(&str);
	    m_msock.sendMessage (*msg);
	}
	delete msg;
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
	try {
	    while (1) {
		Message rmsg = m_sock.recvMessage ();
		std::cout << "% " << rmsg.getMessage () << std::endl;
		std::cout.flush ();
	    }
	}
	catch (kol::SocketException & e) {
	    std::cout << "error !! -- " << e.what () << std::endl;
	}
	catch (...) {
	    std::cout << "error" << std::endl;
	}

	m_sock.close ();

	return 0;
    }
  protected:
    MessageClient m_sock;
};

/*
int
    Message::m_header = 0x20060815;
int
    Message::m_src_id = 500;
*/

int
main (int argc, char **argv)
{
    kol::ThreadController control;

    try {
	char
	    buf[4096];

        MessageClient msock (SERVER_NAME, 8880);

	control.post (new MessageClientRecvThread (msock));
	control.post (new MessageClientSendThread (msock));
    }
    catch (...) {
	std::cout << "Error" << std::endl;
    }

	while(1) {
		//std::cout << control.numrunning() << " ";
		//std::cout.flush();
		if (control.numrunning() != 2) {
			std::cerr << "#E msgc thread dead !! Nthread: "
				<< control.numrunning() << std::endl;
		}
		sleep(5);
	}

    return 0;
}
