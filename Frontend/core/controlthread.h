/*
 *
 *
 */

#ifndef CONTROLTHREAD_INCLUDED
#define CONTROLTHREAD_INCLUDED

//#include <iostream>

//#include "kol/kolsocket.h"
//#include "kol/koltcp.h"
#include "kol/kolthread.h"

#include "nodeprop.h"

class ControlThread : public kol::Thread
{
	public:
		ControlThread(struct node_prop *);
		virtual ~ControlThread();
		int run();
		int ackStatus();
		int sendEntry();
	protected:
		kol::Mutex        m_gstate_mutex;
		
	private:
		struct node_prop *m_nodeprop;
};

#endif
