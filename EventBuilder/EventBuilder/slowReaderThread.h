// -*- C++ -*-

#ifndef SLOW_READER_THREAD_H
#define SLOW_READER_THREAD_H

#include "EventBuilder/readerThread.h"

class SlowReaderThread : public ReaderThread
{

public:
  SlowReaderThread(int buflen,
		   int quelen);
  virtual ~SlowReaderThread();

  virtual int  getRingBufferDepth();
  virtual void initBuffer();
  virtual int  leftEventData();
  virtual int  releaseReadFragData();
  virtual int  releaseWriteFragData();
  virtual EventBuffer* peekReadFragData();
  virtual EventBuffer* peekWriteFragData();

protected:
  virtual int updateEventData(kol::TcpClient& client,
			      unsigned int* header,
			      int trans_byte,
			      int rest_byte);

  kol::Mutex  m_mutex;
  int         m_entries;
  EventBuffer m_buf;
  EventBuffer m_null;
  
};


#endif
