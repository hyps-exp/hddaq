/*
 *
 *
 */

#include <cstdio>
#include <ctime>
#include <iostream>

#include "kol/kolthread.h"
#include "ControlThread/GlobalInfo.h"
#include "Message/GlobalMessageClient.h"
#include "EventBuilder/watchdog.h"

WatchDog::WatchDog(ControlThread* contp,
		   BuilderThread* builder,
		   ReaderThread** readers, int nnode)
  : m_controller_p(contp), m_builder_p(builder),
    m_readers_pp(readers), m_n_node(nnode)
{
}

WatchDog::~WatchDog()
{
  std::cerr << "#E WatchDog destructed" << std::endl;
}

int WatchDog::run()
{
  GlobalMessageClient & msock = GlobalMessageClient::getInstance();

  State nowstate;
  // State laststate = IDLE;
  std::vector<int> unactive(m_n_node, 0);

  while (true)
    {
      ::sleep(2);
      nowstate = m_controller_p->getGeneralState();
#if 0
      if ((nowstate == RUNNING) || (nowstate != laststate)) {
	m_controller_p->ackStatus();
      }
#else
      m_controller_p->ackStatus();
#endif
      // laststate = nowstate;

#if 0
      static const GlobalInfo& gi = GlobalInfo::getInstance();
      if (nowstate == RUNNING) {
	static int prescale = 0;
	if ((prescale++ % 5) == 0) {
	  std::stringstream msg;
	  msg << "EB: Trigger rate : "
	      << gi.trigger_rate/1000 << " kHz";
	  msock.sendString(msg.str());
	}
      }
      ReaderThread **reader = m_readers_pp;
      std::cerr << "#D WD: left B:" << m_builder_p->leftEventData();
      for (int i = 0 ; i < m_n_node ; i++) {
	std::cerr << " R:" << (*reader)->leftEventData();
      }
      std::cerr << " GI.state: " << gi.state;
      std::cerr << std::endl;
#endif

      if (nowstate == RUNNING) {
	for (int i=0 ; i<m_n_node ; i++) {
	  if (!((m_readers_pp[i])->is_active)) {
	    unactive.at(i)++;
	    if(unactive.at(i) >= 4){
	      std::stringstream msg;
	      msg << "EB: node " << i << " is not ACTIVE!!";
	      msock.sendString(MT_ERROR, msg.str());
	    }
	  } else {
	    unactive.at(i) = 0;
	  }
	}
      }
    }
  return 0;
}
