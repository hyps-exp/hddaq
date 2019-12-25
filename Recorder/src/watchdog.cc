/*
 *
 */

#include <iostream>

#include "kol/kolthread.h"
#include "ControlThread/controlThread.h"
#include "Recorder/watchdog.h"

WatchDog::WatchDog(ControlThread* contp) : m_controller_p(contp)
{
}

WatchDog::~WatchDog()
{
  std::cerr << "#E WatchDog destructed" << std::endl;
}

int WatchDog::run()
{
  // State nowstate;
  // State laststate = IDLE;

  while (true)
    {
      sleep(2);
      // nowstate = m_controller_p->getGeneralState();
#if 0
      if ((nowstate == RUNNING) || (nowstate != laststate)) {
	m_controller_p->ackStatus();
      }
#else
      m_controller_p->ackStatus();
#endif
      // laststate = nowstate;
    }

  return 0;
}
