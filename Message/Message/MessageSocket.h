// -*- C++ -*-
/**
 *	@file	 MessageSocket.h
 *	@brief
 *	@author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *	@date
 *
 *	$Id: MessageSocket.h,v 1.2 2009/12/10 10:37:29 igarashi Exp $
 *	$Log: MessageSocket.h,v $
 *	Revision 1.2  2009/12/10 10:37:29  igarashi
 *	Many bugs were fixed
 *
 *	Revision 1.1.1.1	2008/05/14 15:05:43	igarashi
 *	Network DAQ Software Prototype 1.5
 *
 *	Revision 1.1.1.1	2008/01/30 12:33:33	igarashi
 *	Network DAQ Software Prototype 1.4
 *
 *	Revision 1.1.1.1	2007/09/21 08:50:48	igarashi
 *	prototype-1.3
 *
 *	Revision 1.1.1.1	2007/03/28 07:50:17	cvs
 *	prototype-1.2
 *
 *	Revision 1.1.1.1	2007/01/31 13:37:53	kensh
 *	Initial version.
 *
 *
 *
 *
 */
#ifndef MESSAGESOCKET_H_INCLUDED
#define MESSAGESOCKET_H_INCLUDED

#include <iostream>

#include "kol/kolsocket.h"
#include "kol/koltcp.h"

#include "Message/Message.h"

class MessageSocket {
public:
  MessageSocket (const kol::TcpSocket & socket);
  MessageSocket ();
  ~MessageSocket ();

  int close ()
  {
    std::cerr << "#D MessageSocket::close" << std::endl;

    m_sock.shutdown ();
    m_sock.close ();
    return 0;
  }
  int sendMessage (Message msg);
  Message recvMessage ();
  kol::TcpSocket * getSock()
  {
    //return m_sock;
    return &m_sock;
  }

  //int gcount()
  //{
  //	return m_gcount;
  //}

  bool IsGood()
  {
    return m_isgood;
  }
  bool setIsGood(bool status)
  {
    return m_isgood = status;
  }

private:
  // kol::TcpSocket * m_sock;
  kol::TcpSocket m_sock;
  // int m_gcount;
  bool m_isgood;

};

#endif
