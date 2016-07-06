#include <iostream>
#include <sstream>
#include <stdint.h>

#include "apvdaq.hh"

#include "apvdaq_function.hh"
#include "apvdaq_param.hh"
#include "fir_calibration.hh"
#include "MessageHelper.h"

#include "data_buffer.hh"
#include "DetectorID.hh"

uint32_t event_counter_cal = 0;

void  DEV_CONFIGURATION();

void Init_APVDAQ()
{

  char verbose[65000];

  /* Startup Function */
  /* vme_define_intlevel(INTLEVEL); */

  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      init_altera(MASTER+address);
      //      init_altera(SLAVE1);
    }

  std::cout<<"Initial Veto Status"<<std::endl;
  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      printf("Module %d:\t",module);
      is_veto(MASTER+address);
    }

  veto_clear();
  std::cout<<"Veto Status After Setting Veto = 0"<<std::endl;
  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      printf("Module %d:\t",module);
      if(is_veto(MASTER+address))
      {
	std::string veto_error_message = "veto off status error";
	send_error_message(veto_error_message);
      }
    }

  veto_on(BROADCAST);
  std::cout<<"Veto Status After Setting Veto = 1"<<std::endl;
  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      printf("Module %d:\t",module);
      if(!is_veto(MASTER+address))
      {
	std::string veto_error_message = "veto on status error";
	send_error_message(veto_error_message);
      }
    }

  reset_phos();

  //Trigger & ADC Sampling clock delay setting

  //set_phosdelay(CLOCK_INVERTED,MASTER_PHOS_ADCDELAY,PHOS_TRGDELAY,MASTER);
  //  set_phosdelay(CLOCK_INVERTED,SLAVE1_PHOS_ADCDELAY,PHOS_TRGDELAY,SLAVE1);
  long PHOS_ADCDELAY[8] = {MASTER_PHOS_ADCDELAY,SLAVE1_PHOS_ADCDELAY,SLAVE2_PHOS_ADCDELAY,SLAVE3_PHOS_ADCDELAY,
			   SLAVE4_PHOS_ADCDELAY,SLAVE5_PHOS_ADCDELAY,SLAVE6_PHOS_ADCDELAY,SLAVE7_PHOS_ADCDELAY};

  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      set_phosdelay(CLOCK_INVERTED,PHOS_ADCDELAY[module],PHOS_TRGDELAY,MASTER+address);
    }
  

  // D[27..24]: Number of allowed trggers
  // D[23..16]:255-Shift register delay 
  // D[15..7]: Frame length
  // D[5..0]: Tick length 

  write_cnt( (ALLOWED_TRIGGERS<<24) | (SHIFT_REGISTER_DELAY<<16) | ((511-FRAME_LENGTH)<<7) | (63-TICK_LENGTH) , BROADCAST);

  write_seq_ram();//write sequencer RAM

  //D[29..20]:front panel
  //D[19..10]:repeater
  //D[9..0]:ADC offset

  write_dac_ram(ADC_OFFSET);//ADC  offset: 100 (usually)
  init_dac_ram();// initiate one DACRAM  sequence such that dac_ram[255] value remains as ADC offset 

  // SSD                                                                                                                                       
  /* New Sensor */
       unsigned int new_chip_address[48]
       = {  32, 33, 34, 35, 36, 37, 38, 39,33, 35, 37, 39,
   	40, 41, 42, 43, 44, 45, 46, 47, 41, 43, 45, 47};




  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);

      for(int i = 0; i < 4; ++i)
        {
          verbose[0]=0;
          if(!init_apv(new_chip_address[module*4+i],verbose,MASTER+address))
	  {
	    std::stringstream chip_fatal_message;
	    chip_fatal_message <<i+1 << "th module, "<<i+1<<"th chip initialization error!!";
	    send_fatal_message(chip_fatal_message.str());
	  }
	  std::cout << "verbose: " << verbose << std::endl;
        }
    }
  std::cout <<"APVDAQs have been initialized" << std::endl;
  std::string chip_init_message = "Chips Initialized!";
  send_status_message(chip_init_message);
}




void Read_APVDAQ()
{
  static int event_number=1;
  
  //usleep(100);
  //write_testtrigger(1);//artificial test trigger
  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      read_fifo_ZS(MASTER+address);
    }

  clear_fifo();
  //usleep(100);
  
  event_number++;
  
}
int SsdInitialization()
{
  //TURN OFF FIR FILTER                                                                                                                        

  if(FIRONOFF)
    {
      FirCalib& fir_calib = FirCalib::get_instance();
      fir_calib.ClearData();
      fir_calib.ReadFirParam();
      fir_calib.WriteFirParamToFpga();
      //Double Check
      fir_calib.WriteFirParamToFpga();
    }
  FIRZS_CONFIGURATION();

//   {
//     SsdParam& ssd_param = SsdParam::get_instance();
//     ssd_param.ReadPedParam();
//     ssd_param.ReadThresParam();
//     ssd_param.WritePedParamToFpga();
//     ssd_param.WritePulseShapeParamToFpga();
//     ssd_param.WritePedParamToFpga();
//     ssd_param.WritePulseShapeParamToFpga();
//   }
  return 0;
}
