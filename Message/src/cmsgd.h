// -*- C++ -*-

#ifndef CMSGD_H
#define CMSGD_H

#include <string>

#include "kol/kolthread.h"

class MessageSocket;
class MessageClient;

//______________________________________________________________________________
struct msgnode_t
{
  std::string hostname;
  int         port;
  std::string status;

  void show_status() const;
};

//______________________________________________________________________________
class CMessageUpstreamThread
  : public kol::Thread
{

public:
  CMessageUpstreamThread(msgnode_t& node,
			 int ref = 0);
  virtual ~CMessageUpstreamThread();


protected:
  virtual int run();
  
private:
  void close();
  bool connect();
  bool forwardMessage();
  void recvMessage(Message& msg);
  void reflectMessage(const Message& msg);
  void sendMessage(Message& msg,
		   const std::string& str="");

private:
  msgnode_t&     m_node;
  MessageClient* m_sock;
  const int      m_ref_flag;

};

//______________________________________________________________________________
class CMessageDownstreamThread
  : public kol::Thread
{

public:
  CMessageDownstreamThread(MessageSocket& daq_ctrl,
			   int ref = 0);
  virtual ~CMessageDownstreamThread();

protected:
  virtual int run();

private:
  void close();
  bool forwardMessage();
  void printStatus() const;
  void recvMessage(Message& msg);
  void reflectMessage(const Message& msg);
  void sendMessage(const Message& msg);

private:
  MessageSocket& m_ctrl;
  const int      m_ref_flag;

};

#endif





