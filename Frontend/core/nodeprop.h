// -*- C++ -*-
#ifndef NODEPROP_INCLUDED
#define NODEPROP_INCLUDED

#include <kol/kolthread.h>

#include <iostream>

const int daq_port = 9000;
#define DAQ_PORT daq_port;

enum State {
        IDLE, RUNNING, INITIAL
};

struct node_prop {
	node_prop(bool lock=true);
	~node_prop();

	int getRunNumber();
	void setRunNumber(int new_value);

	int getState();
        void setState(int new_state);

	int data_port;
	int g_state;
	int run_number;
	int max_event;
	int node_id;
	char *nickname;
	kol::Thread *daq_thread;
	kol::Thread *controller;
	kol::Thread *watchdog;
	kol::Mutex  *runno_mutex;
	kol::Mutex  *state_mutex;
};

inline
node_prop::node_prop(bool lock)
: data_port(),
  g_state(),
  run_number(0),
  max_event(),
  node_id(),
  nickname(),
  daq_thread(0),
  controller(0),
  watchdog(0),
  runno_mutex(0),
  state_mutex(0)
{ 
	if (lock) {
		runno_mutex = new kol::Mutex;
		runno_mutex->lock(); 
	}
		state_mutex = new kol::Mutex;
}

inline
node_prop::~node_prop()
{
	if (runno_mutex) {
		delete runno_mutex;
		runno_mutex = 0;
	}

	if (state_mutex) {
		delete state_mutex;
		state_mutex = 0;
	}
}

inline
int node_prop::getRunNumber()
{
	if (runno_mutex)
		runno_mutex->lock();
	return run_number;
}

inline
void node_prop::setRunNumber(int new_value)
{
	run_number = new_value;
	if (runno_mutex)
		runno_mutex->unlock();
	return;
}

inline
int node_prop::getState()
{
  int ret = 0;
  if (state_mutex){
    state_mutex->lock();
    ret = g_state;
    state_mutex->unlock();
  }

  return ret;
}

inline
void node_prop::setState(int new_state)
{
  if(state_mutex){
    state_mutex->lock();
    g_state = new_state;
    state_mutex->unlock();
  }
  
  return;
}

#endif
