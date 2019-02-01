/*
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <kol/kolthread.h>
//#include <ControlThread/consoleThread.h>
#include <ControlThread/controlThread.h>
#include <EventBuilder/builderThread.h>
#include <EventBuilder/readerThread.h>


class WatchDog : public kol::Thread
{
 public:
  //WatchDog(ConsoleThread*, BuilderThread*, ReaderThread**, int);
  WatchDog(ControlThread*, BuilderThread*, ReaderThread**, int);
  virtual ~WatchDog();
  int run();
 protected:
  //ConsoleThread *m_console_p;
  ControlThread *m_controller_p;
  BuilderThread *m_builder_p;
  ReaderThread **m_readers_pp;
  int m_n_node;
  //private:
};

#endif
