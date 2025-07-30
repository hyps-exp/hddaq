#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

static uint32_t address_gpio = 0x7f030000;

struct GpioRegister_t
{
  uint32_t m_event;    //0x0000
  uint32_t m_spill;    //0x0004
  uint32_t m_serial;   //0x0008
  uint32_t m_dummy;    //0x000C
  uint32_t m_input_reg; //0x0010
  uint32_t m_reset_reg; //0x0014
  uint32_t m_level;     //0x0018
  uint32_t m_pulse;    //0x001C
};

int init_gpio();
int conf_gpio();
int open_gpio();
bool wait_gpio();
int read_gpio();
int close_gpio();


int reset_inputreg_gpio();

void make_pulse_gpio(int channels);
void make_level_gpio(int channels);
void test_gpio();

#endif
