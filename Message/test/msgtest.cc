/*
 *
 */

#include <stdio.h>
#include <string.h>

#include <iostream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

//#include "Message/Message.h"
//#include "Message/MessageClient.h"
#include "Message/GlobalMessageClient.h"

#define  SERVER_NAME "localhost"


//GlobalMessageClient* GlobalMessageClient::s_messageclient = new GlobalMessageClient();
//GlobalMessageClient* GlobalMessageClient::s_messageclient = 0;

/*
int Message::m_header = 0x20060815;
int Message::m_src_id = 500;
*/


int main(int argc, char* argv[])
{
	
	//GlobalMessageClient msock(SERVER_NAME, 8880);
	//MessageClient msock (SERVER_NAME, 8880);
	GlobalMessageClient& msock = GlobalMessageClient::getInstance(SERVER_NAME, 8880);

	//while (1) {
	for (int i = 0 ; i < 10 ; i++) {

		try {
			Message message("");
			message.setMessage("hello");
			msock.sendMessage(message);

			GlobalMessageClient& msock1 = GlobalMessageClient::getInstance(SERVER_NAME, 8880);

			Message message1("world");
			msock1.sendMessage(message1);

			GlobalMessageClient& msock2 = GlobalMessageClient::getInstance();

			message.setMessage(" !!");
			msock2.sendMessage(message);

			msock.sendString("konnnichiwa");

			{
				struct sockaddr_in sname;
				int snamelen;

				snamelen = sizeof(sname);
				msock.getsockname(
					reinterpret_cast<struct sockaddr *>(&sname),
					reinterpret_cast<socklen_t *>(&snamelen));

				std::cout << "#D sockname " <<
					sname.sin_port << " " <<
					std::hex << sname.sin_addr.s_addr << " " <<
					std::hex << ntohl(sname.sin_addr.s_addr) << " " <<
					std::endl;
			}


		} catch (std::exception & e) {
			std::cerr << "#E main :" << e.what() << std::endl;
		}
		usleep(100);
	}

	//delete msock;

	return 0;
}
