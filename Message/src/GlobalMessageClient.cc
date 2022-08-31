// -*- C++ -*-
/**
 *  @file   GlobalMessageClient.cc
 *  @brief
 *  @author IGARASHI Youichi
 *  @date
 *
 *  $Id: GlobalMessageClient.cc,v 1.4 2012/04/12 10:09:49 igarashi Exp $
 *  $Log: GlobalMessageClient.cc,v $
 *  Revision 1.4  2012/04/12 10:09:49  igarashi
 *  include Tomo's improvements.
 *  Reference was supported in Message interface.
 *  simplify msgd.
 *
 *  Revision 1.3  2010/06/15 10:30:04  igarashi
 *  minar fix
 *
 *  Revision 1.2  2009/10/19 04:50:19  igarashi
 *  update cmsgd
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.2  2008/04/08 07:10:49  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/01/30 12:33:33  igarashi
 *  Network DAQ Software Prototype 1.4
 *
 *  Revision 1.1.1.1  2007/09/21 08:50:48  igarashi
 *  prototype-1.3
 *
 *  Revision 1.1.1.1  2007/03/28 07:50:17  cvs
 *  prototype-1.2
 *
 *  Revision 1.2  2007/03/07 15:18:59  igarashi
 *  little bug fix
 *
 *  Revision 1.1  2007/03/05 06:10:07  igarashi
 *  a class of singleton client socket
 *
 *  Revision 1.2  2007/02/14 15:37:59  igarashi
 *  minor debug
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#include <iostream>
#include <sstream>
#include <cassert>

#include "kol/kolthread.h"
#include "Message/GlobalMessageClient.h"

GlobalMessageClient* GlobalMessageClient::s_messageclient = 0;
kol::Mutex GlobalMessageClient::s_mutex;

GlobalMessageClient::GlobalMessageClient (const char *host, int port)
  : MessageClient(host, port)
{
  std::cerr << "GlobalMessageClient Created : host = " << host
	    << " port = " << port << std::endl;
}

GlobalMessageClient::GlobalMessageClient (const char *host, int port, int node_id)
  : MessageClient(host, port), m_node_id(node_id)
{
  std::cerr << "GlobalMessageClient Created : host = " << host
	    << " port = " << port << " node_id = " << m_node_id << std::endl;

}

GlobalMessageClient::~GlobalMessageClient ()
{
  delete s_messageclient;
  s_messageclient = 0;
  std::cerr << "GlobalMessageClient Deleted" << std::endl;
}

GlobalMessageClient& GlobalMessageClient::getInstance()
{
  if (s_messageclient == 0) {
    std::cerr << "#E No GlobalMeassageClient Instance"
	      << std::endl;
  }
  return *s_messageclient;
}

GlobalMessageClient& GlobalMessageClient::getInstance(const char *host, int port)
{
  s_mutex.lock();
  if (s_messageclient == 0) {
    try {
      s_messageclient = new GlobalMessageClient(host, port);
    } catch (const std::bad_alloc& e) {
      std::cerr << "#E Message port connection Error !! "
		<< e.what()
		<< std::endl;
    } catch(const std::exception& e) {
      std::cerr << "#E Message port connection Error !! "
		<< e.what()
		<< std::endl;
    } catch (...) {
      std::cerr << "#E Message port connection Error !! "
		<< std::endl;
      throw;
    }
    if (s_messageclient == 0) {
      std::cerr << "#E GlobalMeassageClinet construct Fail!!"
		<< std::endl;
    }
    assert(0);
  }
  s_mutex.unlock();

  return *s_messageclient;
}

GlobalMessageClient& GlobalMessageClient::getInstance(const char *host, int port, int src_id)
{
  s_mutex.lock();
  if (s_messageclient == 0) {
    try {
      s_messageclient = new GlobalMessageClient(host, port, src_id);
    } catch (const std::bad_alloc& e) {
      std::cerr << "#E Message port connection Error !!"
		<< e.what()
		<< std::endl;
    } catch (const std::exception& e) {
      std::cerr << "Message port connection Error !! "
		<< e.what()
		<< std::endl;
    } catch (...) {
      std::cerr << "Message port connection Error !! "
		<< std::endl;
      throw;
    }
    if (s_messageclient == 0) {
      std::cerr << "#E GlobalMeassageClinet construct Fail!!"
		<< std::endl;
    }
  }
  s_mutex.unlock();

  return *s_messageclient;
}


int GlobalMessageClient::sendMessage (const Message& msg)
{
  int retval;

  s_mutex.lock();
  retval = MessageClient::sendMessage(msg);
  s_mutex.unlock();

  return retval;
}

int GlobalMessageClient::sendString(const char *message)
{
  Message mmessage(message);
  mmessage.setHeader(g_MESSAGE_MAGIC);
  mmessage.setSrcId(m_node_id);
  mmessage.setDstId(1);
  mmessage.setSeqNum(0);
  mmessage.setType(0);
  return sendMessage(mmessage);
}

int GlobalMessageClient::sendString(const std::string *message)
{
  return sendString(message->c_str());
}

int GlobalMessageClient::sendString(const std::string &message)
{
  return sendString(message.c_str());
}

int GlobalMessageClient::sendString(const std::ostringstream &message)
{
  return sendString(message.str());
}

int GlobalMessageClient::sendString(int mtype, const char *message)
{
  Message mmessage(message);
  mmessage.setHeader(g_MESSAGE_MAGIC);
  mmessage.setSrcId(m_node_id);
  mmessage.setDstId(1);
  mmessage.setSeqNum(0);
  mmessage.setType(mtype);
  return sendMessage(mmessage);
}

int GlobalMessageClient::sendString(int mtype, const std::string *message)
{
  return sendString(mtype, message->c_str());
}

int GlobalMessageClient::sendString(int mtype, const std::string& message)
{
  return sendString(mtype, message.c_str());
}

int GlobalMessageClient::sendString(int mtype, const std::ostringstream& message)
{
  return sendString(mtype, message.str());
}

