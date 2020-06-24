// -*- C++ -*-

#ifndef SYNC_READER_THREAD_H
#define SYNC_READER_THREAD_H

#include <string>

#include "EventBuilder/readerThread.h"

class SyncReaderThread : public ReaderThread
{

public:
  SyncReaderThread(int buflen,
		   int quelen,
		   const std::string& sync);
  virtual ~SyncReaderThread();

protected:
  virtual int readHeader(kol::TcpClient& client,
			 unsigned int* header);
  //   virtual int updateEventData(kol::TcpClient& client,
  // 			      unsigned int* header,
  // 			      int trans_byte,
  // 			      int rest_byte);

private:
  virtual int sync(kol::TcpBuffer& tcp);

  std::string m_sync;
};

#endif
