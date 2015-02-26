#ifndef NODEPROP_INCLUDED
#define NODEPROP_INCLUDED

#include <string>

enum State {
  INITIAL, IDLE, RUNNING, END
};

enum DaqMode {
  DM_NORMAL, DM_NULL, DM_DUMMY
};

namespace kol { 
  class Mutex;
  class TcpSocket;
}
class GlobalMessageClient;
class NodeProp
{
public:
  NodeProp(GlobalMessageClient& msock, int nodeid, std::string nickname, int dataport);
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

  kol::TcpSocket& getDataSocket(){ return *m_dsock; }

  int getDataPort(){ return m_data_port; }
  int getNodeId(){ return m_node_id; }

  void ackStatus();
  void sendEntry();
  void sendNormalMessage(const char* message);
  void sendWarningMessage(const char* message);
  void sendErrorMessage(const char* message);

  std::string recvMessage();

private:
  State m_state;
  DaqMode m_daq_mode;
  int m_run_number;
  int m_node_id;
  int m_event_number;
  int m_event_size;
  int m_data_port;
  std::string m_nickname;
  GlobalMessageClient& m_msock;
  kol::TcpSocket* m_dsock;
  kol::Mutex* access_mutex;
  kol::Mutex* recv_mutex;
};

#endif
