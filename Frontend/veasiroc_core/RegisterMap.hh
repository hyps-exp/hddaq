#ifndef ADDRESS_MAP_H_
#define ADDRESS_MAP_H_

// FPGA local address
const uint32_t addr_direct_ctrl = 0x00000000;
const uint32_t addr_slow_ctrl1  = 0x00000003;
const uint32_t addr_slow_ctrl2  = 0x0000003d;
const uint32_t addr_read_reg1   = 0x0000003c;
const uint32_t addr_read_reg2   = 0x00000076;
const uint32_t addr_status_reg  = 0x00000077;
const uint32_t addr_selectable  = 0x00000078;
const uint32_t addr_time_window = 0x00000079;
const uint32_t addr_pede_supp   = 0x00001000;
const uint32_t addr_version     = 0xF0000000;

// DAQ mode
const uint8_t  daq_mode_bit     = 0x01;
const uint8_t  send_adc_bit     = 0x02;
const uint8_t  send_tdc_bit     = 0x04;
const uint8_t  send_scaler_bit  = 0x08;

// Direct control register
enum indexRegEasiroc{
  i_razChn,   i_valEvt, i_resetPA, i_pwrOn,
  i_selectSc, i_loadSc, i_rstbSr,  i_rstbRead
};

enum indexRegModule{
  i_startCycle1, i_startCycle2, i_userOutput, i_ledUser,
  i_ledReady,    i_ledBusy,     i_selectHg,   i_selectProbe
};



#endif
