// -*- C++ -*-
/*
 *
 */

#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <csignal>
#include <cerrno>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "Message/GlobalMessageClient.h"

#include <daqthread.h>
#include <controlthread.h>
#include <watchdogthread.h>
#include "globalvals.h"



int mknodeid()
{
	char hostname[256];
	if (gethostname(hostname, 255) != 0) {
		perror("mknodeid");
		return 0;
	}
	//std::cout << hostname << std::endl;
	
	struct hostent *he;
	he = gethostbyname(hostname);
	if (he == NULL) {
		perror("mknodeid gethostbyname");
		return 0;
	}
	/*
	std::cout << "h_name " << he->h_name << std::endl;
	std::cout << "h_length " << he->h_length << std::endl;
	int a1 = **(he->h_addr_list) & 0xff;
	int a2 = *(*(he->h_addr_list) + 1) & 0xff;
	int a3 = *(*(he->h_addr_list) + 2) & 0xff;
	int a4 = *(*(he->h_addr_list) + 3) & 0xff;
	std::cout << "h_addr " << a1 << " " << a2 << " " << a3 << " " << a4 << std::endl;
	std::cout << "h_addr2 " << (int)*(he->h_addr_list + 1) << std::endl;
	*/

	//int nodeid = *(*(he->h_addr_list) + he->h_length - 1) & 0xff;
	int nodeid = *(*(he->h_addr_list) + he->h_length - 1) & 0xff;
	nodeid |= (*(*(he->h_addr_list) + he->h_length - 2) & 0xff) << 8;

	return nodeid;
}


void sigpipehandler(int signum)
{
	fprintf(stderr, "Got SIGPIPE! %d\n", signum);
	return;
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

int g_argc;
char ** g_argv;
int main(int argc, char* argv[])
{
	//struct node_prop nodeprop;

	bool isRunNumberLocked = true;
	int nodeid = 0;
	int dataport = 0;
	std::string nickname = "";
	static char cnickname[128];
	std::istringstream iss;

	g_argc = argc;
	g_argv = argv;


	for (int i = 1 ; i < argc ; i++) {
		std::string arg = argv[i];
		iss.str("");
		iss.clear();
		if (arg.substr(0, 9) == "--nodeid=") {
			iss.str(arg.substr(9));
			iss >> nodeid;
		}
		if (arg.substr(0, 11) == "--nickname=") {
			nickname = arg.substr(11);
		}
		if (arg.substr(0, 12) == "--data-port=") {
			iss.str(arg.substr(12));
			iss >> dataport;
		}
		if (arg == "--run-number-nolock") {
		  std::cout << "noloxk" << std::endl;
			isRunNumberLocked = false;
		}

	}

	if (dataport == 0) dataport = DAQ_PORT;
	struct node_prop nodeprop(isRunNumberLocked);
	nodeprop.data_port = dataport;
	if (nodeid == 0) nodeid = mknodeid();
	nodeprop.node_id = nodeid;
	std::cout << "node id  : " << nodeid << std::endl;
	if (nickname.empty()) {
		std::ostringstream oss;
		oss << "FE" ;
		if (nodeid < 10) oss << "0";
		oss << nodeid;
		nickname = oss.str();
	}
	strncpy(cnickname, nickname.c_str(), 128);
	nodeprop.nickname = cnickname;
	std::cout << "nickname : " << nodeprop.nickname << std::endl;


	set_signal();

	GlobalMessageClient & msock = GlobalMessageClient::getInstance("localhost", g_MESSAGE_PORT_UPSTREAM, nodeid);

	if (&msock == 0) {
		std::cerr << "#E " << argv[0]
			<< " can not connect MESSAGE PATH !!"
			<< std::endl;
		return 1;
	}


	DaqThread daqthread(&nodeprop);
	ControlThread controller(&nodeprop);
	WatchdogThread watchdog(&nodeprop);
	nodeprop.daq_thread = &daqthread;
	nodeprop.controller = &controller;
	nodeprop.watchdog = &watchdog;

	//std::cerr << "one" << " ";
	daqthread.start();
	//std::cerr << "two" << " ";
	controller.start();
	//std::cerr << "three" << " ";
	watchdog.start();
	//std::cerr << std::endl;

	daqthread.join();
	watchdog.join();	
	controller.join();

	return 0;
}
