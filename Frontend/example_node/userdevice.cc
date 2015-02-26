// example_node: userdevice.cc

#include "userdevice.h"

//maximum datasize by byte unit
static const int max_data_size = 4*1024*1024; 

int get_maxdatasize()
{
  return max_data_size;
}

int open_device()
{
  return 0;
}

int init_device(DaqMode daq_mode)
{
  
  switch(daq_mode){
  case DM_NORMAL:
    {
      return 0;
    }
  case DM_DUMMY:
    {
      return 0;
    }
  default:
    return 0;
  }

}

int finalize_device(DaqMode daq_mode)
{
  return 0;
}

int close_device()
{
  return 0;
}


int wait_device(DaqMode daq_mode)
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  
  switch(daq_mode){
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


int read_device(DaqMode daq_mode, unsigned int* data, int& len)
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  
  switch(daq_mode){
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
