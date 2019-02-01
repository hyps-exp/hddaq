/*
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "kol/kolthread.h"
#include "ControlThread/controlThread.h"

class WatchDog : public kol::Thread
{
 public:
  //WatchDog(ConsoleThread*);
  WatchDog(ControlThread*);
  virtual ~WatchDog();
  int run();
 protected:
  //ConsoleThread *m_console_p;
  ControlThread *m_controller_p;
  //private:
};

#endif
