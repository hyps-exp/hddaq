#include <iostream>
#include <sstream>

#include "Message/GlobalMessageClient.h"

#include "daqthread.h"
#include "controlthread.h"
#include "watchdogthread.h"
#include "nodeprop.h"

int main(int argc, char* argv[])
{
  int nodeid = 0;
  int dataport = 9000;
  std::string nickname = "nickname";
  bool noupdate_flag = false;

  std::istringstream iss;

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
    if (arg.substr(0, 24) == "--ignore-nodeprop-update") {
      noupdate_flag = true;
    }
  }

  if (nodeid == 0){
    std::cout << "set nodeid using [--nodeid=]" << std::endl;
    return 0;
  }

  if (nickname == "nickname"){
    std::cout << "set nickname using [--nickname=]" << std::endl;
    return 0;
  }

  std::cout << "node id  : " << nodeid << std::endl;
  std::cout << "nickname : " << nickname << std::endl;

  // GlobalMessageClient& msock =
  GlobalMessageClient::getInstance("localhost",
				   g_MESSAGE_PORT_UPSTREAM, nodeid);
  // if (&msock == 0) {
  //   std::cout << "#E " << argv[0]
  // 	      << " can not connect MESSAGE PATH !!"
  // 	      << std::endl;
  //   return 0;
  // }

  NodeProp       nodeprop(nodeid, nickname, dataport, noupdate_flag);
  nodeprop.setArgc(argc);
  nodeprop.setArgv(argv);
  DaqThread      daqthread(nodeprop);
  ControlThread  controller(nodeprop);
  WatchdogThread watchdog(nodeprop);

  controller.start();
  daqthread.start();
  watchdog.start();

  daqthread.join();
  return 0;
}
