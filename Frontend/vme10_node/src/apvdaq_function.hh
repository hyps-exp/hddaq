#include "apvdaq_param.hh"

#ifndef APVDAQ_FUNCTION_H
#define APVDAQ_FUNCTION_H



void reset_ai2c_state(unsigned long module);
unsigned char apv_i2c_write(unsigned char address, unsigned char *i2cdata, unsigned char bytes,unsigned long module);
unsigned char apv_i2c_read(unsigned char address, unsigned long *i2cdata, unsigned char bytes,unsigned long module);
int apvi2c_writeread(unsigned char address, unsigned char reg, unsigned char i2cdata, char *verbose,unsigned long module);
int apv_readerr(unsigned char address, unsigned char i2cdata, char *verbose,unsigned long module);
int init_apv(unsigned char address, char *verbose,unsigned long module);
void apv_softreset(void);
void apv_hardreset(void);
void write_conf(long conf_register);
int phos_i2c_write(long address,long phosi2cdata,unsigned long module);
void reset_phos(void);
void set_phosdelay(bool fifo_clk_pol,int adc_clk_delay,int trg_in_delay,unsigned long module);

void init_altera(unsigned long module);
void clear_fifo(void);
void write_cnt(long cnt_word,unsigned long module);
void veto_on(unsigned int module);
void veto_clear(void);
bool is_veto(unsigned int module);
void write_seq_ram(void);
void write_dac_ram(long seq);
void write_trigger_seq(long softbits);
void init_dac_ram(void);
void write_soft_trigger(void);
void write_apv_caltrig(void);
void write_testtrigger(unsigned long module,long testrig);

void read_2fifo_pio(unsigned long module);
void read_fifo_ZS(unsigned long module);

void set_zero_suppression_mode(unsigned long module,int number);
void gettimeofday_sec(unsigned long *tv_sec, unsigned long *tv_usec);
void get_clock_gettime(unsigned long *tv_sec, unsigned long *tv_usec);

void ssleep(int time);
void msleep(int time);
void delay_us();


//19 Aug 2015 at Tokai #2 bldg.
void switch_buffer();
void switch_buffer(unsigned long module,unsigned long buffer_address);
unsigned long read_buffer_address(unsigned long module);

void FIR_ON();
void FIR_OFF();
void FIRZS_CONFIGURATION();

void WaitDReadySsd(unsigned long module);

//apvdaq_main.cc function                                                                                                                      
void Init_APVDAQ();
void Init_APVDAQ_warmStart();
void Read_APVDAQ();
int SsdInitialization();

class SsdParam
{
private:
  double mean[APVDAQNUMB][APVCHIPNUMB][128][NumberOfSamples];
  double rms[APVDAQNUMB][APVCHIPNUMB][128][NumberOfSamples];
  double DevMean[APVDAQNUMB][APVCHIPNUMB][128];
  double DevRms[APVDAQNUMB][APVCHIPNUMB][128];
  double ThresParam[APVDAQNUMB];
  double ThresParam2[APVDAQNUMB];
  double MaxPedThreshold;

public:
  SsdParam();
  ~SsdParam(){};
  static SsdParam& get_instance(void);

  void ReadPedParam();
  void ReadThresParam();
  void WritePedParamToFpga();
  void WriteDevParamToFpga();
  void WritePulseShapeParamToFpga();

  void ClearData();
};


#endif

