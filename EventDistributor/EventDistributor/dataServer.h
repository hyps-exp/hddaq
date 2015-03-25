// -*- C++ -*-
/**
 *  @file   dataServer.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: dataServer.h,v 1.2 2012/04/16 11:23:05 igarashi Exp $
 *  $Log: dataServer.h,v $
 *  Revision 1.2  2012/04/16 11:23:05  igarashi
 *  include Tomo's improvements
 *
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
#ifndef DATA_SERVER_H
#define DATA_SERVER_H

#include <string>
#include "kol/kolthread.h"

class DataSender;

class DataServer : public kol::Thread
{
public:
  DataServer(int port,
             DataSender& dataSender);
  virtual ~DataServer();
  
protected: 
  int run();
  
private:
  DataServer(const DataServer&);
  DataServer& operator=(const DataServer&);

  int         m_port;
  DataSender& m_data_sender;
};

#endif

