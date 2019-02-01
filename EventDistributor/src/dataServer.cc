// -*- C++ -*-
/**
 *  @file   dataServer.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: dataServer.cc,v 1.4 2012/04/17 06:47:12 igarashi Exp $
 *  $Log: dataServer.cc,v $
 *  Revision 1.4  2012/04/17 06:47:12  igarashi
 *  miner refine
 *
 *  Revision 1.3  2012/04/16 11:23:06  igarashi
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
#include <cstdio>

#include "EventDistributor/dataServer.h"
#include "EventDistributor/dataSender.h"
#include "kol/koltcp.h"

//______________________________________________________________________________
DataServer::DataServer(int port,
                       DataSender& dataSender)
  : m_port(port),
    m_data_sender(dataSender)
{
  std::cerr << "DataServer created\n";
}

//______________________________________________________________________________
DataServer::~DataServer()
{
  std::cerr << "DataServer deleted\n";
}

//______________________________________________________________________________
int DataServer::run()
{
  std::cerr <<"dataServer Port:" << m_port << std::endl;

  while (1) {
    kol::TcpServer server(m_port);

    try {
      while (server.good()) {
	kol::TcpSocket sock = server.accept();
	new SenderThread(sock, m_data_sender);
	std::cerr << "Data Server accepted..."
		  << std::endl;
	std::cerr << "dataServer("
		  << m_data_sender.getName()
		  << ":" << m_port << ")"
		  << " Num of Clients = "
		  << m_data_sender.getSenderNum()
		  << std::endl;
      }
    } catch(kol::SocketException &e) {
      std::cerr << "ERROR: DataServer:" << e.what()
		<< std::endl;
      perror("#W DataServer::run ");
    }
    server.shutdown();
    server.close();
    sleep(1);
  }

  return 0;
}
