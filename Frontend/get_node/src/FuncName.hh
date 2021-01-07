// -*- C++ -*-

#ifndef FUNC_NAME_HH
#define FUNC_NAME_HH

#include <string>

#ifndef FUNC_NAME
#define FUNC_NAME \
  std::string("["+ClassName()+"::"+__func__+"()]")
#endif

#endif
