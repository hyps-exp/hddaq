// -*- C++ -*-
#ifndef GLOBALINFO_H_INCLUDED
#define GLOBALINFO_H_INCLUDED


#include <vector>
#include "ControlThread/statableThread.h"

class GlobalInfo {
public:
  virtual ~GlobalInfo();
  static GlobalInfo& getInstance();

  StatableThread *builder;
  StatableThread *sender;
  StatableThread *monsender;
  StatableThread *reader;
  std::vector <StatableThread *> readers;
  State state;
  int max_event;
  int run_number;
  int node_id;
  double trigger_rate;
  std::string nickname;
protected:
private:
  GlobalInfo();
};
#endif
