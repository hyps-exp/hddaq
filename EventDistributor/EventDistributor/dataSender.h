// -*- C++ -*-
/**
 *  @file   dataSender.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: dataSender.h,v 1.3 2012/04/16 11:23:05 igarashi Exp $
 *  $Log: dataSender.h,v $
 *  Revision 1.3  2012/04/16 11:23:05  igarashi
 *  include Tomo's improvements
 *
 *  Revision 1.2  2009/04/14 06:20:50  igarashi
 *  Debug for Monitor buffer relay sequence
 *  Debug startup threads sequence, wait reader threads until the ring buffer initialized
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.3  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.2  2008/02/25 13:24:20  igarashi
 *  Threads start timing was changed.
 *  Threads start as series, reader, builder, sender.
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
#ifndef DATA_SENDER_H
#define DATA_SENDER_H

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "kol/kolthread.h"
#include "ControlThread/statableThread.h"
#include "EventDistributor/distReader.h"

struct timeval;
class SenderThread;

//______________________________________________________________________________
class DataSender : public StatableThread
{
public:
  DataSender(DistReader& reader);
  virtual ~DataSender();

  void                add(SenderThread* t);
  int                 getSenderNum() const;
  const std::string   getName() const;
  const timeval*      getTimeout() const;
  void                remove(SenderThread* t);
  void                setTimeout(unsigned int tv_sec=0,
				 unsigned int tv_usec=0);

protected:
  virtual int          active_loop();
  virtual void         notify();
  virtual EventBuffer* read();
  virtual void         releaseReader();

private:
  DataSender(const DataSender&);
  DataSender& operator=(const DataSender&);

  void init();
  int  waitReader();

protected:
  DistReader&              m_dist_reader;
  std::list<SenderThread*> m_sender_list;
  kol::Mutex               m_list_mutex;
  std::vector<char>        m_common_data;

private:
  kol::ThreadController  m_controller;
  timeval*               m_timeoutv;

};

//______________________________________________________________________________
template <typename T>
class Locker
{

public:
  Locker() : m_busy(), m_data() { m_data.lock(); };
  ~Locker(){};

  void clearBusy()  { m_busy.unlock(); };
  bool isBusy()     { return (m_busy.trylock()!=0);  };
  void lockData()   { m_data.lock(); };
  void unlockData() { m_data.unlock(); };

private:
  T m_busy;
  T m_data;

};

//______________________________________________________________________________
template <>
class Locker<kol::Semaphore>
{
public:
  Locker() : m_empty(1), m_filled(0) {};
  ~Locker() {};

  void clearBusy()  { while (m_empty.trywait()==0); m_empty.post(); };
  bool isBusy()     { return (m_empty.trywait()!=0); };
  void lockData()   { m_filled.wait(); };
  void unlockData() { while (m_filled.trywait()==0); m_filled.post(); };

private:
  kol::Semaphore m_empty;
  kol::Semaphore m_filled;

};

//______________________________________________________________________________
class SenderThread : public kol::Thread
{

public:
  SenderThread(const kol::TcpSocket& socket,
	       DataSender& dataSender);
  virtual ~SenderThread();

  void clearBusy();
  bool good() const;
  bool isBusy();
  void update(const std::vector<char>& common_buffer);

protected:
  virtual int run();

private:
  SenderThread(const SenderThread&);
  SenderThread& operator=(const SenderThread&);

  bool send();

private:
  kol::TcpSocket    m_socket;
  DataSender&       m_data_sender;
  const timeval*    m_timeoutv;
  std::vector<char> m_buffer;
  Locker<kol::Mutex>       m_locker;
  // 	Locker<kol::Semaphore>   m_locker;

};

#endif
