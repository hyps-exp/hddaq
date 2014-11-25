/*
 *
 *
 */

#ifndef WATCHDOGTHREAD_INCLUDED
#define WATCHDOGTHREAD_INCLUDED

//#include <iostream>

//#include "kol/kolsocket.h"
//#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "nodeprop.h"

class WatchdogThread : public kol::Thread
{
	public:
		WatchdogThread(struct node_prop *);
		virtual ~WatchdogThread();
		int run();
	protected:
	private:
		struct node_prop *m_nodeprop;
};

#endif
