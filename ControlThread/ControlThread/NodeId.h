#ifndef NODEID_H_INCLUDED
#define NODEID_H_INCLUDED

#include <string>

enum NodeType {NODETYPE_FE, NODETYPE_EB,NODETYPE_ED, NODETYPE_REC,
	       NODETYPE_MON, NODETYPE_CONT};
static std::string NODENAMEKEY[] = {"FE", "BLD", "DST", "REC", "MON", "CNT"};

class NodeId {
 public:
  NodeId();
  virtual ~NodeId();
  static std::string getNodeId(int, int *);
  //	static std::string getNodeId(enum NodeType, int *);
  //	static int getNodeId();
  //	static std::string getNickname(char *);
  //	static std::string getNickname(std::string);
 protected:
 private:
};


/*
  const int g_NODETYPE_FE = 1;
  const int g_NODETYPE_EB = 2;
  const int g_NODETYPE_ED = 3;
  const int g_NODETYPE_REC = 4;
  const int g_NODETYPE_MON = 5;
  const int g_NODETYPE_CONT = 6;
*/


#endif
