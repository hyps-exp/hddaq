// -*- C++ -*-
/**
 *  @file   clientInfo.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: clientInfo.h,v 1.1.1.1 2008/05/14 15:05:43 igarashi Exp $
 *  $Log: clientInfo.h,v $
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
#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <string>
#include "kol/koltcp.h"
#include "kol/kolthread.h"

class MonDataSender;
class ClientInfo
{
public:
  ClientInfo(kol::TcpSocket sock);
  virtual ~ClientInfo();
  
  kol::TcpSocket getSock();
  kol::Semaphore * getSem();
  MonDataSender * getSender();
  
private:
  kol::TcpSocket m_sock;
  kol::Semaphore *m_sem;
  MonDataSender *m_mon_sender;
};
#endif
