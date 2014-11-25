// -*- C++ -*-
/**
 *  @file   msg_fmt.h
 *  @brief  
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date   
 *
 *  $Id: msg_fmt.h,v 1.2 2010/06/28 08:31:50 igarashi Exp $
 *  $Log: msg_fmt.h,v $
 *  Revision 1.2  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
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
#ifndef MSG_FMT_H_INCLUDED
#define MSG_FMT_H_INCLUDED

struct msg_fmt
{
    int header;
    int length;
    int src_id;
    int dst_id;
    int seq_num;
    int type;
};
//static msg_fmt msg_fmt_zero;
#endif
