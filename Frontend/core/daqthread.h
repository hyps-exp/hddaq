// -*- C++ -*-
/*
 *
 *
 */

#ifndef DAQTHREAD_INCLUDED
#define DAQTHREAD_INCLUDED

#include <iostream>
#include <kol/koltcp.h>
#include <kol/kolthread.h>

#include "nodeprop.h"


const int g_EVENT_MAGIC = 0x45564e54;
#define EV_MAGIC g_EVENT_MAGIC

const int g_EVENT_TYPE_NORMAL = 0;
const int g_EVENT_TYPE_NULL   = 1;
const int g_EVENT_TYPE_DUMMY  = 2;

#define ET_NORMAL g_EVENT_TYPE_NORMAL;
#define ET_NULL g_EVENT_TYPE_NULL;
#define ET_DUMMY g_EVENT_TYPE_DUMMY;


struct event_header {
	unsigned int magic;
	unsigned int size;
	unsigned int event_number;
	unsigned int run_number;
	unsigned int node_id;
	unsigned int type;
	unsigned int nblock;
	unsigned int reserve;
};

class DaqThread : public kol::Thread
{
	public:
		DaqThread(struct node_prop *);
		virtual ~DaqThread();
		int run();
                int getState() {return m_state;};
		int getEventNumber() {return m_event_number;};
		int getEventSize() {return m_event_size;};
                int getRunNumber() {return m_run_number;};

	protected:
	private:
		int m_state;
		int m_event_number;
		int m_event_size;
                int m_run_number;
		struct node_prop *m_nodeprop;
};

#endif
