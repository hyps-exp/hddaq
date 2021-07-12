// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "userdevice.h"

#include <fstream>
#include <sstream>

#include "Header.hh"
#include "VmeManager.hh"

#include <sys/time.h>
#include <iostream>
#include <stdlib.h>

#define USE_RMME 0
#define TPC_RM   1
#define USE_V830 0

namespace
{
  vme::VmeManager& gVme = vme::VmeManager::GetInstance();
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100;         //maximum count to check data ready
  const int max_data_size = 4*1024*1024; //maximum datasize by byte unit
  DaqMode g_daq_mode = DM_NORMAL;
  uint32_t g_evnum = 0;
  int g_gate_mode = 0;

  template <typename T>
  std::string
  user_message( T *m, const std::string& arg )
  {
    return
      std::string( gVme.GetNickName() + " : " + m->ClassName()
		   + " [" + m->AddrStr() + "] " + arg );
  }

  void
  read_gate_mode( void )
  {
    std::ifstream ifs( "/home/axis/gate.txt" );
    if( !ifs.is_open() ){
      send_fatal_message( "cannot find gate mode" );
      g_gate_mode = 0;
      std::exit( -1 );
    }
    ifs >> g_gate_mode;
  }
}

//_____________________________________________________________________________
int
get_maxdatasize( void )
{
  return max_data_size;
}

//_____________________________________________________________________________
void
open_device( NodeProp& nodeprop )
{
  gVme.SetNickName( nodeprop.getNickName() );

#if USE_RMME
  gVme.AddModule( new vme::RMME( 0xff040000 ) );
#else
  gVme.AddModule( new vme::RM( 0x7f040000 ) );
#endif

#if USE_V830
  gVme.AddModule( new vme::CaenV830( 0xcd010000 ) );
#endif

  // gVme.SetDmaAddress( 0xaa000000 );

  gVme.Open();

  read_gate_mode();
  std::ostringstream oss;
  oss << "gate mode is " << g_gate_mode;
  send_normal_message( oss.str() );

  {
#if USE_RMME
    vme::RMME* m = gVme.GetModule<vme::RMME>(0);
    int reg = vme::RMME::regSelNIM4; // Busy out from NIM4
    // int reg = 0; // Reserve1 out from NIM4 / Reserve2 in from NIM4
    reg = reg | vme::RMME::regFifoReset | vme::RMME::regInputReset | vme::RMME::regSerialReset;
    m->WriteRegister( vme::RMME::Control, reg );
    //    m->WriteRegister( vme::RMME::Pulse, 0x1 );
    m->WriteRegister( vme::RMME::FifoDepth, 0x1d);
#else
    vme::RM* m = gVme.GetModule<vme::RM>(0);
    m->WriteRegister( vme::RM::Reset, 0x1 );
    m->WriteRegister( vme::RM::Pulse, 0x1 );
    m->WriteRegister( vme::RM::Level, 0x0 | g_gate_mode << 2 );
#endif

#ifdef DebugPrint
    m->Print();
#endif
  }

#if USE_V830
  {
    vme::CaenV830* m = gVme.GetModule<vme::CaenV830>(0);
    m->WriteRegister16( vme::CaenV830::SoftReset, 0x0 );
    m->WriteRegister32( vme::CaenV830::ChEnable, 0x0 );
    uint16_t acq_mode = 0x01; // external trigger
    m->WriteRegister16( vme::CaenV830::Ctrl, acq_mode );
    m->Print();
  }
#endif

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
      read_gate_mode();
      std::ostringstream oss;
      oss << "gate mode is " << g_gate_mode;
      send_normal_message( oss.str() );

#if USE_RMME
      vme::RMME* m = gVme.GetModule<vme::RMME>(0);
      int reg = vme::RMME::regSelNIM4 | vme::RMME::regDaqGate;
      reg = reg | vme::RMME::regFifoReset | vme::RMME::regInputReset | vme::RMME::regSerialReset;
      m->WriteRegister( vme::RMME::Control, reg );
      //      m->WriteRegister( vme::RMME::Pulse, 0x1 );
      m->WriteRegister( vme::RMME::Level, 0x2 | g_gate_mode << 2 );
#else
      vme::RM* m = gVme.GetModule<vme::RM>(0);
      m->WriteRegister( vme::RM::Reset, 0x1 );
      m->WriteRegister( vme::RM::Pulse, 0x1 );
      m->WriteRegister( vme::RM::Level, 0x2 | g_gate_mode << 2 );
#endif
      g_evnum = 0;
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
#if USE_RMME
  vme::RMME* m = gVme.GetModule<vme::RMME>(0);
  int reg = vme::RMME::regSelNIM4;
  m->WriteRegister( vme::RMME::Control, reg );
  m->WriteRegister( vme::RMME::Level, 0x0 | g_gate_mode << 2 );
#else
  vme::RM* m = gVme.GetModule<vme::RM>(0);
  m->WriteRegister( vme::RM::Level, 0x0 | g_gate_mode << 2 );
#endif

  return;
}

//____________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{
  gVme.Close();
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
      ////////// Polling
#if USE_RMME
      int reg = 0;
      vme::RMME* m = gVme.GetModule<vme::RMME>(0);
      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RMME::WriteCount );
	if( (reg & 0x3ff) != 0 ) return 0; // FIFO is not empty
      }
#else
      int reg = 0;
      vme::RM* m = gVme.GetModule<vme::RM>(0);
      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RM::Input );
	if( (reg>>8)&0x1 == 0x1 ){
	  m->WriteRegister( vme::RM::Reset, 0x1 );
	  return 0;
	}
      }
#endif
      // TimeOut
      std::cout << "wait_device() Time Out" << std::endl;
      //send_warning_message( gVme.GetNickName()+" : wait_device() Time Out" );
      return -1;
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
      ////////// VME_RM
      {
#if USE_RMME
	static const int n = gVme.GetNumOfModule<vme::RMME>();
	for( int i=0; i<n; ++i ){
	  vme::RMME* m = gVme.GetModule<vme::RMME>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  unsigned int fifo_data = m->ReadRegister( vme::RMME::FifoData  );
	  unsigned int lock_bit  = fifo_data & vme::RMME::mask_lock;
	  unsigned int enc_data  = lock_bit + ((fifo_data >> vme::RMME::shift_enc) & vme::RMME::mask_enc);
	  unsigned int snc_data  = lock_bit + ((fifo_data >> vme::RMME::shift_snc) & vme::RMME::mask_snc);
	  data[ndata++] = enc_data;
	  data[ndata++] = snc_data;
	  data[ndata++] = m->ReadRegister( vme::RMME::Serial );
	  data[ndata++] = 0x1; // m->ReadRegister( vme::RM::Time   );
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;

	//std::cout << "fifo_data : " << fifo_data << std::endl;
	}
#else
	static const int n = gVme.GetNumOfModule<vme::RM>();
	for( int i=0; i<n; ++i ){
	  vme::RM* m = gVme.GetModule<vme::RM>(i);
	  // m->Print();
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  data[ndata++] = m->ReadRegister( vme::RM::Event  );
	  data[ndata++] = m->ReadRegister( vme::RM::Spill  );
	  data[ndata++] = m->ReadRegister( vme::RM::Serial );
	  // data[ndata++] = ++g_evnum;
// #if TPC_RM
// 	  std::ifstream ifs("/misc/eb1_hdd1/e03_2020dec/misc/runno.txt");
// 	  ifs >> data[ndata++];
// #else
	  data[ndata++] = 0x0; // m->ReadRegister( vme::RM::Time   );
// #endif
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}
#endif
      }

#if USE_V830
      {
	static std::string file_name( "/home/axis/scaler.txt" );
	static std::string channel_name[]
	  = { "Spark", "L1-Req", "L1-Acc", "Real-Time", "Live-Time",
	      "HTOF", "GET-Busy" };
	std::ofstream ofs( file_name.c_str() );

	vme::CaenV830* m = gVme.GetModule<vme::CaenV830>(0);
	int module_header_start = ndata;
	ndata += vme::ModuleHeaderSize;
	for( int i=0; i<32; ++i ){
	  data[ndata++] = m->ReadCounter( i );
	  if( i < 7 ){
	    ofs << std::setw(12) << std::left << channel_name[i]
		<< std::setw(10) << std::right << data[ndata-1]
		<< std::endl;
	  }
	  // printf( "%2d %08x\n", i, data[ndata-1] );
	}
	module_num++;
	vme::SetModuleHeader( m->Addr(),
			      ndata - module_header_start,
			      &data[module_header_start] );
      }
#endif

      vme::SetMasterHeader( ndata, module_num, &data[0] );

      len = ndata;
      {
#if USE_RMME
	vme::RMME* m = gVme.GetModule<vme::RMME>(0);
	m->WriteRegister( vme::RMME::Pulse, 0x1 );
#else
	vme::RM* m = gVme.GetModule<vme::RM>(0);
	m->WriteRegister( vme::RM::Pulse, 0x1 );
#endif
      }
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
