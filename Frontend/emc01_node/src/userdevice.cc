// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

#include "userdevice.h"

#include "Header.hh"
#include "VmeManager.hh"

namespace
{
  vme::VmeManager& gVme = vme::VmeManager::GetInstance();
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100;         //maximum count to check data ready
  const int max_data_size = 4*1024*1024; //maximum datasize by byte unit
  DaqMode g_daq_mode = DM_NORMAL;
}

//______________________________________________________________________________
int
get_maxdatasize( void )
{
  return max_data_size;
}

//______________________________________________________________________________
void
open_device( NodeProp& nodeprop )
{
  gVme.SetNickName( nodeprop.getNickName() );

  gVme.AddModule( new vme::RM( 0xff0e0000 ) );
  gVme.AddModule( new vme::RPV130( 0x8000 ) );
  gVme.AddModule( new vme::EMC( 0xe3c00000 ) );

  gVme.Open();

  ////////// RM
  {
    vme::RM* m = gVme.GetModule<vme::RM>(0);
    m->WriteRegister( vme::RM::Reset, 0x1 );
#ifdef DebugPrint
    m->Print();
#endif
  }

  ////////// RPV130
  {
    vme::RPV130 *m = gVme.GetModule<vme::RPV130>(0);
    m->WriteRegister( vme::RPV130::Csr1, 0x1 );
    m->WriteRegister( vme::RPV130::Pulse, 0x1 );
#ifdef DebugPrint
    m->Print();
#endif
  }

  ////////// EMC
  {
    vme::EMC *m = gVme.GetModule<vme::EMC>(0);
#ifdef DebugPrint
    m->Print();
#endif
  }

  return;
}

//______________________________________________________________________________
void
init_device( NodeProp& nodeprop )
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      ////////// RPV130
      {
	vme::RPV130 *m = gVme.GetModule<vme::RPV130>(0);
	m->WriteRegister( vme::RPV130::Csr1, 0x1 );
	m->WriteRegister( vme::RPV130::Pulse, 0x1 );
	m->WriteRegister( vme::RPV130::Level, 0x2 );
      }

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

//______________________________________________________________________________
void
finalize_device( NodeProp& nodeprop )
{
  vme::RPV130 *m = gVme.GetModule<vme::RPV130>(0);
  m->WriteRegister( vme::RPV130::Level, 0x0 );

  return;
}

//______________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{
  return;
}

//______________________________________________________________________________
int
wait_device( NodeProp& nodeprop )
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      ////////// Polling
      int reg = 0;
      vme::RPV130 *m = gVme.GetModule<vme::RPV130>(0);
      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RPV130::Rsff );
	if( (reg>>0) & 0x1 ){
	  m->WriteRegister( vme::RPV130::Csr1, 0x1 );
	  return 0;
	}
      }
      // TimeOut
      std::cout<<"wait_device() Time Out"<<std::endl;
      //send_warning_message( gVme.GetNickName()+": wait_device() Time Out" );
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

//______________________________________________________________________________
int
read_device( NodeProp& nodeprop, unsigned int* data, int& len )
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int ndata = 0;
      int module_num = 0;
      ndata += vme::MasterHeaderSize;

      ////////// VME_RM
      {
	static const int n = gVme.GetNumOfModule<vme::RM>();
	for( int i=0; i<n; ++i ){
	  vme::RM *m = gVme.GetModule<vme::RM>(i);
	  int vme_module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  data[ndata++] = m->ReadRegister( vme::RM::Event  );
	  data[ndata++] = m->ReadRegister( vme::RM::Spill  );
	  data[ndata++] = m->ReadRegister( vme::RM::Serial );
	  // data[ndata++] = m->ReadRegister( vme::RM::Time   );
	  data[ndata++] = 0x0;
	  vme::SetModuleHeader( m->Addr(),
				ndata - vme_module_header_start,
				&data[vme_module_header_start] );
	  module_num++;
	}
      }

      ////////// EMC
      {
	static const int n = gVme.GetNumOfModule<vme::EMC>();
	for( int i=0; i<n; ++i ){
	  vme::EMC *m = gVme.GetModule<vme::EMC>(i);

	  bool ifs_flag = false;
	  for( int j=0; j<max_try; ++j ){
	    if( m->ReadFile() ){
	      ifs_flag = true;
	      break;
	    }
	  }

	  if( !ifs_flag ){
	    std::ostringstream oss;
	    oss << gVme.GetNickName() << " : " << std::setw(14) << m->ClassName()
		<< "[" << m->AddrStr() << "] emc file was busy";
	    send_warning_message(oss.str());
	  }

	  int vme_module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  data[ndata++] = m->ReadData( vme::EMC::Header );
	  data[ndata++] = m->ReadData( vme::EMC::Serial );
	  data[ndata++] = m->ReadData( vme::EMC::Xpos );
	  data[ndata++] = m->ReadData( vme::EMC::Ypos );
	  data[ndata++] = m->ReadData( vme::EMC::State );
	  data[ndata++] = m->ReadData( vme::EMC::Utime );
	  data[ndata++] = m->ReadData( vme::EMC::Ltime );
	  data[ndata++] = m->ReadData( vme::EMC::Footer );
	  vme::SetModuleHeader( m->Addr(),
				ndata - vme_module_header_start,
				&data[vme_module_header_start] );
	  module_num++;
	}
      }

      vme::SetMasterHeader( ndata, module_num, &data[0] );

      len = ndata;

      {
	vme::RPV130* m = gVme.GetModule<vme::RPV130>(0);
	m->WriteRegister( vme::RPV130::Pulse, 0x1 );
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
