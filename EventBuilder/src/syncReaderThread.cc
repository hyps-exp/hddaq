// -*- C++ -*-

#include <iostream>
#include <string>
#include <sstream>
#include <cerrno>

#include "Message/GlobalMessageClient.h"
#include "EventBuilder/syncReaderThread.h"


//______________________________________________________________________________
SyncReaderThread::SyncReaderThread(int buflen,
				   int quelen,
				   const std::string& sync)
  : ReaderThread(buflen, quelen),
    m_sync(sync)
{
}

//______________________________________________________________________________
SyncReaderThread::~SyncReaderThread()
{
}

//______________________________________________________________________________
int
SyncReaderThread::readHeader(kol::TcpClient& client,
			     unsigned int* header)
{
  if (sync(client)!=0) return -1;
  //  	std::cout << "#D read header " << std::endl;
  int ret = ReaderThread::readHeader(client, header);
  return ret;

}

//______________________________________________________________________________
int
SyncReaderThread::sync(kol::TcpBuffer& tcp)
{
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  int status = 0;
  try {
    tcp.write(const_cast<char*>(m_sync.c_str()), m_sync.size());
    tcp.flush();
    status = 0;
  }
  catch (const kol::SocketException& e) {
    std::stringstream msg;
    msg << "EB SyncReader exception : "
	<< m_host << ": " << m_port << ": " << e.what();
    msock.sendString(MT_ERROR, msg.str());
    std::cerr << "#E " << msg.str() << std::endl;
    status = -1;
  }
  return status;
}

