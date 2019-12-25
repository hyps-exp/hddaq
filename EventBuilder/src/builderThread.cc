// -*- C++ -*-
/**
 *  @file   builderThread.cc
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *  @note   Modified by Ken Sakashita <kensh@post.kek.jp>
 *
 *  $Id: builderThread.cc,v 1.10 2012/04/17 06:47:12 igarashi Exp $
 *  $Log: builderThread.cc,v $
 *  Revision 1.10  2012/04/17 06:47:12  igarashi
 *  miner refine
 *
 *  Revision 1.9  2012/04/13 12:04:11  igarashi
 *  include Tomo's improvements
 *    slowReader
 *
 *  Revision 1.8  2010/06/28 08:31:50  igarashi
 *  adding C header files to accept the varied distribution compilers
 *
 *  Revision 1.7  2009/12/15 08:27:30  igarashi
 *  minar update for error message and trigger rate display
 *
 *  Revision 1.6  2009/10/19 04:38:38  igarashi
 *  add some messages
 *
 *  Revision 1.5  2008/06/27 15:30:18  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.4  2008/06/03 15:03:27  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.3  2008/05/18 15:50:14  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/05/16 06:29:22  igarashi
 *  update eventbuilder control sequence
 *
 *  Revision 1.1.1.1  2008/05/14 15:05:43  igarashi
 *  Network DAQ Software Prototype 1.5
 *
 *  Revision 1.6  2008/05/14 14:38:53  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.5  2008/05/13 06:41:57  igarashi
 *  change control sequence
 *
 *  Revision 1.4  2008/04/08 07:10:49  igarashi
 *  *** empty log message ***
 *
 *  Revision 1.3  2008/02/27 16:24:46  igarashi
 *  Control sequence was introduced in ConsoleThread and Eventbuilder.
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
 *  Revision 1.3  2007/03/07 15:27:18  igarashi
 *  change MessageClient to GlobalMessageClient
 *
 *  Revision 1.2  2007/02/20 00:14:40  nakayosi
 *  having no effect, removed "isRunning()" and "setRunFlag()".
 *
 *  Revision 1.1.1.1  2007/01/31 13:37:53  kensh
 *  Initial version.
 *
 *
 *
 *
 */

#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <cstring>
#include <ctime>

#include <sstream>
#include <iomanip>

#include "EventBuilder/EventBuilder.h"
#include "EventBuilder/builderThread.h"
#include "Message/GlobalMessageClient.h"
#include "ControlThread/GlobalInfo.h"

// #define USE_PARAPORT
#ifdef USE_PARAPORT
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#endif

BuilderThread::BuilderThread(int buflen, int quelen)
 :m_node_num(0), m_debug_print(1000)
{
  m_send_rb = new RingBuffer(buflen, quelen);
  m_command = STOP;
  m_event_number = 0;
}

BuilderThread::~BuilderThread()
{
  delete m_send_rb;
}

void BuilderThread::setAllReaders(ReaderThread ** readers, int node_num)
{
  m_readers = readers;
  m_node_num = node_num;
}

void BuilderThread::initAllNodeBuffers()
{
  for(int node=0; node<m_node_num; node++) {
    m_readers[node]->initBuffer();
  }
}

void BuilderThread::initSendBuffer()
{
  m_send_rb->initBuffer();
}

EventBuffer *BuilderThread::peekReadMergData()
{
  return m_send_rb->readBufPeek();
}

int BuilderThread::releaseReadMergData()
{
  return m_send_rb->readBufRelease();
}

EventBuffer *BuilderThread::peekWriteMergData()
{
  return m_send_rb->writeBufPeek();
}

int BuilderThread::releaseWriteMergData()
{
  return m_send_rb->writeBufRelease();
}

int BuilderThread::leftEventData()
{
  return m_send_rb->left();
}

int BuilderThread::getRingBufferDepth()
{
  return m_send_rb->depth();
}


void BuilderThread::setSemPost()
{
  for(int node=0; node<m_node_num; node++) {
    m_readers[node]->releaseReadFragData();
  }
  releaseWriteMergData();
}

void BuilderThread::setDebugPrint(int d_print)
{
  m_debug_print = d_print;
}

void BuilderThread::setParaFd(int fd_para)
{
  m_fd_para = fd_para;
}

#ifdef USE_PARAPORT
void BuilderThread::getOneShot()
{
  int dat = 128;
  ::ioctl(m_fd_para, PPWDATA, &dat);
  dat = 0;
  ::ioctl(m_fd_para, PPWDATA, &dat);
}
#endif

int BuilderThread::waitReaders()
{
  int flag = 1;
  while(flag) {
    flag = 0;
    for(int node=0; node<m_node_num; node++) {
      if (m_readers[node]->getState() != RUNNING) {
	flag = 1;
      }
    }
    usleep(10000);
  }

  return 0;
}

int BuilderThread::checkEventNumber()
{
  GlobalMessageClient& msock = GlobalMessageClient::getInstance();
  int node_event_num[m_node_num];
  unsigned int node_id[m_node_num];
  for(int i=0; i<m_node_num; i++) {
    node_event_num[i] = m_event_f[i]->getEventNo() & 0xffff;
    event_header* header
      = reinterpret_cast<event_header*>(m_event_f[i]->getBuf());
    if (header) node_id[i] = header->node_id;
  }

#if 0
  for(int i=0; i<m_node_num; i++) {
    if (m_event_number & 0xffff != node_event_num[i] & 0xffff) {
      std::cerr
	<< "#E Event number missmatch !  EB:"
	<< m_event_number
	<< ", node: " << i << " " << node_event_num[i]
	<< " " << m_event_f[i]->getEventNo()
	<< std::endl;
    }
  }
#endif

  if (m_node_num > 1) {
    int tagerr = 0;
    for(int i=1; i<m_node_num; i++) {
      if (node_event_num[0] != node_event_num[i]) {
	tagerr = 1;
      }
    }
    if (tagerr) {
      std::stringstream msg;
      msg << "#E Event Number Miss match !\n";
      msg << std::setfill(' ') << std::setw(12)
	  << "node id(hex)"
	  << " "
	  << std::setfill(' ') << std::setw(10)
	  << "event number"
	  << "\n";
      for(int i=0; i<m_node_num; i++) {
	msg << std::setfill(' ') << std::setw(12)
	    << std::showbase << std::hex
	    << node_id[i] << " "
	    << std::noshowbase << std::dec
	    << std::setfill(' ') << std::setw(10)
	    << node_event_num[i] << "\n";
	std::cerr << " " << node_event_num[i];
      }
      msock.sendString(MT_WARNING, msg.str());
      std::cerr << msg.str();
      std::cerr << std::endl;

      for(int i=0; i<m_node_num; i++) {
	std::cerr << " " << m_event_f[i]->getLength();
      }
      std::cerr << std::endl;
    }
  }

#if 0
  if (m_node_num > 1) {
    if (node_event_num[0] != node_event_num[1]) {
      std::cerr << "#E Event Number Missmatch "
		<< node_event_num[0] << " " << node_event_num[1]
		<< std::endl;
      FILE *errfile;
      errfile = fopen("error.log", "a+");
      time_t ttt = time(NULL);
      fprintf(errfile, "%s: Event Number Missmatch %d, %d\n",
	      ctime(&ttt), node_event_num[0],
	      node_event_num[1]);
      unsigned int *ev0
	= reinterpret_cast<unsigned int *>
	(m_event_f[0]->getBuf());
      unsigned int *ev1
	= reinterpret_cast<unsigned int *>
	(m_event_f[1]->getBuf());
      fprintf(errfile,
	      "ev0 %08x %08x %08x %08x %08x %08x %08x %08x\n",
	      ev0[0], ev0[1], ev0[2], ev0[3],
	      ev0[4], ev0[5], ev0[6], ev0[7]);
      fprintf(errfile,
	      "ev1 %08x %08x %08x %08x %08x %08x %08x %08x\n",
	      ev1[0], ev1[1], ev1[2], ev1[3],
	      ev1[4], ev1[5], ev1[6], ev1[7]);
      fclose(errfile);
    }
  }
#endif

  return 0;
}


int BuilderThread::active_loop()
{
  struct event_header *eheader;
  int run_number = getRunNumber();
  EventBuffer null_event(sizeof(struct event_header) + 8);
  struct event_header *nev_header
    = reinterpret_cast < struct event_header *>
    (null_event.getBuf());
  nev_header->magic = EV_MAGIC;
  nev_header->size = sizeof(struct event_header)/sizeof(int);
  nev_header->event_number = m_event_number;
  nev_header->run_number = run_number;
  nev_header->node_id = 0;
  nev_header->type = ET_NULL;
  nev_header->nblock = 0;
  nev_header->reserve = 0;

  GlobalInfo & gi = GlobalInfo::getInstance();

  //initAllNodeBuffers(); --> reader thread de yaru
  initSendBuffer();

  GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  std::cerr << "### builder entered active_loop" << std::endl;
  msock.sendString("EB: start");

  /* Activete ReaderThread flag */
  for(int node=0; node<m_node_num; node++) {
    m_readers[node]->is_active = 1;
    m_readers[node]->is_active4msg = 1;
  }

  waitReaders();
  m_state = RUNNING;
  m_event_number = 0;
  while( true ){
    if(checkCommand() != 0)
      break;
    // if((m_event_number % 100) == 0){
    //   gi.trigger_rate = checkTrigRate(100);
    // }
    int total_len=0;
    if(m_debug_print != 0){
      if((m_event_number % m_debug_print) == 0){
	std::cerr << std::dec
		  << "## BT Event Num.: "
		  << m_event_number
		  << " size: " << total_len * 4
		  << " B "
		  << "Trigger "
		  << gi.trigger_rate / 1000.0
		  << " kHz"
		  << std::endl;
      }
    }

    {
      // ReaderThread **tempreader = m_readers;
      // ofs << leftEventData();
      // std::cerr <<"#D BT: active_loop left: "
      // 		<< "send buf: " << leftEventData()
      // 		<< " read buf: ";
      // for(int i=0; i<m_node_num; i++) {
      // 	ofs << " " << m_readers[i]->leftEventData();
      // 	// std::cerr << i << ":"
      // 	// 	  << (*tempreader)->leftEventData()
      // 	// 	  << " ";
      // }
      // // std::cerr << std::endl;
      // ofs << std::endl;
    }

    /** builder thread is down when all front-ends are down **/
    int num_active = 0;
    for(int node=0; node<m_node_num; node++) {
      if (m_readers[node]->is_active) {
	num_active++;
      }
    }
    if (num_active == 0) {
      int is_noevent = 0;
      for(int node=0; node<m_node_num; node++) {
	if (m_readers[node]->leftEventData() <= 0) is_noevent++;
      }

      if (is_noevent) {
	std::cerr << "#E EB All Front-ends are down!!"
		  << std::endl;
	msock.sendString(MT_ERROR,
			 "EB: Err. All Front-ends are down.");
	break;
      }
    }

    total_len = 0;

    for(int node=0; node<m_node_num; node++) {
      if (m_readers[node]->is_active) {
	m_event_f[node] = m_readers[node]->peekReadFragData();
	if (m_event_f[node]->getHeader() != (int)EV_MAGIC) {
	  if( m_event_number > 0 ){
	    std::stringstream msg;
	    msg << "## ERROR bad event header : "
		<< std::showbase << std::hex
		<< m_event_f[node]->getHeader()
		<< std::dec << std::noshowbase
		<< " node " << node
		<< " ## ";
	    msock.sendString(MT_WARNING, msg.str());
	    std::cerr << msg.str() << std::endl;
	  }
	}
      } else {
	nev_header->event_number = m_event_number;
	m_event_f[node] = &null_event;
	if (m_readers[node]->is_active4msg) {
	  std::stringstream msg;
	  msg << "#W PUT NULL EVENT !! node: " << node;
	  msock.sendString(MT_WARNING, msg.str());
	  m_readers[node]->is_active4msg = 0;
	}
      }
      total_len = total_len + m_event_f[node]->getLength();
    }

    if (checkEventNumber()) {
      std::stringstream msg;
      msg << "#E BT: Event Number Missmatch !!";
      msock.sendString(MT_ERROR, msg.str());
      std::cerr << msg.str() << std::endl;
    }

    total_len = total_len
      + sizeof(struct event_header) / sizeof(unsigned int);

    EventBuffer *event_merg = m_send_rb->writeBufPeek();
    char *event_buf = event_merg->getBuf();
    eheader = reinterpret_cast<struct event_header*>(event_buf);
    eheader->magic        = EV_MAGIC;
    eheader->size         = total_len;
    eheader->event_number = m_event_number;
    eheader->node_id      = 1;
    eheader->run_number   = run_number;
    eheader->type         = 0;
    eheader->nblock       = m_node_num;
    eheader->reserve      = (unsigned int)std::time(0);

    int total_frag_len = 0;
    char *ptr = event_buf + sizeof(struct event_header);
    for(int node=0; node<m_node_num; node++) {
      int frag_len = m_event_f[node]->getLength();
      std::memcpy(ptr, reinterpret_cast<char*>
		  (m_event_f[node]->getBuf()),
		  frag_len * 4);
      ptr += frag_len * sizeof(int);
      total_frag_len += frag_len;
      if (m_readers[node]->is_active) {
	m_readers[node]->releaseReadFragData(); //rotate the node_rb
      }
    }

    if( (unsigned int)total_len !=
    	(total_frag_len + sizeof(struct event_header) / sizeof(unsigned int)) ){
      std::stringstream msg;
      msg << "#ERR. EB: Total length missmatch!! "
    	  << total_len << "/"
    	  << total_frag_len + sizeof(struct event_header) / sizeof(unsigned int);
      std::cout << msg.str() << std::endl;
      msock.sendString(MT_ERROR, msg.str());
      break;
    }

    m_send_rb->writeBufRelease();
    m_event_number++;

#ifdef USE_PARAPORT
    getOneShot();
#endif

  }//while()
  std::cerr << "builder exited active_loop" << std::endl;

  msock.sendString("EB: stop");

  return 0;
}

double BuilderThread::checkTrigRate(int ntimes)
{
  static struct timeval now, last;

  // int status = gettimeofday(&now, NULL);

  //GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  double elapse =
    (now.tv_sec - last.tv_sec)
    + (now.tv_usec - last.tv_usec) / 1000000.0;
  double rate = 0;
  if (elapse>0)
    rate = ntimes / elapse;
  std::cerr << " Elapse: " << std::setprecision(3) << elapse
	    << "(s) Freq: " << std::setprecision(3) << rate / 1000.0
	    << "(kHz)" << std::endl;

  last.tv_sec = now.tv_sec;
  last.tv_usec = now.tv_usec;

  return rate;
}
