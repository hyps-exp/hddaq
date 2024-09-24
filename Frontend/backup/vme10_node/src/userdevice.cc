// vme07_node: userdevice.cc
// test version for E07 SSD DAQ System  15 September 2015

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include "userdevice.h"

#include "DetectorID.hh"
#include "apvdaq.hh"
#include "apvdaq_param.hh"
#include "fir_calibration.hh"
#include "gpio.hh"
#include "data_buffer.hh"
#include "apvdaq_function.hh"
#include "tdc.hh"

//maximum datasize by byte unit
static const int max_data_size = 4*1024*1024; 
static const int max_polling   = 2000000;     //maximum count until time-out             
static const int max_try       = 100;         //maximum count to check data ready        

DaqMode g_daq_mode = DM_NORMAL;

int get_maxdatasize()
{
  return max_data_size;
}

void open_device(NodeProp& nodeprop) // Before Run Start
{
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();


  init_gpio();
  init_apvdaq();

  conf_gpio();
  conf_apvdaq();

  open_gpio();
  open_apvdaq();

  Init_APVDAQ();
  SsdInitialization(); //-- comment

    return;
}

void init_device(NodeProp& nodeprop) // After Run Start
{
  // update DAQ mode
  g_daq_mode = nodeprop.getDaqMode();

  switch(g_daq_mode){
  case DM_NORMAL:
    {
      // fifo_clear (output buffer clear) veto_clear for apvdaq
      // io clear, busy off

      //APVDAQ
    SsdParam& ssd_param = SsdParam::get_instance();
    ssd_param.ReadPedParam();
    ssd_param.ReadThresParam();
    ssd_param.WritePedParamToFpga();
    ssd_param.WritePulseShapeParamToFpga();
    ssd_param.WritePedParamToFpga();
    ssd_param.WritePulseShapeParamToFpga();

      clear_fifo();    
      switch_buffer();  //comment
      clear_fifo();    
      switch_buffer(); //comment
      clear_fifo();    
      //      veto_clear();
      
      //___________________________________________________
      
      //GPIO
      reset_inputreg_gpio();
      make_pulse_gpio(0);
      
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
  close_gpio();
  close_apvdaq();
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
      for( int i = 0; i < max_polling; ++i )
	{
	  //	  polling
	  if(wait_gpio())
	    { 
	      return 0;
	    }
	}
      std::cout<<"wait_device() Time Out"<<std::endl;
      return -1;
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
  data_buffer & g_dbuf = data_buffer::get_instance();   
  switch(g_daq_mode){
  case DM_NORMAL:
    {

      /*
      len = 0;
      return 0;
      */
      uint32_t module_num = 0;
      uint32_t ndata = 0;

      ndata += VME_MASTER_HSIZE;

      //gpio readout
      {
	uint32_t vme_module_header_start = ndata;
	ndata += VME_MODULE_HSIZE;
	g_dbuf.clear_data();
	read_gpio();
	
	std::vector<uint32_t> data_gpio = g_dbuf.get_one_event_data();
	
	for(int i=0;i<data_gpio.size();++i)
	  {
	    data[ndata++] = data_gpio[i];
	  }
	
	VME_MODULE_HEADER vme_module_header;                         
	init_vme_module_header( &vme_module_header, address_gpio,  
				  ndata - vme_module_header_start );   
	
	memcpy( &data[vme_module_header_start],                      
		&vme_module_header, VME_MODULE_HSIZE*4 );            
	
	module_num++;
      }
      WaitDReadySsd(MASTER); //--comment out
//       WaitDReadySsd(SLAVE1);
//       WaitDReadySsd(SLAVE2);
//       WaitDReadySsd(SLAVE3);
//       WaitDReadySsd(SLAVE4);
//       WaitDReadySsd(SLAVE5);
      unsigned long temp1 = read_buffer_address(MASTER);
      switch_buffer(); //APVDAQ Switch Buffer -- comment out

      unsigned long temp2 = read_buffer_address(MASTER);
      int buffer_try = 0;
      //      while(temp1==temp2) 
      if(temp1==temp2) 
	{
	  std::stringstream ss;
	  ss<<"before: "<<temp1<<"  after: "<<temp2 <<"  try: "<<buffer_try;
	  send_warning_message("buffer address error!!");
	  send_warning_message(ss.str());
	  //	  switch_buffer(SLAVE1,temp1+1);
	  //  temp2 = read_buffer_address(SLAVE1);
	}
      //apvdaq module readout
      make_pulse_gpio(0);//Pulse Output via Channel 0 ->Busy Clear
      static int cnt_read = 0;
      for(int module = 0; module <APVDAQNUMB; ++module)
      //      for(int module = APVDAQNUMB-1; module >= 0; --module)
	{
	  uint32_t vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;

	  g_dbuf.clear_data();
	  unsigned int address = module <<24;
	  //	  if(cnt_read%100==0)
	  read_fifo_ZS(MASTER+address); //--comment out
	  //	  usleep(500);
	  //	  read_2fifo_pio(MASTER+address);
	  std::vector<uint32_t> data_apv = g_dbuf.get_one_event_data();

	  for(int i=0;i<data_apv.size();++i)
	    {
	      data[ndata++] = data_apv[i];
	    }

	  VME_MODULE_HEADER vme_module_header;                         
	  init_vme_module_header( &vme_module_header, MASTER+address,  
				  ndata - vme_module_header_start );   

	  memcpy( &data[vme_module_header_start],                      
		  &vme_module_header, VME_MODULE_HSIZE*4 );            

	  module_num++;
	}
      cnt_read++;
      //      veto_clear();
      //      make_pulse_gpio(0);//Pulse Output via Channel 0 ->Busy Clear --comment out
//        if(cnt_read%100000==0)
//  	{
//  	  long config = 0x0;
//  	  config = config | (FIRONOFF&0x1);
//  	  config = config | ((0&0x1)<<1);
//  	  config = config | ((L2TRG&0x1)<<2);
//  	  vwrite32(BROADCAST,WRITE_USER_CONF,&config);
//  	}
//        if(cnt_read%100000==1)
//  	{
//  	  long config = 0x0;
//  	  config = config | (FIRONOFF&0x1);
//  	  config = config | ((1&0x1)<<1);
//  	  config = config | ((L2TRG&0x1)<<2);
//  	  vwrite32(BROADCAST,WRITE_USER_CONF,&config);
//  	}

      clear_fifo();// apvdaq modules clear data buffer
      //      veto_clear();

      VME_MASTER_HEADER vme_master_header;                            
      init_vme_master_header( &vme_master_header, ndata, module_num );
      memcpy( &data[0], &vme_master_header, VME_MASTER_HSIZE*4 );     
      len = ndata;                                                    

//        static unsigned int cnt_end = 0;
//        std::string message = "READ END";
//        for(int i=0;i<30000;i++)
//  	{
//  	  cnt_end++;
//  	  if(!cnt_end) send_status_message(message);
//  	}

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
