#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include <iostream>
#include <vector>
#include <stdint.h>

class data_buffer
{
private:
  std::vector<uint32_t> one_event_data_array[10];
  std::vector<uint32_t> one_event_data;
  data_buffer();

public:
  ~data_buffer(){};
  static data_buffer &get_instance(void);
  void add_data(uint32_t data,int i);
  void add_data(uint32_t data);

  std::vector<uint32_t> get_one_event_data(int i);
  std::vector<uint32_t> get_one_event_data();
  void clear_data();
  void clear_data(int i);
};


#endif
