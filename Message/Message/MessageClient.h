// -*- C++ -*-
/**
 *  @file   MessageClient.h
 *  @brief  
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date   
 *
 *  $Id: MessageClient.h,v 1.1.1.1 2008/05/14 15:05:43 igarashi Exp $
 *  $Log: MessageClient.h,v $
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
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#ifndef MESSAGECLIENT_H_INCLUDED
#define MESSAGECLIENT_H_INCLUDED

#include <iostream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"

#include "Message/Message.h"

class MessageClient : public kol::TcpClient {
public:
  MessageClient (const char *host, int port);
  ~MessageClient ();
  int sendMessage (Message msg);
  virtual Message recvMessage ();

};
#endif
