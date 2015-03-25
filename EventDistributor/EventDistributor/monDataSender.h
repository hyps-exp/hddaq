// -*- C++ -*-
/**
 *  @file   monDataSender.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: monDataSender.h,v 1.3 2012/04/16 11:23:05 igarashi Exp $
 *  $Log: monDataSender.h,v $
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
#ifndef MON_DATA_SENDER_H
#define MON_DATA_SENDER_H

#include "EventDistributor/dataSender.h"

class MonDataSender : public DataSender
{
public:
  MonDataSender(DistReader& reader);
  virtual ~MonDataSender();
  
protected:
  virtual void         notify();
  virtual EventBuffer* read();
  virtual void         releaseReader();

private:
  MonDataSender(const MonDataSender&);
  MonDataSender& operator=(const MonDataSender&);
};

#endif
