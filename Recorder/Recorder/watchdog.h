/*
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <kol/kolthread.h>
#include <ControlThread/controlThread.h>

class WatchDog : public kol::Thread
{
 public:
  WatchDog(ControlThread*);
  virtual ~WatchDog();
  int run();
 protected:
  ControlThread *m_controller_p;
  //private:
};

#endif
