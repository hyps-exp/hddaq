// -*- C++ -*-
/**
 *  @file   MessageSocket.cc
 *  @brief
 *  @author Yasuchi Nagasaka <nagasaka@cc.it-hiroshima.ac.jp>
 *  @date
 *
 *  $Id: MessageSocket.cc,v 1.5 2012/04/12 10:38:14 igarashi Exp $
 *  $Log: MessageSocket.cc,v $
 *  Revision 1.5  2012/04/12 10:38:14  igarashi
 *  suppress debug message
 *
 *  Revision 1.4  2012/04/12 10:09:49  igarashi
 *  include Tomo's improvements.
 *  Reference was supported in Message interface.
 *  simplify msgd.
 *
 *  Revision 1.3  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
 *
 *  Revision 1.2  2009/12/10 10:37:29  igarashi
 *  Many bugs were fixed
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
#include <string.h>
#include "Message/MessageSocket.h"

static bool g_verbose = false;


MessageSocket::MessageSocket (const kol::TcpSocket & sock)
  : m_sock(sock), m_isgood(true)
{
}

MessageSocket::MessageSocket ()
{
}

MessageSocket::~MessageSocket ()
{
  if (g_verbose) {
    std::cout << "#D MessageSocket destructed" << std::endl;
  }
}

int
MessageSocket::sendMessage (Message msg)
{
  struct msg_fmt hdr;
  std::string str;
  char buf[4096];

  hdr.header = msg.getHeader ();
  hdr.length = msg.getLength ();
  hdr.src_id = msg.getSrcId ();
  hdr.dst_id = msg.getDstId ();
  hdr.seq_num = msg.getSeqNum ();
  hdr.type = msg.getType ();

  str = msg.getMessage ();

  for (int i = 0; i < (int)str.size (); i++) {
    buf[i] = str[i];
  }

  bool status;
  status = m_sock.write (&hdr, sizeof(struct msg_fmt));
  //std::cerr << "#D MessageSocket::sendMessage m_sock->write:" << status;
  status = m_sock.write (buf, str.size ());
  //std::cerr << " " << status;
  m_sock.flush ();
  //std::cerr << " " << status << std::endl;

  return static_cast<int>(status);
}

Message MessageSocket::recvMessage ()
{
  const int BUFSIZE = 4096;
  struct msg_fmt hdr;
  int msg_size;
  char buf[BUFSIZE];
  Message msg;

  memset(&hdr, 0, sizeof(struct msg_fmt));
  memset(&buf, 0, sizeof(buf));

  m_sock.read ((char *) &hdr, sizeof(struct msg_fmt));

  if (hdr.header != g_MESSAGE_MAGIC) {
    std::cerr << "#E Illegal Message Header " << hdr.header << std::endl;
    msg.setAllHeader (&hdr);
    msg.setType(MT_BROKEN);
    return msg;
  }

  if ((m_sock.gcount ()) == sizeof(struct msg_fmt)) {
    msg_size = hdr.length - sizeof(struct msg_fmt);
    if (msg_size < BUFSIZE - 1) {
      m_sock.read(buf, msg_size);
      buf[msg_size] = '\0';
    } else {
      m_sock.read(buf, BUFSIZE - 1);
      buf[BUFSIZE-1] = '\0';
      std::cerr << "#E TOO LONG Message " << msg_size
		<< "Message was trancated!"
		<< std::endl;
      for (int i = BUFSIZE - 1 ; i < msg_size ; i++) {
	char tbuf[4];
	m_sock.read(tbuf, 1);
      }
    }
    msg.setAllHeader (&hdr);
    msg.setMessage (buf);
  } else {
    if (g_verbose) {
      std::cerr << "#D m_scok.gcount() " << m_sock.gcount() << std::endl;
    }
    msg.setAllHeader (&hdr);
    msg.setMessage (buf);
    throw kol::SocketException
      ("MessageScoket::recvMessage, gcount() = 0");
  }

  return msg;
}
