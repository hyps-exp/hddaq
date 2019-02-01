// -*- C++ -*-
/**
 *  @file   MessageClient.cc
 *  @brief
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date
 *
 *  $Id: MessageClient.cc,v 1.3 2012/04/12 10:09:49 igarashi Exp $
 *  $Log: MessageClient.cc,v $
 *  Revision 1.3  2012/04/12 10:09:49  igarashi
 *  include Tomo's improvements.
 *  Reference was supported in Message interface.
 *  simplify msgd.
 *
 *  Revision 1.2  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
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
 *  Revision 1.1.1.1  2007/09/21 08:50:48  igarashi
 *  prototype-1.3
 *
 *  Revision 1.1.1.1  2007/03/28 07:50:17  cvs
 *  prototype-1.2
 *
 *  Revision 1.2  2007/02/14 15:37:59  igarashi
 *  minor debug
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */

#include <string.h>

#include <iostream>
#include "Message/MessageClient.h"

MessageClient::MessageClient (const char *host, int port)
  : TcpClient(host,port)
{
  std::cerr << "MessageClient Created\n";
}

MessageClient::~MessageClient ()
{
  std::cerr << "MessageClient Deleted\n";
}

Message MessageClient::recvMessage ()
{

  int msg_size;
  char buf[4096];
  Message msg;

  struct msg_fmt hdr;
  memset(&hdr, 0, sizeof(struct msg_fmt));

  read ((char *) &hdr, sizeof(msg_fmt));
  if (gcount() == 0) {
    throw kol::SocketException("MessageClient::recvMessage, gcount() = 0");
  }

#if DEBUG
  std::cerr << "#D MessageClient::recvMessage recv count: "
	    << gcount() << std::endl;
  std::cerr << "#D hdr header: 0x" << std::hex << hdr.header ;
  std::cerr << " length: " << std::dec << hdr.length ;
  std::cerr << " src_id: " << std::dec << hdr.src_id ;
  std::cerr << " dst_id: " << std::dec << hdr.dst_id ;
  std::cerr << " seq_num: " << std::dec << hdr.seq_num ;
  std::cerr << " type: " << std::dec << hdr.type << std::endl;
#endif

  msg_size = hdr.length - sizeof(msg_fmt);
  read (buf, msg_size);
  buf[msg_size] = '\0';
  msg.setAllHeader (&hdr);
  if (gcount() == msg_size) {
    msg.setMessage (buf);
  } else {
    std::cerr << "#E MessageClient::recvMessage message body was cut"
	      << std::endl;
  }

  return msg;
}

int MessageClient::sendMessage (Message msg)
{
  int value = 0;
  struct msg_fmt hdr;
  std::string str;

  const int bufsize = 4096;
  char buf[bufsize];

  hdr.header = msg.getHeader ();
  hdr.length = msg.getLength ();
  hdr.src_id = msg.getSrcId ();
  hdr.dst_id = msg.getDstId ();
  hdr.seq_num = msg.getSeqNum ();
  hdr.type = msg.getType ();

  str = msg.getMessage ();

  if ((int)str.size() + 1 < bufsize) {
    for (int i = 0; i < (int)str.size (); i++) {
      buf[i] = str[i];
    }
    buf[(int)str.size() + 1] = '\0';
  } else {
    for (int i = 0; i < bufsize ; i++) {
      buf[i] = str[i];
    }
    buf[bufsize - 1] = '\0';
    std::cerr << "#E Message Buffer overflow!! trancate"
	      << "MessageClient::sendMessage()" << std::endl;

  }

  //std::cout << "buf[last] " << buf[(int)str.size()] << " "
  //	<< str.size() << std::endl;

  write (&hdr, sizeof(msg_fmt));
  write (buf, str.size ());
  flush ();

  return value;
}
