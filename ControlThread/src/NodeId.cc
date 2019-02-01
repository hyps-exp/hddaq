#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "ControlThread/NodeId.h"

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>



//static std::string NODENAMEKEY[] = {"FE", "EB", "ED", "REC", "MON", "CONT"};

NodeId::NodeId()
{
  std::cerr << "NodeId constructed" << std::endl;
}

NodeId::~NodeId()
{
  std::cerr << "NodeId destructed" << std::endl;
}


std::string NodeId::getNodeId(int ntype, int *nodeid)
{

  char hostname[256];
  if (gethostname(hostname, 255) != 0) {
    perror("NodeID::getNodeId");
    return 0;
  }

  struct hostent *he;
  he = gethostbyname(hostname);
  if (he == NULL) {
    perror("NodeId::getNodeId gethostbyname");
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

  pid_t pid = getpid();
  *nodeid = ((ntype & 0xff) << 16)
    | (*(*(he->h_addr_list) + he->h_length - 1) & 0xff) << 8
    | (pid & 0xff);

  std::ostringstream oss;
  oss << NODENAMEKEY[ntype] << "-" << std::hex << *nodeid;
  std::string nickname = oss.str();

  return nickname;
}


#ifdef TESTMAIN
int main(int argc, char* argv[])
{

  int nodeid;

  std::string nick = NodeId::getNodeId(NODETYPE_EB, &nodeid);
  std::cout << "node: " << nick << " id:" << nodeid << std::endl;

  return 0;
}
#endif
