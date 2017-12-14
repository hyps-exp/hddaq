#ifndef NODEPROP_INCLUDED
#define NODEPROP_INCLUDED

#include <string>
#include <vector>

enum State
  {
    INITIAL, IDLE, RUNNING, END
  };

enum DaqMode
  {
    DM_NORMAL, DM_DUMMY
  };

namespace kol
{
  class Mutex;
}

//______________________________________________________________________________
class NodeProp
{
public:
  NodeProp(int nodeid, std::string nickname, int dataport, bool noupdate=false);
  ~NodeProp();

private:
  kol::Mutex*              access_mutex;
  int                      m_argc;
  std::vector<std::string> m_argv;
  State                    m_state;
  DaqMode                  m_daq_mode;
  int                      m_run_number;
  int                      m_node_id;
  int                      m_event_number;
  int                      m_event_size;
  int                      m_data_port;
  std::string              m_nickname;
  bool                     m_update_flag;
  bool                     m_noupdate_flag;

public:
  void setRunNumber(int new_value);
  int getRunNumber();

  void setUpdate(bool new_value);
  bool getUpdate();

  void setState(State new_state);
  void setStateAck(State new_state){ setState(new_state); ackStatus(); }
  State getState();

  void setDaqMode(DaqMode new_mode);
  DaqMode getDaqMode();

  void setEventNumber(int new_value);
  int getEventNumber();

  void setEventSize(int new_value);
  int getEventSize();

  int getDataPort() const { return m_data_port; }
  int getNodeId() const { return m_node_id; }

  void ackStatus();
  void sendEntry();

  void setArgc(int argc){ m_argc = argc; }
  void setArgv(char** argv);

  int  getArgc() const { return m_argc; }
  std::vector<std::string> getArgv() const { return m_argv; }
  std::string getArgv(int index) const { return m_argv[index]; }

  std::string getNickName() const { return m_nickname; }

};

#endif
