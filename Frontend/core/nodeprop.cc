#include <sstream>

#include "kol/kolthread.h"
#include "kol/koltcp.h"
#include "Message/GlobalMessageClient.h"
#include "nodeprop.h"

NodeProp::NodeProp(GlobalMessageClient& msock, int nodeid, std::string nickname, int data_port)
  : m_state(INITIAL),
    m_daq_mode(DM_NORMAL),
    m_run_number(0),
    m_node_id(nodeid),
    m_event_number(0),
    m_event_size(0),
    m_data_port(data_port),
    m_nickname(nickname),
    m_msock(msock)
{
  access_mutex = new kol::Mutex;
  recv_mutex   = new kol::Mutex;
  m_dsock      = new kol::TcpSocket;
}

NodeProp::~NodeProp()
{
  delete access_mutex;
  delete recv_mutex;
  delete m_dsock;
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
  case INITIAL:
    oss << "INITIAL";
    break;
  case IDLE:
    oss << "IDLE";
    break;
  case RUNNING:
    oss << "RUNNING";
    break;
  case END:
    oss << "END";
    break;
  default:
    oss << "UNKNOWN";
    break;
  }

  oss << " ";

  switch (m_daq_mode){
  case DM_NORMAL:
    oss << "DM_NORMAL";
    break;
  case DM_NULL:
    oss << "DM_NULL";
    break;
  case DM_DUMMY:
    oss << "DM_DUMMY";
    break;
  default:
    oss << "UNKNOWN";
    break;
  }
  
  oss << " run:" << m_run_number;
  oss << " event:" << m_event_number;
  oss << " size:" << m_event_size;

  access_mutex->unlock();

  m_msock.sendString(MT_STATUS, oss);
  
  return;
}

void NodeProp::sendEntry()
{
  m_msock.sendString(MT_STATUS, "ENTRY " + m_nickname);
  return;
}

void NodeProp::sendNormalMessage(const char* message)
{
  m_msock.sendString(MT_NORMAL, m_nickname + ": " + message);
  return;
}

void NodeProp::sendWarningMessage(const char* message)
{
  m_msock.sendString(MT_WARNING, m_nickname + ": " + message);
  return;
}

void NodeProp::sendErrorMessage(const char* message)
{
  m_msock.sendString(MT_ERROR, m_nickname + ": " + message);
  return;
}

std::string NodeProp::recvMessage()
{
  recv_mutex->lock();
  Message message = m_msock.recvMessage();
  recv_mutex->unlock();
  return message.getMessage();
}
