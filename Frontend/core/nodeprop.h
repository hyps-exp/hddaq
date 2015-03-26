#ifndef NODEPROP_INCLUDED
#define NODEPROP_INCLUDED

#include <string>
#include <vector>

enum State {
  INITIAL, IDLE, RUNNING, END
};

enum DaqMode {
  DM_NORMAL, DM_DUMMY
};

namespace kol { 
  class Mutex;
}

class NodeProp
{
public:
  NodeProp(int nodeid, std::string nickname, int dataport);
  ~NodeProp();
  
  void setRunNumber(int new_value);
  int getRunNumber();

  void setState(State new_state);
  void setStateAck(State new_state){ setState(new_state); ackStatus(); }
  State getState();
  
  void setDaqMode(DaqMode new_mode);
  DaqMode getDaqMode();
  
  void setEventNumber(int new_value);
  int getEventNumber();

  void setEventSize(int new_value);
  int getEventSize();

  int getDataPort(){ return m_data_port; }
  int getNodeId(){ return m_node_id; }

  void ackStatus();
  void sendEntry();

  void setArgc(int argc){m_argc = argc;}
  void setArgv(char** argv);

  int  getArgc(){return m_argc;}
  std::string getArgv(int index){return m_argv[index];};

private:
  State m_state;
  DaqMode m_daq_mode;
  int m_run_number;
  int m_node_id;
  int m_event_number;
  int m_event_size;
  int m_data_port;
  std::string m_nickname;
  kol::Mutex* access_mutex;
  int m_argc;
  std::vector<std::string> m_argv;
  
};

#endif
