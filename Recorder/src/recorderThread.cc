// -*- C++ -*-
/**
 *  @file   recorderThread.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: recorderThread.cc,v 1.8 2012/04/27 11:15:53 igarashi Exp $
 *  $Log: recorderThread.cc,v $
 *  Revision 1.8  2012/04/27 11:15:53  igarashi
 *  chame the timeout value
 *
 *  Revision 1.7  2012/04/27 10:48:09  igarashi
 *  debug mutex lock in EventDistributor
 *
 *  Revision 1.6  2012/04/13 07:07:57  igarashi
 *  update Logging function
 *
 *  Revision 1.5  2009/12/15 10:55:00  igarashi
 *  add logging function
 *
 *  Revision 1.4  2009/08/03 06:39:04  igarashi
 *  add gzip compress mode
 *
 *  Revision 1.3  2009/06/29 02:50:20  igarashi
 *  add a option of the setting data directory
 *
 *  Revision 1.2  2009/04/14 06:26:37  igarashi
 *  debug about the recovery of the timeout
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
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
 *  Revision 1.2  2007/03/07 15:29:48  igarashi
 *  change MessageClient to GlobalMessageClient
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */
#include <iomanip>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <ctime>
#include <cerrno>
#include <ctime>

#include "Recorder/recorderThread.h"
#include "Recorder/OGZFileStream.hh"
#include "EventBuilder/EventBuilder.h"
#include "Message/Message.h"
#include "Message/GlobalMessageClient.h"
#include "Recorder/recorderBookmarker.hh"
#include "Recorder/recorderLogger.hh"

using namespace  hddaq::unpacker;


RecorderThread::RecorderThread()
{
  std::cerr << "Recorder Created" << std::endl;
}

RecorderThread::RecorderThread(std::string hostname, int port)
  : m_port(port), m_hostname(hostname)
{
  std::cerr << "Recorder Created" << std::endl;
}

RecorderThread::~RecorderThread()
{
  std::cerr << "Recorder Deleted" << std::endl;
}

void RecorderThread::setDirectoryName(const std::string& dir_name)
{
  if (dir_name.empty()) return;
  if (dir_name[dir_name.size()-1]=='/') m_dir_name = dir_name;
  else m_dir_name = dir_name + "/";
}


void RecorderThread::setHostname(const std::string& hostname)
{
  m_hostname = hostname;
}

void RecorderThread::setPortNo(int port)
{
  m_port = port;
}

void RecorderThread::setRecordMode(int rec_mode)
{
  m_rec_mode = rec_mode;
}

int RecorderThread::getRecordMode()
{
  return m_rec_mode;
}

int RecorderThread::active_loop()
{

  //std::cerr << "#D one" << std::endl;
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();
  //std::cerr << "#D two" << std::endl;

  std::cerr << "recorderThread: entered active_loop with port="
	    << m_port << std::endl;
  int run_number = getRunNumber();
  std::vector<unsigned int> data;
  try {

    std::cerr << "RUN NO: " << run_number << " ";
    std::string fname;

    fname = makeFileName(run_number);
    if (m_rec_mode == REC_COMPRESS) {
      fname += ".gz";
    }
    std::cerr << "filename: " << fname << std::endl;

    std::string messageStr = "Rec: record start " + fname;
    msock.sendString(messageStr);

    kol::TcpClient client(m_hostname.c_str(), m_port);
    unsigned int header[2];
    // 		static unsigned int data[max_event_len];
    data.clear();
    //unsigned int *data = new unsigned int[max_event_len];

    struct timeval timeoutv;
    //socklen_t optlen;
    //client.getsockopt(SOL_SOCKET, SO_RCVTIMEO, &timeoutv, &optlen);
    //std::cout << "#D timeout " << timeoutv.tv_sec << " "
    //		<< timeoutv.tv_usec << " " << optlen << std::endl;
    //timeoutv.tv_sec = 10;
    timeoutv.tv_sec = 5;
    timeoutv.tv_usec = 0;
    client.setsockopt(SOL_SOCKET, SO_RCVTIMEO, &timeoutv,
		      sizeof(timeoutv));

    m_state = RUNNING;
    m_event_number = 0;

    std::ostream *ofsp = 0;
    if (m_rec_mode == REC_COMPRESS) {
      ofsp = new OGZFileStream(fname.c_str(), std::ios::out | std::ios::binary);
    } else {
      ofsp = new std::ofstream(fname.c_str(), std::ios::out | std::ios::binary);
    }
    if( !ofsp || !(*ofsp) || ofsp->fail() ){
      std::ostringstream msgss;
      msgss << "Rec: unable to create or write file";
      std::cerr << msgss.str() << std::endl;
      msock.sendString(MT_ERROR, msgss.str());
      return -1;
    }
    Logger logger(m_event_number,
		  run_number, m_dir_name+"recorder.log");
    Bookmarker bookmarker(run_number, m_dir_name);

    while (true) {
      while (true) {
	if (checkCommand()!=0) break;
	try {
	  client.read(reinterpret_cast<char *>(header), sizeof(header));
	  break;
	} catch(kol::SocketException & e) {
	  if (e.reason() == EWOULDBLOCK) {
	    //std::cerr << "#D Data socket timeout. retry "
	    //	<< std::endl;
	    client.iostate_good();
	  } else {
	    std::ostringstream msgss;
	    msgss << "#W Data socket err. "
		  << e.what() << " "
		  << std::dec << e.reason();
	    msock.sendString(MT_WARNING, msgss);
	    std::cerr << msgss.str() << std::endl;
	    throw e;
	  }
	}
      }
      if (checkCommand()!=0) break;

      if (checkTcp(client, m_name)!=0) break;

      checkHeader(header[0], m_name);

      size_t recv_byte = (header[1] - 2)*sizeof(unsigned int);
      if (checkDataSize(max_event_len*sizeof(unsigned int),
			recv_byte, m_name)!=0)
	continue;
      data.resize(recv_byte/sizeof(unsigned int));
      try {
	if (!client.read(reinterpret_cast<char *>(&data[0]), recv_byte))
	  break;
      } catch(kol::SocketException & e) {
	std::ostringstream msgss;
	msgss << "#E Rec: Data socket data body read err."
	      << e.what() << " " << e.reason();
	msock.sendString(MT_ERROR, msgss);
	std::cerr << msgss.str() << std::endl;
	break;
      }
      unsigned int received_size = client.gcount();
      if (received_size != recv_byte) {
	std::ostringstream msgss;
	msgss << "#E Rec: read data misssmatch : "
	      << std::dec << client.gcount()
	      << "/" << recv_byte;
	msock.sendString(MT_ERROR, msgss);
	std::cerr << msgss.str() << std::endl;
	break;
      }

      /*
	if (checkCopperData((data + 1), ((receved_size/4) - 1))) {
	std::cerr << "#Error: broken data structure!!" << std::endl;
	break;
	}
      */
      ofsp->write(reinterpret_cast<char *>(header), sizeof(header));
      ofsp->write(reinterpret_cast<char *>(&data[0]), recv_byte);
      logger += (sizeof(header) + recv_byte);
      bookmarker += (sizeof(header) + recv_byte);
      //std::cerr << '.';
      m_event_number++;
    }
    delete ofsp;
    ofsp = 0;
    ::chmod(fname.c_str(), S_IRUSR | S_IRGRP | S_IROTH);
    std::cerr << "file   closed" << std::endl;
    client.close();
    std::cerr << "client closed" << std::endl;

    messageStr = "Rec: " + fname + " closed";
    msock.sendString(messageStr);

  } catch(std::exception & e) {
    std::cerr << "@@ Recorder Error: ";
    std::cerr << e.what() << std::endl;
    std::string messageStr = "#ERR. Rec: " + std::string(e.what());
    msock.sendString(MT_ERROR, messageStr);
  }

  std::cerr << "recorderThread: exited active_loop" << std::endl;

  m_state = IDLE;
  return 0;

}

std::string RecorderThread::makeFileName(int run_no)
{
  int fname_size = 8;
  std::string fname = "run00000";
  std::string rnum;

  std::ostringstream str_stream;
  str_stream << run_no;
  rnum = str_stream.str();
  int no_size = rnum.size();

  fname.replace(fname_size - no_size, no_size, rnum);
  fname += ".dat";
  fname = m_dir_name + fname;
  return fname;
}

int RecorderThread::checkCopperData(unsigned int *data, int len)
{
  int errorflag = 0;
  static int last_runno = 0;

  if (data[0] != 0x45564e54) {
    std::cerr << "#E DAQ Server header Error !! "
	      << std::hex << data[0] << std::endl;
    errorflag |= 0x1;
  }

  if (data[2] == 1)
    last_runno = 0;

  if (data[2] - last_runno != 1) {
    std::cerr << "#E Event Slip !! "
	      << std::dec << data[2] << "/" << last_runno << std::endl;
    errorflag |= 0x2;
  }
  last_runno = data[2];

  if (data[8] != 0xfffffafa) {
    std::cerr << "#E COPPER header Error !! "
	      << std::hex << data[8] << std::endl;
    errorflag |= 0x4;
  }
  if (data[len - 1] != 0xfffff5f5) {
    std::cerr << "#E COPPER trailer Error !! "
	      << std::hex << data[len - 1] << std::endl;
    errorflag |= 0x8;
  }
  if ((data[len - 2] & 0xffff0000) != 0xff550000) {
    std::cerr << "#E FINESSE trailer Error !! "
	      << std::hex << data[len - 2] << std::endl;
    errorflag |= 0x10;
  }
  if (data[1] != (unsigned int) len) {
    std::cerr << "#E Data length miss-match  !! "
	      << std::dec << data[1] << "/" << len << std::endl;
    errorflag |= 0x20;
  }

  /*
    std::cerr << "#D len:" << std::dec << len << "/" << data[1]
    << std::endl;
  */

  /*
    std::cerr << "COPPER DATA" << std::endl;
    for (int i = 0 ; i < 4 ; i++) {
    std::cerr << std::hex << std::setfill('0')
    << std::setw(4) << i * 4 << ":"
    << std::setw(8) << data[i * 4] << " "
    << std::setw(8) << data[i * 4 + 1] << " "
    << std::setw(8) << data[i * 4 + 2] << " "
    << std::setw(8) << data[i * 4 + 3] << std::endl;
    }
    std::cerr << "---" << std::endl;
    std::cerr << "tail:"
    << std::hex << std::setw(8)
    << data[len - 4] << " "
    << data[len - 3] << " "
    << data[len - 2] << " "
    << data[len - 1] << std::endl;
  */

  return errorflag;
}

int RecorderThread::dump_data(unsigned int *header, unsigned int *data,
			      int recv_byte)
{

  std::cerr << std::dec << std::setw(4) << m_event_number << ": ";
  std::cerr << "header[]: " << std::hex << std::setfill('0')
	    << std::setw(8) << header[0]
	    << " " << std::setw(8) << header[1] << std::endl;
  std::cerr << " event[]: " << std::hex << std::setfill('0')
	    << std::setw(8) << data[0] << " "
	    << std::setw(8) << data[1] << " "
	    << std::setw(8) << data[2] << " "
	    << std::setw(8) << data[3] << std::endl;
  std::cerr << "				: " << std::hex << std::setfill('0')
	    << std::setw(8) << data[4] << " "
	    << std::setw(8) << data[5] << " "
	    << std::setw(8) << data[6] << " "
	    << std::setw(8) << data[7] << std::endl;
  std::cerr << "				: " << std::hex << std::setfill('0')
	    << std::setw(8) << data[8] << " "
	    << std::setw(8) << data[9] << " "
	    << std::setw(8) << data[10] << " "
	    << std::setw(8) << data[11] << std::endl;
  std::cerr << "tail		: " << std::hex << std::setw(8)
	    << data[recv_byte / 4 - 3] << " "
	    << data[recv_byte / 4 - 2] << " "
	    << data[recv_byte / 4 - 1] << " "
	    << data[recv_byte / 4] << std::endl;

  return 0;
}
