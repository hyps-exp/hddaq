// -*- C++ -*-

#include <iostream>
#include "ControlThread/GlobalInfo.h"

GlobalInfo::GlobalInfo()
{
  std::cerr << "GlobalInfo constructed" << std::endl;
}

GlobalInfo::~GlobalInfo()
{
  std::cerr << "GlobalInfo destructed" << std::endl;
}

GlobalInfo& GlobalInfo::getInstance()
{
  static GlobalInfo ginfo;
  return ginfo;
}

#ifdef TESTMAIN
int sub1()
{
  GlobalInfo& gi = GlobalInfo::getInstance();
  std::cout << "sub1: " << gi.gstate << std::endl;
  gi.gstate = 1;
  return 0;
}

int sub2()
{
  GlobalInfo& gi = GlobalInfo::getInstance();
  std::cout << "sub2: " << gi.gstate << std::endl;
  gi.gstate = 2;
  return 0;
}

int main(int argc, char* argv[])
{
  GlobalInfo& gi = GlobalInfo::getInstance();

  gi.gstate = 0;
  sub1();
  sub2();
  std::cout << "main: " << gi.gstate << std::endl;

  return 0;
}
#endif
