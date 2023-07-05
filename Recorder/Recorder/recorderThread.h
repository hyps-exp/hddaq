// -*- C++ -*-
/**
 *  @file   recorderThread.h
 *  @brief
 *  @author Kazuo NAKAYOSHI <kazuo.nakayoshi@kek.jp>
 *  @date
 *
 *  $Id: recorderThread.h,v 1.3 2009/08/03 06:39:04 igarashi Exp $
 *  $Log: recorderThread.h,v $
 *  Revision 1.3  2009/08/03 06:39:04  igarashi
 *  add gzip compress mode
 *
 *  Revision 1.2  2009/06/29 02:50:20  igarashi
 *  add a option of the setting data directory
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
#ifndef RECORD_THREAD_H
#define RECORD_THREAD_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "kol/kolthread.h"
#include "kol/koltcp.h"
#include "ControlThread/statableThread.h"
#include "EventData/EventParam.h"

enum {REC_NORMAL, REC_COMPRESS};

class RecorderThread : public StatableThread
{
public:
  RecorderThread();
  RecorderThread(std::string hostname, int port);
  virtual ~RecorderThread();
  void setDirectoryName(const std::string& dir_name);
  void setHostname(const std::string& hostname);
  void setPortNo(int port);
  void setRecordMode(int);
  int getRecordMode();

protected:
  int active_loop();
  std::string makeFileName(int run_no);
  int checkCopperData(unsigned int*, int);
  int dump_data(unsigned int *, unsigned int *, int);

private:
  int m_port;
  std::string m_fname;
  std::string m_dir_name;
  std::string m_hostname;
  int m_rec_mode;
};

#endif
