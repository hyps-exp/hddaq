// -*- C++ -*-
/**
 *  @file   Message.h
 *  @brief  
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date   
 *
 *  $Id: Message.h,v 1.3 2012/04/12 10:09:49 igarashi Exp $
 *  $Log: Message.h,v $
 *  Revision 1.3  2012/04/12 10:09:49  igarashi
 *  include Tomo's improvements.
 *  Reference was supported in Message interface.
 *  simplify msgd.
 *
 *  Revision 1.2  2009/07/01 07:38:17  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.2  2008/04/08 07:10:49  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/01/30 12:33:33  igarashi
 *  Network DAQ Software Prototype 1.4
 *
 *  Revision 1.2  2007/09/28 13:47:03  igarashi
 *  *** empty log message ***
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
#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <iostream>
#include <string>
#include "Message/msg_fmt.h"

class Message {
public:
  // constructor/destructor
  Message();
  Message(const std::string * message, int dst_id = 0, int type = 0);
  Message(const std::string& message, int dst_id = 0, int type = 0);
  Message(const char *message,    int dst_id = 0, int type = 0);
  ~Message();

  // extructor
  int getHeader ();
  int getLength ();
  int getSrcId ();
  int getDstId ();
  int getSeqNum ();
  int getType ();
  std::string getMessage ();

  // method
  void setHeader (int header);
  void setSrcId (int src_id);
  void setDstId (int dst_id);
  void setSeqNum (int seq_num);
  void setType (int type);
  void setMessage (const std::string * message);
  void setMessage (const std::string& message);
  void setMessage (const char *message);
  void setAllHeader (struct msg_fmt *hdr);

  void showAll ();

  static int getMessageNodeId(int node_type);
  int setMessageNodeId(int node_type);

private:
  msg_fmt m_fmt;
  std::string m_message;
  
};

const int g_MESSAGE_MAGIC = 0x4d455347;
const int g_MESSAGE_PORT_UPSTREAM = 8880;
const int g_MESSAGE_PORT_DOWNSTREAM = 8881;
const int g_MESSAGE_PORT_CONTROL = 8882;

const int g_MESSAGE_TYPE_NORMAL = 0;
const int g_MESSAGE_TYPE_CONTROL = 1;
const int g_MESSAGE_TYPE_STATUS = 2;
const int g_MESSAGE_TYPE_WARNING = 3;
const int g_MESSAGE_TYPE_ERROR = 4;
const int g_MESSAGE_TYPE_FATAL = 5;
const int g_MESSAGE_TYPE_BROKEN = -1;

#define MT_NORMAL g_MESSAGE_TYPE_NORMAL
#define MT_CONTROL g_MESSAGE_TYPE_CONTROL
#define MT_STATUS g_MESSAGE_TYPE_STATUS
#define MT_WARNING g_MESSAGE_TYPE_WARNING
#define MT_ERROR g_MESSAGE_TYPE_ERROR
#define MT_FATAL g_MESSAGE_TYPE_FATAL
#define MT_BROKEN g_MESSAGE_TYPE_BROKEN


const int g_MESSAGE_DIST_BLOADCAST = 0;

#define MD_BLOADCAST g_MESSAGE_DIST_BLOADCAST



const int g_NODE_COLL = 1;
const int g_NODE_EB = 210;
const int g_NODE_ED = 220;
const int g_NODE_REC = 230;
const int g_NODE_MON = 240;
const int g_NODE_CONT = 250;


#endif
