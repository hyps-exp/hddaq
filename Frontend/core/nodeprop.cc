#include <sstream>

#include "kol/kolthread.h"
#include "nodeprop.h"
#include "MessageHelper.h"

NodeProp::NodeProp(int nodeid, std::string nickname, int data_port, bool noupdate)
  : m_argc(0),
    m_state(INITIAL),
    m_daq_mode(DM_NORMAL),
    m_run_number(0),
    m_node_id(nodeid),
    m_event_number(0),
    m_event_size(0),
    m_data_port(data_port),
    m_nickname(nickname),
    m_update_flag(false),
    m_noupdate_flag(noupdate)
{
  access_mutex = new kol::Mutex;
}

NodeProp::~NodeProp()
{
  delete access_mutex;
}

void NodeProp::setRunNumber(int new_value)
{
  access_mutex->lock();
  m_run_number = new_value;
  access_mutex->unlock();
  return;
}
int NodeProp::getRunNumber()
{
  access_mutex->lock();
  int ret = m_run_number;
  access_mutex->unlock();
  return ret;
}

void NodeProp::setUpdate(bool new_value)
{
  access_mutex->lock();
  m_update_flag = new_value;
  access_mutex->unlock();
  return;
}
bool NodeProp::getUpdate()
{
  if(m_noupdate_flag){
    return true;
  }else{
    access_mutex->lock();
    bool ret = m_update_flag;
    access_mutex->unlock();
    return ret;
  }
}

void NodeProp::setState(State new_state)
{
  access_mutex->lock();
  m_state = new_state;
  access_mutex->unlock();
  return;
}
State NodeProp::getState()
{
  access_mutex->lock();
  State ret = m_state;
  access_mutex->unlock();
  return ret;
}

void NodeProp::setDaqMode(DaqMode new_mode)
{
  access_mutex->lock();
  if(m_state == IDLE) m_daq_mode = new_mode;
  access_mutex->unlock();
  return;
}
DaqMode NodeProp::getDaqMode()
{
  access_mutex->lock();
  DaqMode ret = m_daq_mode;
  access_mutex->unlock();
  return ret;
}

void NodeProp::setEventNumber(int new_value)
{
  access_mutex->lock();
  m_event_number = new_value;
  access_mutex->unlock();
  return;
}
int NodeProp::getEventNumber()
{
  access_mutex->lock();
  int ret = m_event_number;
  access_mutex->unlock();
  return ret;
}

void NodeProp::setEventSize(int new_value)
{
  access_mutex->lock();
  m_event_size = new_value;
  access_mutex->unlock();
  return;
}
int NodeProp::getEventSize()
{
  access_mutex->lock();
  int ret = m_event_size;
  access_mutex->unlock();
  return ret;
}

void NodeProp::ackStatus()
{
  std::ostringstream oss;

  access_mutex->lock();

  switch (m_state){
  case INITIAL: oss << "INITIAL"; break;
  case IDLE:    oss << "IDLE";    break;
  case RUNNING: oss << "RUNNING"; break;
  case END:     oss << "END";     break;
  default:      oss << "UNKNOWN"; break;
  }

  oss << " ";

  switch (m_daq_mode){
  case DM_NORMAL: oss << "DM_NORMAL"; break;
  case DM_DUMMY:  oss << "DM_DUMMY";  break;
  default:        oss << "DM_UNKNOWN";   break;
  }

  oss << " run:" << m_run_number;
  oss << " event:" << m_event_number;
  oss << " size:" << m_event_size;

  access_mutex->unlock();

  send_status_message(oss.str());

  return;
}

void NodeProp::sendEntry()
{
  send_status_message("ENTRY " + m_nickname);
  return;
}

void NodeProp::setArgv(char** argv)
{
  m_argv.clear();
  m_argv.resize(m_argc);
  for(int i = 0; i<m_argc; ++i){
    m_argv[i] = argv[i];
    std::cout << m_argv[i] << std::endl;
  }
}
