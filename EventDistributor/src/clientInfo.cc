// -*- C++ -*-
/**
 *  @file   clientInfo.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: clientInfo.cc,v 1.1.1.1 2008/05/14 15:05:43 igarashi Exp $
 *  $Log: clientInfo.cc,v $
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
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
#include "EventDistributor/clientInfo.h"


//
ClientInfo::ClientInfo(kol::TcpSocket sock)
  : m_sock(sock)
{
  //
}

//
ClientInfo::~ClientInfo()
{
  //
}

kol::TcpSocket 
ClientInfo::getSock() 
{
  return m_sock;
}

kol::Semaphore* 
ClientInfo::getSem()
{
  return m_sem;
}
  
MonDataSender* 
ClientInfo::getSender()
{
  return m_mon_sender;
}
