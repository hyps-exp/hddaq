// -*- C++ -*-
/**
 *  @file   MessageClient.h
 *  @brief  
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date   
 *
 *  $Id: GlobalMessageClient.h,v 1.2 2012/04/12 10:09:49 igarashi Exp $
 *  $Log: GlobalMessageClient.h,v $
 *  Revision 1.2  2012/04/12 10:09:49  igarashi
 *  include Tomo's improvements.
 *  Reference was supported in Message interface.
 *  simplify msgd.
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
 *  Revision 1.1  2007/03/05 06:09:36  igarashi
 *  a class of singleton client socket
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#ifndef GLOBALMESSAGECLIENT_H_INCLUDED
#define GLOBALMESSAGECLIENT_H_INCLUDED

#include "kol/kolsocket.h"
#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "Message/Message.h"
#include "Message/MessageClient.h"

class GlobalMessageClient : public MessageClient {
public:
  ~GlobalMessageClient ();
  static GlobalMessageClient& getInstance();
  static GlobalMessageClient& getInstance(const char *host, int port);
  static GlobalMessageClient& getInstance(const char *host, int port, int src_id);
  int sendMessage (const Message& msg);
  int sendString (const char *message);
  int sendString (const std::string *message);
  int sendString (const std::string &message);
  int sendString (const std::ostringstream &message);
  int sendString (int mtype, const char *message);
  int sendString (int mtype, const std::string *message);
  int sendString (int mtype, const std::string &message);
  int sendString (int mtype, const std::ostringstream &message);

private:
  GlobalMessageClient (const char *host, int port);
  GlobalMessageClient (const char *host, int port, int src_id);
  static GlobalMessageClient* s_messageclient;
  static kol::Mutex s_mutex;
  int m_node_id;
};
#endif
