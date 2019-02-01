// -*- C++ -*-
/**
 *  @file   EbControl.h
 *  @brief
 *  @author
 *  @date
 *
 */
#ifndef EB_CONTROL_H
#define EB_CONTROL_H

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include "kol/kolthread.h"
#include "ControlThread/statableThread.h"
#include "ControlThread/controlThread.h"


class EdControl : public ControlThread
{
public:
  int ackStatus();
  int CommandStart();
  int CommandStop();
protected:
private:
};
#endif
