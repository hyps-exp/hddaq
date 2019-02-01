// -*- C++ -*-
/**
 *  @file   EventBuilder.h
 *  @brief
 *  @author Ken Sakashita
 *  @date
 *  @note   divided from commonParam.h
 *
 *  $Id: EventBuilder.h,v 1.2 2012/04/13 12:04:11 igarashi Exp $
 *  $Log: EventBuilder.h,v $
 *  Revision 1.2  2012/04/13 12:04:11  igarashi
 *  include Tomo's improvements
 *    slowReader
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

#ifndef EVENTBUILDER_INCLUDED
#define EVENTBUILDER_INCLUDED

static const int    eventbuilder_port  = 8900;

struct event_header {
  unsigned int magic;
  unsigned int size;
  unsigned int event_number;
  unsigned int run_number;
  unsigned int node_id;
  unsigned int type;
  unsigned int nblock;
  unsigned int reserve;
};

const int g_EVENT_MAGIC = 0x45564e54;
#define EV_MAGIC g_EVENT_MAGIC

const int g_EVENT_TYPE_NORMAL  = 0;
const int g_EVENT_TYPE_NULL    = 1;

#define ET_NOMAL g_EVENT_TYPE_NORMAL;
#define ET_NULL g_EVENT_TYPE_NULL;

#endif
