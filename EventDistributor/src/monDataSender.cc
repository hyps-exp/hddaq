// -*- C++ -*-
/**
 *  @file   monDataSender.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: monDataSender.cc,v 1.7 2012/04/27 10:48:09 igarashi Exp $
 *  $Log: monDataSender.cc,v $
 *  Revision 1.7  2012/04/27 10:48:09  igarashi
 *  debug mutex lock in EventDistributor
 *
 *  Revision 1.6  2012/04/16 11:23:06  igarashi
 *  include Tomo's improvements
 *
 *  Revision 1.4  2009/12/15 09:37:22  igarashi
 *  minar update
 *
 *  Revision 1.3  2009/06/29 02:44:26  igarashi
 *  debug Monitor data transport about buffer locking
 *
 *  Revision 1.2  2009/04/14 06:20:50  igarashi
 *  Debug for Monitor buffer relay sequence
 *  Debug startup threads sequence, wait reader threads until the ring buffer initialized
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.2  2008/05/13 06:41:57  igarashi
 *  change control sequence
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
 *  Revision 1.3  2007/03/07 15:28:48  igarashi
 *  change MessageClient to GlobalMessageClient and little debug
 *
 *  Revision 1.2  2007/02/14 11:19:36  igarashi
 *  debug sender loop
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#include <iostream>
#include <exception>

#include "EventDistributor/monDataSender.h"

//______________________________________________________________________________
MonDataSender::MonDataSender(DistReader& reader)
  : DataSender(reader)
{
  std::cerr << m_name << " created\n";
}

//______________________________________________________________________________
MonDataSender::~MonDataSender()
{
  std::cerr << m_name << " deleted\n";
}

//______________________________________________________________________________
void MonDataSender::notify()
{
  m_list_mutex.lock();
  for (std::list<SenderThread*>::iterator
	 i = m_sender_list.begin(), iEnd = m_sender_list.end();
       i!=iEnd; ++i) {
    SenderThread* t  = *i;
    if (!t)
      continue;
    if (t->isBusy()) {
      //if (checkCommand()!=0) return;
      if (checkCommand()!=0) break;
      else continue;
    }
    t->update(m_common_data);
  }
  m_list_mutex.unlock();
  return;
}

//______________________________________________________________________________
EventBuffer* MonDataSender::read()
{
  //   EventBuffer* b =m_dist_reader.getMonData();
  return m_dist_reader.getMonData();
  //  	return b;

}

//______________________________________________________________________________
void MonDataSender::releaseReader()
{
  m_dist_reader.releaseMonData();
  return;
}
