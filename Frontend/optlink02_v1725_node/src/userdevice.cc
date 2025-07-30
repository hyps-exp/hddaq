// -*- C++ -*-

// Author: Shuhei Hayakawa
// Modified by K. Miwa

#include "userdevice.h"

#include <sstream>

#include "Header.hh"
#include "OptlinkManager.hh"
#include "OptCaenV1725.hh"

namespace
{
  opt::OptlinkManager& gOpt = opt::OptlinkManager::GetInstance();
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100000;         //maximum count to check data ready
  const int max_data_size = 4*1024*1024; //maximum datasize by byte unit
  DaqMode g_daq_mode = DM_NORMAL;

  template <typename T>
  std::string
  user_message( T *m, const std::string& arg )
  {
    return
      std::string( gOpt.GetNickName() + " : " + m->ClassName()
		   + " [" + m->AddrStr() + "] " + arg );
  }
}

//____________________________________________________________________________
int
get_maxdatasize( void )
{
  return max_data_size;
}

//____________________________________________________________________________
void
open_device( NodeProp& nodeprop )
{
  gOpt.SetNickName( nodeprop.getNickName() );

  int pid;
  int conet_node;
  gOpt.AddModule( new opt::CaenV1725( pid=24617, conet_node=0, 0xADE00000 ) );
  //gOpt.AddModule( new opt::CaenV1725( link_num=1, conet_node=0, 0xADE10000 ) );
  //gOpt.AddModule( new opt::CaenV1725( link_num=1, conet_node=1, 0xADE20000 ) );

  gOpt.Open();
  
  ////////// V1725
  {
    // Record Length
    // Number of samples in the waveform according to the formula Ns = N * 4,
    //where Ns is the record length and N is the register value.
    // For example, write N = 6 to acquire 24 samples.
    //uint32_t record_length = 1024;
    uint32_t record_length[] = 
      {128};

    bool enable_channel[][opt::CaenV1725::NofCh] = 
      {
	// {true, true, true, true, true, true, true, true,
	{true, true, false, false, false, false, false, false,
	 false, false, false, false, false, false, false, false}
      };

    bool enable_ZLE[][opt::CaenV1725::NofCh] = 
      {
       {true, true, true, true, true, true, true, true,
       true, true, true, true, true, true, true, true}
       //{false, false, false, false, false, false, false, false,
       //false, false, false, false, false, false, false, false}
      };

    // Channel DC offset
    int dc_offset[][opt::CaenV1725::NofCh] = 
      {
	{0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff,
	 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff}
      };
    
    // pre trigger
    // Number of pre trigger samples according to the formula Ns = N * 4,
    // where Ns is the pre trigger and N is the register value.
    // NOTE: maximum value is 240.
    uint32_t preTrigger[][opt::CaenV1725::NofCh] =
      {
       {123, 123, 123, 123, 123, 123, 123, 123,
	10, 10, 10, 10, 10, 10, 10, 10 }
      };

    // Samples Look Back
    // Number of samples according to the formula N_LBCK = N * 2,
    // where N is the register value.
    uint32_t preSample[][opt::CaenV1725::NofCh] =
      {
       {20, 20, 20, 20, 20, 20, 20, 20,
	10, 10, 10, 10, 10, 10, 10, 10 }
      };

    // Samples Look Forward
    // Number of samples according to the formula N_LFW = N * 2,
    // where N is the register value.
    uint32_t postSample[][opt::CaenV1725::NofCh] =
      {
       {10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10 }
      };

    
    uint32_t baseline[][opt::CaenV1725::NofCh] =
      {
       {15500, 15300, 15300, 15300, 15300, 15300, 15300, 15300,
 	8145, 8145, 8145, 8145, 8210, 8210, 8185, 8207 }
      };

    // Number of LSB counts for the ZLE Threshold,
    // where 1 LSB = 0.12 mV for 725 and 730 series with 2 Vpp input range,
    // and 1 LSB = 0.03 mV for 725 and 730 series with 0.5 Vpp input range.
    // The threshold is referred to the baseline level.
    uint16_t ZLE_threshold[][opt::CaenV1725::NofCh] =
      {
       {20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20 }
      };


    const int n = gOpt.GetNumOfModule<opt::CaenV1725>();
    for( int i=0; i<n; ++i ){
      opt::CaenV1725* m = gOpt.GetModule<opt::CaenV1725>(i);
      m->SetDefaultConfig();
      m->SetRecordLength(record_length[i]);

      uint16_t mask=0;
      for(int ch = 0; ch<opt::CaenV1725::NofCh; ++ch){
	if (enable_channel[i][ch])
	  mask |= (1 << ch);
      }
      m->SetChannelEnable(mask);      

      for(int ch = 0; ch<opt::CaenV1725::NofCh; ++ch){
	if (enable_channel[i][ch]) {
	  m->SetDCOffset(ch, dc_offset[i][ch]);
	  m->SetPreTriggerSamples( ch, preTrigger[i][ch]);
	  m->SetPreSamples( ch, preSample[i][ch]);
	  m->SetPostSamples( ch, postSample[i][ch]);
	  m->SetBLineDefValue( ch, baseline[i][ch]);
	  if (enable_ZLE[i][ch]) {
	    m->SetEnableZLE( ch );
	    m->SetDataThreshold( ch, ZLE_threshold[i][ch] );
	    m->SetTriggerThreshold( ch, ZLE_threshold[i][ch] );
	  } else {
	    m->SetDisableZLE( ch );
	  }
	}
      }
      m->ProgramRegister();

      //m->MallocReadoutBuffer();
      //m->StartRunMode();
    }// for(i)
  }// V1725


  gOpt.Close();

  return;
}

//____________________________________________________________________________
void
init_device( NodeProp& nodeprop )
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      {// V1725
	gOpt.Open();
	
	sleep(1);

	const int n = gOpt.GetNumOfModule<opt::CaenV1725>();

	for( int i=0; i<n; ++i ){
	  opt::CaenV1725* m = gOpt.GetModule<opt::CaenV1725>(i);
	  m->ProgramRegister();
	  m->MallocReadoutBuffer();
	  m->ClearData();
	  m->SetEventIndex(-1);
	  m->StartRunMode();	  
	}

      }// V1725

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

//____________________________________________________________________________
void
finalize_device( NodeProp& nodeprop )
{
  const int n = gOpt.GetNumOfModule<opt::CaenV1725>();

  for( int i=0; i<n; ++i ){
    opt::CaenV1725* m = gOpt.GetModule<opt::CaenV1725>(i);
    m->StopRunMode();	  
  }

  gOpt.Close();
  return;
}

//____________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{
  //gOpt.Close();
  return;
}

//____________________________________________________________________________
int
wait_device( NodeProp& nodeprop )
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      bool all_module_ready = true;
      
      static const int n = gOpt.GetNumOfModule<opt::CaenV1725>();
      for( int i=0; i<n; ++i ){
	int dready = 0;
	opt::CaenV1725* m = gOpt.GetModule<opt::CaenV1725>(i);

	if (m->GetEventIndex()>=0) 
	  dready = 1;
	else {
	  for(int j=0;j<max_try;j++){

	    uint32_t buf_size = m->ReadData();
	    if (buf_size>0)
	      dready=1;

	    if(dready==1) break;
	  }
	}

	all_module_ready = dready? all_module_ready & true : false;
      }// for(i)

      if(all_module_ready){
	return 0;
      }else{
	//	send_warning_message( "data is not ready. Time out." );
	return -1;
      }

    }
  case DM_DUMMY:
    {
      ::usleep(200000);
      return 0;
    }
  default:
    return 0;
  }
}

//____________________________________________________________________________
int
read_device( NodeProp& nodeprop, unsigned int* data, int& len )
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int ndata  = 0;
      int module_num = 0;
      ndata += vme::MasterHeaderSize;

      ////////// V1725
      {
	static const int n = gOpt.GetNumOfModule<opt::CaenV1725>();
	for( int i=0; i<n; ++i ){
	  opt::CaenV1725* m = gOpt.GetModule<opt::CaenV1725>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;

	  //uint32_t eventIndex = m->GetEventIndex();
	  uint32_t nword_in_event = m->GetOneEvent((char *)&data[ndata]);
	  ndata += (int)nword_in_event;

	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;

	}//for(i)
      }

      vme::SetMasterHeader( ndata, module_num, &data[0] );

      len = ndata;

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
