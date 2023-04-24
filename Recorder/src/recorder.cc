// -*- C++ -*-
/**
 *	@file	 recorder.cc
 *	@brief
 *	@author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *	@date
 *
 *	$Id: recorder.cc,v 1.8 2012/04/27 10:48:09 igarashi Exp $
 *	$Log: recorder.cc,v $
 *	Revision 1.8  2012/04/27 10:48:09  igarashi
 *	debug mutex lock in EventDistributor
 *
 *	Revision 1.7  2012/04/13 07:07:57  igarashi
 *	update Logging function
 *
 *	Revision 1.6  2010/06/28 08:31:50  igarashi
 *	adding C header files to accept the varied distribution compilers
 *
 *	Revision 1.5  2009/12/15 10:55:00  igarashi
 *	add logging function
 *
 *	Revision 1.4  2009/08/03 06:39:04  igarashi
 *	add gzip compress mode
 *
 *	Revision 1.3  2009/07/01 07:37:40  igarashi
 *	add ANYONE/ENTRY scheme, and NodeId class
 *
 *	Revision 1.2	2009/06/29 02:50:20	igarashi
 *	add a option of the setting data directory
 *
 *	Revision 1.1.1.1	2008/05/14 15:05:43	igarashi
 *	Network DAQ Software Prototype 1.5
 *
 *	Revision 1.2	2008/05/13 06:41:57	igarashi
 *	change control sequence
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
 *	Revision 1.2	2007/03/07 15:29:48	igarashi
 *	change MessageClient to GlobalMessageClient
 *
 *	Revision 1.1.1.1	2007/01/31 13:37:53	kensh
 *	Initial version.
 *
 *
 *
 *
 */

#include <string.h>

#include <string>

#include "Recorder/recorderThread.h"
#include "ControlThread/controlThread.h"
#include "ControlThread/GlobalInfo.h"
#include "ControlThread/NodeId.h"
#include "EventDistributor/EventDistributor.h"
#include "EventBuilder/EventBuilder.h"
#include "Message/GlobalMessageClient.h"
#include "Recorder/watchdog.h"

bool g_VERBOSE = false;

int main(int argc, char* argv[])
{
  int port;
  std::string hostname = "localhost";
  char host[128];
  char hosttemp[128];
  char nicknametemp[128];
  //char idchar[128];
  char dir_name_temp[128];
  std::string dir_name;

  //int nodeid = g_NODE_REC;
  int val;

  strcpy(host, "localhost");
  port = eventDist_rec_port;
  //strcpy(nickname, "REC");

  int nodeid;
  std::string nickname = NodeId::getNodeId(NODETYPE_REC, &nodeid);

  int rmode = REC_NORMAL;

  for (int i = 1 ; i < argc ; i++) {
    if (strcmp(argv[i], "--ebport") == 0) {
      port = eventbuilder_port;
      //std::cout << "Port : Event builder ("
      //	<< port << ")" << std::endl;
    } else
      if (sscanf(argv[i], "--dataserver=%s", hosttemp) == 1) {
	strcpy(host, hosttemp);
	//std::cout << "DAQ Server : " << host << " Port ("
	//	<< port << ")" << std::endl;
      } else
	if (sscanf(argv[i], "--port=%d", &val) == 1) {
	  port = val;
	} else
	  if (sscanf(argv[i], "--node-id=%d", &val) == 1) {
	    nodeid = val;
	  } else
#if 0 //2014.11.26 K.Hosomi
	    if (sscanf(argv[i], "--id=%s", idchar) == 1) {
	      for (int i = strlen(idchar) ; i < 5 ; i++) {
		idchar[i] = '\0';
	      }
	      nodeid = *((int *)idchar);
	    } else
#endif //2014.11.26 K.Hosomi
	      if (sscanf(argv[i], "--nickname=%s", nicknametemp) == 1) {
		nickname = nicknametemp;
		std::cout << "NICKNAME : " << nickname << std::endl;
	      } else
		if (sscanf(argv[i], "--dir=%s", dir_name_temp) == 1) {
		  dir_name = dir_name_temp;
		  std::cout << "output file will be in the directory : "
			    << dir_name << std::endl;
		} else
		  if (strcmp(argv[i], "--compress") == 0) {
		    rmode = REC_COMPRESS;
		    std::cout << "Data compress mode" << std::endl;
		  } else {
		    std::cout << "unknown option : " << argv[i] << std::endl;
		  }
  }

  std::cout << "NODE ID : " << nodeid << std::endl;
  std::cout << "Server: " << host
	    << ", port (" << port << ")" << std::endl;


  GlobalMessageClient::getInstance("localhost", g_MESSAGE_PORT_UPSTREAM, nodeid);

  GlobalInfo& gi = GlobalInfo::getInstance();
  gi.nickname = nickname;
  gi.node_id = nodeid;

  //char mstring[128];
  //sprintf(mstring, "ENTRY %s", nickname);
  //msock.sendString(MT_STATUS, mstring);

  try {
    //int slave_num = 1;
    RecorderThread recorder(host, port);
    recorder.setName("$$ recorder");
    recorder.setDirectoryName(dir_name);
    recorder.setRecordMode(rmode);
    ControlThread controller;
    controller.setSlave(&recorder);

    WatchDog watchdog(&controller);

    controller.sendEntry();
    controller.start();
    recorder.start();
    watchdog.start();

    recorder.join();
    controller.join();
  } catch(std::exception &e) {
    std::cerr << "recorder ERROR:" << e.what() << std::endl;
  }

  return 0;
}
