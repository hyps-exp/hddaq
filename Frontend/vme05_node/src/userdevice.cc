// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "userdevice.h"

#include <iomanip>
#include <sstream>

#include "Header.hh"
#include "VmeManager.hh"

#define DMA_TDC64M 0

namespace
{
  vme::VmeManager& gVme = vme::VmeManager::GetInstance();
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100;         //maximum count to check data ready
  const int max_data_size = 4*1024*1024; //maximum datasize by byte unit
  DaqMode g_daq_mode = DM_NORMAL;

  template <typename T>
  std::string
  user_message( T *m, const std::string& arg )
  {
    return
      std::string( gVme.GetNickName() + " : " + m->ClassName()
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
  gVme.SetNickName( nodeprop.getNickName() );

  // gVme.AddModule( new vme::RM( 0xff050000 ) );
  gVme.AddModule( new vme::RPV130( 0x8000 ) );
  gVme.AddModule( new vme::NoticeTDC64M( 0x00510000 ) );
  gVme.AddModule( new vme::NoticeTDC64M( 0x00520000 ) );
  gVme.AddModule( new vme::NoticeTDC64M( 0x00530000 ) );
  gVme.AddModule( new vme::NoticeTDC64M( 0x00540000 ) );
  gVme.AddModule( new vme::NoticeTDC64M( 0x00550000 ) );

  gVme.Open();

  ////////// TDC64M
  {
    GEF_UINT32 reset         = 0; //
    GEF_UINT32 dynamic_range = 0; // 0-7, 2^n[us]
    GEF_UINT32 edge_mode     = 0; // 0:leading 1:leading&trailing
    GEF_UINT32 search_window = 1000/8; // 1us
    GEF_UINT32 mask_window   = 0x0;    // 0us
    const int n = gVme.GetNumOfModule<vme::NoticeTDC64M>();
    for( int i=0; i<n; ++i ){
      GEF_UINT32 module_id = i+1; // 5bit 0-31
      vme::NoticeTDC64M* m = gVme.GetModule<vme::NoticeTDC64M>(i);
      m->WriteRegister( vme::NoticeTDC64M::Ctrl,
			( ( reset         & 0x1  ) << 0 ) |
			( ( dynamic_range & 0x7  ) << 1 ) |
			( ( edge_mode     & 0x1  ) << 4 ) |
			( ( module_id     & 0x1f ) << 5 ) );
      m->WriteRegister( vme::NoticeTDC64M::Enable1, 0xffffffff );
      m->WriteRegister( vme::NoticeTDC64M::Enable2, 0xffffffff );
      m->WriteRegister( vme::NoticeTDC64M::Window,
			( ( search_window & 0xffff ) <<  0 ) |
			( ( mask_window   & 0xffff ) << 16 ) );
      m->WriteRegister( vme::NoticeTDC64M::ClStat, 0 );
#ifdef DebugPrint
      m->Print();
#endif
    }
  }

  //   ////////// RM
  //   {
  //     vme::RM* m = gVme.GetModule<vme::RM>(0);
  //     m->WriteRegister( vme::RM::Reset, 0x1 );
  // #ifdef DebugPrint
  //     m->Print();
  // #endif
  //   }

  ////////// RPV130
  {
    vme::RPV130* m = gVme.GetModule<vme::RPV130>(0);
    m->WriteRegister( vme::RPV130::Csr1, 0x1 );
    m->WriteRegister( vme::RPV130::Pulse, 0x1 );
#ifdef DebugPrint
    m->Print();
#endif
  }

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
      ////////// TDC64M
      {
	static const int n = gVme.GetNumOfModule<vme::NoticeTDC64M>();
	for( int i=0; i<n; ++i ){
	  vme::NoticeTDC64M* m = gVme.GetModule<vme::NoticeTDC64M>(i);
	  m->WriteRegister( vme::NoticeTDC64M::Ctrl,   0x1 );
	  m->WriteRegister( vme::NoticeTDC64M::ClStat, 0x0 );
	}
      }

      // ////////// RM
      // {
      // 	vme::RM* m = gVme.GetModule<vme::RM>(0);
      // 	m->WriteRegister( vme::RM::Reset, 0x1 );
      // }

      ////////// RPV130
      {
	vme::RPV130* m = gVme.GetModule<vme::RPV130>(0);
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

//____________________________________________________________________________
void
finalize_device( NodeProp& nodeprop )
{
  vme::RPV130* m = gVme.GetModule<vme::RPV130>(0);
  m->WriteRegister( vme::RPV130::Level, 0x0 );
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
      int reg = 0;
      vme::RPV130* m = gVme.GetModule<vme::RPV130>(0);
      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RPV130::Rsff );
	if( (reg>>0) & 0x1 ){
	  m->WriteRegister( vme::RPV130::Csr1, 0x1 );
	  return 0;
	}
      }
      // TimeOut
      std::cout << "wait_device() Time Out" << std::endl;
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
      int ndata      = 0;
      int module_num = 0;
      ndata += vme::MasterHeaderSize;
      ////////// VME_RM
      // {
      // 	static const int n = gVme.GetNumOfModule<vme::RM>();
      // 	for( int i=0; i<n ; ++i ){
      // 	  vme::RM* m = gVme.GetModule<vme::RM>(i);
      // 	  int module_header_start = ndata;
      // 	  ndata += vme::ModuleHeaderSize;
      // 	  data[ndata++] = m->ReadRegister( vme::RM::Event  );
      // 	  data[ndata++] = m->ReadRegister( vme::RM::Spill  );
      // 	  data[ndata++] = m->ReadRegister( vme::RM::Serial );
      // 	  data[ndata++] = 0x0; // m->ReadRegister( vme::RM::Time   );
      // 	  vme::SetModuleHeader( m->Addr(),
      // 				ndata - module_header_start,
      // 				&data[module_header_start] );
      // 	  module_num++;
      // 	}
      // }

      ////////// TDC64M
      {
	static const int n = gVme.GetNumOfModule<vme::NoticeTDC64M>();
	for( int i=0; i<n ; ++i ){
	  vme::NoticeTDC64M* m = gVme.GetModule<vme::NoticeTDC64M>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  int data_len = 0;
	  int dready   = 0;
	  for( int j=0; j<max_try; ++j ){
	    GEF_UINT32 buf32 = m->ReadRegister( vme::NoticeTDC64M::ClStat );
	    data_len = buf32 & 0xfff;
	    dready   = (buf32>>12) & 0x1;
	    if( dready==1 && data_len>0 ) break;
	  }
	  if( dready==1 ){
#if DMA_TDC64M // DmaRead ... not supported yet
	    if( data_len>gVme.DmaBufLen() ){
	      std::ostringstream oss;
	      oss << "data_len is too much "
		  << data_len << "/" << gVme.DmaBufLen();
	      send_error_message( user_message( m, oss.str() ) );
	      data_len = gVme.DmaBufLen();
	    }

	    gVme.ReadDmaBuf( m->AddrParam(), 4*data_len, vme::NoticeTDC64M::Data );
	    for( int j=0; j<data_len; ++j )
	      data[ndata++] = gVme.GetDmaBuf(j);

#else
	    for( int j=0; j<data_len; ++j )
	      data[ndata++] = m->DataBuf(j);
#endif
	  }else{
	    send_warning_message( user_message( m, "data is not ready" ) );
	  }
	  m->WriteRegister( vme::NoticeTDC64M::ClStat, 0x0 );
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}//for(i)
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
