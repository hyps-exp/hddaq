// -*- C++ -*-
/**
 *  @file   nodeInfo.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: nodeInfo.h,v 1.2 2012/04/13 12:04:11 igarashi Exp $
 *  $Log: nodeInfo.h,v $
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
#ifndef NODE_INFO_H
#define NODE_INFO_H

#include <string>
#include <iostream>

class NodeInfo
{
public:
  NodeInfo(){;}
  NodeInfo(const std::string& hostname, 
	   int portno,
	   int ringbuf_size, 
	   int ringbuf_len,
	   const std::string& sync="")
    : m_hostname(hostname),
      m_portno(portno), 
      m_ringbuf_size(ringbuf_size), 
      m_ringbuf_len(ringbuf_len),
      m_sync(sync)
  {
  }
  
  std::string getHostName(){ return m_hostname;}
  int         getPortNo(){ return m_portno;}
  int         getRingbufSize(){ return m_ringbuf_size;}
  int         getRingbufLen(){ return m_ringbuf_len;}
  std::string getSyncFlag() { return m_sync;}
  
private:
  std::string m_hostname;
  int         m_portno;
  int         m_ringbuf_size;
  int         m_ringbuf_len;
  std::string m_sync;
};
#endif
