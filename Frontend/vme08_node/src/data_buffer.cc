#include "data_buffer.hh"

#include <iostream>
#include <stdint.h>
#include <vector>

//====================
data_buffer & data_buffer::get_instance(void)
{
  static data_buffer g_dbf;
  return g_dbf;
}

//====================
data_buffer::data_buffer()
{
  one_event_data.clear();
}

//====================
void data_buffer::add_data(uint32_t data,int i)
{
  one_event_data_array[i].push_back(data);
}
//====================
void data_buffer::add_data(uint32_t data)
{
  one_event_data.push_back(data);
}
//====================
std::vector<uint32_t> data_buffer::get_one_event_data(int i)
{
  return one_event_data_array[i];
}
//====================
std::vector<uint32_t> data_buffer::get_one_event_data()
{
  return one_event_data;
}
//====================
void data_buffer::clear_data()
{
  one_event_data.clear();
  for(int i = 0; i<10; i++)
    {
      one_event_data_array[i].clear();
    }
}
//====================
void data_buffer::clear_data(int i)
{
  one_event_data_array[i].clear();
}
