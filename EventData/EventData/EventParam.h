// -*- C++ -*-
/**
 *  @file   EventParam.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: EventParam.h,v 1.2 2009/12/15 09:51:08 igarashi Exp $
 *  $Log: EventParam.h,v $
 *  Revision 1.2  2009/12/15 09:51:08  igarashi
 *  change max buffer size
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
#ifndef EVENT_PARAM_H
#define EVENT_PARAM_H

static const int    max_node_num     = 150;
//static const size_t max_fragment_len = 5 * 1024;  //1024 * 20byte
static const size_t max_fragment_len = 128 * 1024;  //1024 * 20byte
static const int    max_event_len = max_fragment_len * max_node_num;

#endif
