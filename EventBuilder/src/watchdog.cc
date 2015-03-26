/*
 *
 *
 */

#include <stdio.h>

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
	GlobalInfo & gi = GlobalInfo::getInstance();
	GlobalMessageClient & msock = GlobalMessageClient::getInstance();

	State nowstate;
	State laststate = IDLE;

	while (true)
	{
		sleep(2);
		nowstate = m_controller_p->getGeneralState();
#if 0
		if ((nowstate == RUNNING) || (nowstate != laststate)) {
			m_controller_p->ackStatus();
		}
#else
		m_controller_p->ackStatus();
#endif
		laststate = nowstate;

		if (nowstate == RUNNING) {
			static int prescale = 0;
			if ((prescale++ % 5) == 0) {
				char message[128];
				sprintf(message,
				"EB: Trigger rate : %f kHz", gi.trigger_rate/1000);
				// msock.sendString(message);
			}
		}

#if 0
		ReaderThread **reader = m_readers_pp;
		std::cerr << "#D WD: left B:" << m_builder_p->leftEventData();
		for (int i = 0 ; i < m_n_node ; i++) {
			std::cerr << " R:" << (*reader)->leftEventData();
		}
		std::cerr << " GI.state: " << gi.state;
		std::cerr << std::endl;
#endif

		if (nowstate == RUNNING) {
			for (int i = 0 ; i < m_n_node ; i++) {
				if (!((m_readers_pp[i])->is_active)) {
					char message[128];
					sprintf(message,
					"#EB Error node %d is not ACTIVE!!", i);
					msock.sendString(MT_ERROR, message);
				}
			}
		}
	}

	return 0;
}
