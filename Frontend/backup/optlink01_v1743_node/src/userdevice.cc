// -*- C++ -*-

// Author: Shuhei Hayakawa
// Modified by K. Miwa

#include "userdevice.h"

#include <sstream>

#include "Header.hh"
#include "OptlinkManager.hh"
#include "OptCaenV1743.hh"

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

  int link_num;
  int conet_node;
  gOpt.AddModule( new opt::CaenV1743( link_num=0, conet_node=0, 0xADE00000 ) );
  gOpt.AddModule( new opt::CaenV1743( link_num=1, conet_node=0, 0xADE10000 ) );
  gOpt.AddModule( new opt::CaenV1743( link_num=1, conet_node=1, 0xADE20000 ) );

  gOpt.Open();

  ////////// V1743
  {
    // CAEN_DGTZ_SAMFrequency_t
    // CAEN_DGTZ_SAM_3_2GHz
    // CAEN_DGTZ_SAM_1_6GHz
    // CAEN_DGTZ_SAM_800MHz
    // CAEN_DGTZ_SAM_400MHz
    //CAEN_DGTZ_SAMFrequency_t frequency = CAEN_DGTZ_SAM_3_2GHz;
    CAEN_DGTZ_SAMFrequency_t frequency[opt::CaenV1743::NofCh] = 
      {CAEN_DGTZ_SAM_3_2GHz,
       CAEN_DGTZ_SAM_3_2GHz,
       CAEN_DGTZ_SAM_3_2GHz};

    //uint32_t record_length = 1024;
    uint32_t record_length[opt::CaenV1743::NofCh] = 
      //{140, 140, 300};
      {140, 140, 140};

    //uint32_t record_length = 130;
    bool enable_channel[][opt::CaenV1743::NofCh] = 
      {
	{true, true, true, true, true, true, true, true,
	 true, true, true, true, true, true, true, true},
	{true, true, true, true, true, true, true, true,
	 true, true, true, true, true, true, true, true},
	{true, true, true, true, true, true, false, false,
	 false, false, false, false, false, false, false, false}
      };

    // Channel DC offset
    float dc_offset[][opt::CaenV1743::NofCh] = 
      {
	{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
	{1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
      };
    
    int delay[opt::CaenV1743::NofCh] = //{35, 35, 57};
      {35, 35, 35};

    const int n = gOpt.GetNumOfModule<opt::CaenV1743>();
    for( int i=0; i<n; ++i ){
      opt::CaenV1743* m = gOpt.GetModule<opt::CaenV1743>(i);
      m->SetDefaultConfig();
      m->SetRecordLength(record_length[i]);
      m->SetSamplingFrequency(frequency[i]);
      m->SetDelay(delay[i]);

      for(int ch = 0; ch<opt::CaenV1743::NofCh; ++ch){
	if (enable_channel[i][ch]) {
	  m->SetChannelEnable(ch);
	  m->SetDCOffset(ch, dc_offset[i][ch]);
	}
      }
      m->ProgramRegister();
      m->MallocReadoutBuffer();
      m->StartRunMode();
    }// for(i)
  }// V1743


  //gOpt.Close();

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
      {// V1743
	//gOpt.Open();
	
	sleep(1);

	const int n = gOpt.GetNumOfModule<opt::CaenV1743>();

	for( int i=0; i<n; ++i ){
	  opt::CaenV1743* m = gOpt.GetModule<opt::CaenV1743>(i);
	  //m->ProgramRegister();
	  //m->MallocReadoutBuffer();
	  m->ClearData();
	  m->StartRunMode();
	  m->SetEventIndex(-1);
	}

      }// V1743

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
  //gOpt.Close();
  return;
}

//____________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{
  gOpt.Close();
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
      
      static const int n = gOpt.GetNumOfModule<opt::CaenV1743>();
      for( int i=0; i<n; ++i ){
	int dready = 0;
	opt::CaenV1743* m = gOpt.GetModule<opt::CaenV1743>(i);

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

      ////////// V1743
      {
	static const int n = gOpt.GetNumOfModule<opt::CaenV1743>();
	for( int i=0; i<n; ++i ){
	  opt::CaenV1743* m = gOpt.GetModule<opt::CaenV1743>(i);
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
