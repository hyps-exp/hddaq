// example_node: userdevice.cc

#include <unistd.h>

#include "userdevice.h"

//maximum datasize by byte unit
static const int max_data_size = 4*1024*1024; 

DaqMode g_daq_mode = DM_NORMAL;

int get_maxdatasize()
{
  return max_data_size;
}

void open_device(NodeProp& nodeprop)
{
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();
  return;
}

void init_device(NodeProp& nodeprop)
{
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  switch(g_daq_mode){
  case DM_NORMAL:
    {
      return;
    }
  case DM_DUMMY:
    {
      return;
    }
  default:
    return;
  }
}

void finalize_device(NodeProp& nodeprop)
{
  return;
}

void close_device(NodeProp& nodeprop)
{
  return;
}

int wait_device(NodeProp& nodeprop)
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      return 0;
    }
  case DM_DUMMY:
    {
      usleep(200000);
      return 0;
    }
  default:
    return 0;
  }
}

int read_device(NodeProp& nodeprop, unsigned int* data, int& len)
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      len = 0;
      return 0;
    }
  case DM_DUMMY:
    {
      len = 0;
      return 0;
    }
  default:
    len = 0;
    return 0;
  }
}
