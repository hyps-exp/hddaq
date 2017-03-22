// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "userdevice.h"

#include <sstream>

#include "Header.hh"
#include "VmeManager.hh"

#define DMA_CHAIN 1
#define DMA_V792  1 // if DMA_CHAIN 0

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

  gVme.AddModule( new vme::CaenV775( 0xbd010000 ) );
  gVme.AddModule( new vme::CaenV775( 0xbd020000 ) );
  gVme.AddModule( new vme::CaenV775( 0xbd030000 ) );
  gVme.AddModule( new vme::CaenV775( 0xbd040000 ) );
  gVme.AddModule( new vme::CaenV775( 0xbd050000 ) );
  gVme.AddModule( new vme::CaenV792( 0xad010000 ) );
  gVme.AddModule( new vme::CaenV792( 0xad020000 ) );
  gVme.AddModule( new vme::CaenV792( 0xad030000 ) );
  gVme.AddModule( new vme::CaenV792( 0xad040000 ) );
  gVme.AddModule( new vme::RM( 0xff010000 ) );

  gVme.SetDmaAddress( 0xaa000000 );

  gVme.Open();

  ////////// V792
  {
    GEF_UINT16 geo_addr[]  = { 0x2, 0x4, 0x6, 0x8 };
    GEF_UINT16 chain_set[] = { 0x2, 0x3, 0x3, 0x3 };
    GEF_UINT16 overflow_suppression = 1; // 0:enable 1:disable
    GEF_UINT16 zero_suppression     = 1; // 0:enable 1:disable
    GEF_UINT16 iped[] = { 255, 255, 255, 255 }; // 0x0-0xff
    const int n = gVme.GetNumOfModule<vme::CaenV792>();
    for( int i=0; i<n; ++i ){
      vme::CaenV792* m = gVme.GetModule<vme::CaenV792>(i);
      m->WriteRegister( vme::CaenV792::GeoAddr,   geo_addr[i] );
      m->WriteRegister( vme::CaenV792::BitSet1,   0x80        );
      m->WriteRegister( vme::CaenV792::BitClr1,   0x80        );
      m->WriteRegister( vme::CaenV792::ChainAddr, 0xaa        );
      m->WriteRegister( vme::CaenV792::ChainCtrl, chain_set[i] );
      m->WriteRegister( vme::CaenV792::BitSet2,
			( overflow_suppression & 0x1 ) << 3 |
			( zero_suppression     & 0x1 ) << 4 );
      m->WriteRegister( vme::CaenV792::Iped, iped[i] );
#ifdef DebugPrint
      m->Print();
#endif
    }
  }
  ////////// V775
  {
    GEF_UINT16 geo_addr[]   = { 0xa, 0xc, 0xe, 0x10, 0x12 };
    GEF_UINT16 chain_set[]  = { 0x3, 0x3, 0x3, 0x3, 0x1 };
    GEF_UINT16 range        = 0xff; // 0x18-0xff, range: 1200-140[ns]
    GEF_UINT16 common_input = 0;    // 0:common start, 1:common stop
    GEF_UINT16 empty_prog   = 1;    // 0: if data is empty, no header and footer
                                    // 1: add header and footer always
    const int n = gVme.GetNumOfModule<vme::CaenV775>();
    for( int i=0; i<n; ++i ){
      vme::CaenV775* m = gVme.GetModule<vme::CaenV775>(i);
      m->WriteRegister( vme::CaenV775::GeoAddr,   geo_addr[i] );
      m->WriteRegister( vme::CaenV775::BitSet1,   0x80        );
      m->WriteRegister( vme::CaenV775::BitClr1,   0x80        );
      m->WriteRegister( vme::CaenV775::ChainAddr, 0xaa        );
      m->WriteRegister( vme::CaenV775::ChainCtrl, chain_set[i] );
      m->WriteRegister( vme::CaenV775::BitSet2,
			( empty_prog   & 0x1 ) << 12 |
			( common_input & 0x1 ) << 10 );
      m->WriteRegister( vme::CaenV775::Range,     range );
#ifdef DebugPrint
      m->Print();
#endif
    }
  }

  {
    vme::RM* m = gVme.GetModule<vme::RM>(0);
    m->WriteRegister( vme::RM::Reset, 0x1 );
    m->WriteRegister( vme::RM::Pulse, 0x1 );
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
      vme::RM* m = gVme.GetModule<vme::RM>(0);
      m->WriteRegister( vme::RM::Reset, 0x1 );
      m->WriteRegister( vme::RM::Pulse, 0x1 );
      m->WriteRegister( vme::RM::Level, 0x2 );
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
  vme::RM* m = gVme.GetModule<vme::RM>(0);
  m->WriteRegister( vme::RM::Level, 0x0 );
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
      vme::RM* m = gVme.GetModule<vme::RM>(0);
      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RM::Input );
	if( (reg>>8)&0x1 == 0x1 ){
	  m->WriteRegister( vme::RM::Reset, 0x1 );
	  return 0;
	}
      }
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
	static const int n = gVme.GetNumOfModule<vme::RM>();
	for( int i=0; i<n; ++i ){
	  vme::RM* m = gVme.GetModule<vme::RM>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  data[ndata++] = m->ReadRegister( vme::RM::Event  );
	  data[ndata++] = m->ReadRegister( vme::RM::Spill  );
	  data[ndata++] = m->ReadRegister( vme::RM::Serial );
	  data[ndata++] = 0x0; // m->ReadRegister( vme::RM::Time   );
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}
      }

#if DMA_CHAIN
      int dready = 0;
      for(int j=0;j<max_try;j++){
	vme::CaenV792* m = gVme.GetModule<vme::CaenV792>(0);
	dready = m->ReadRegister( vme::CaenV792::Str1 ) & 0x1;
	if(dready==1) break;
      }

      if( dready==1 ){
	gVme.ReadDmaBuf( 4*9*34 );
	//gVme.ReadDmaBuf( gVme.DmaBufLen() );
	for( int i=0; i<gVme.DmaBufLen(); ){
	  GEF_UINT32 buf = gVme.GetDmaBuf(i);
	  if( buf==0x0 || buf==0xffffffff )
	    break;

	  GEF_UINT64 vme_addr;
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  // header
	  int geo_addr = (buf>>27) & 0x1f;
	  int ncount   = (buf>> 8) & 0x3f;
	  switch( geo_addr ){
	  case 0x2: case 0x4: case 0x6: case 0x8:
	    vme_addr = 0xAD000000 | (geo_addr<<15);
	    break;
	  case 0xa: case 0xc: case 0xe: case 0x10: case 0x12:
	    vme_addr = 0xBD000000 | ( (geo_addr-0x8)<<15 );
	    break;
	  default:
	    {
	      std::ostringstream oss;
	      oss << gVme.GetNickName() << " : "
		  << "unknown GEO_ADDRESS " << geo_addr;
	      send_fatal_message( oss.str() );
	      std::exit( EXIT_FAILURE );
	    }
	  }

	  data[ndata++] = buf; i++;
	  for( int j=0; j<ncount+1; ++j ){
	    data[ndata++] = gVme.GetDmaBuf(i++);
	  }
	  vme::SetModuleHeader( vme_addr,
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}
      } else {
	send_warning_message( gVme.GetNickName()+" : data is not ready" );
      }
#else
      ////////// V792
      {
	static const int n = gVme.GetNumOfModule<vme::CaenV792>();
	for( int i=0; i<n; ++i ){
	  vme::CaenV792* m = gVme.GetModule<vme::CaenV792>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  int data_len = 34;
	  int dready   = 0;
	  for(int j=0;j<max_try;j++){
	    dready = m->ReadRegister( vme::CaenV792::Str1 ) & 0x1;
	    if(dready==1) break;
	  }
	  if(dready==1){
# if DMA_V792
	    gVme.ReadDmaBuf( m->AddrParam(), 4*data_len );
	    for(int j=0;j<data_len;j++){
	      data[ndata++] = gVme.GetDmaBuf(j);
	    }
# else
	    for(int j=0;j<data_len;j++){
	      data[ndata++] = m->DataBuf();
	    }
# endif
	  }else{
	    send_warning_message( user_message( m, "data is not ready" ) );
	  }
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}//for(i)
      }
      ////////// v775
      {
	const int n = gVme.GetNumOfModule<vme::CaenV775>();
	for( int i=0; i<n; ++i ){
	  vme::CaenV775* m = gVme.GetModule<vme::CaenV775>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  int data_len = 34;
	  int dready   = 0;
          for(int j=0;j<max_try;j++){
            dready = m->ReadRegister( vme::CaenV775::Str1 ) & 0x1;
            if(dready==1) break;
          }
	  if(dready==1){
	    for(int k=0;k<data_len;k++){
	      uint32_t data_buf = m->DataBuf();
	      data[ndata++] = data_buf;
	      int data_type = (data_buf>>24)&0x7; // 2:header, 0:data, 4:footer
	      if(data_type==4)  break;
	      if(k+1==data_len && data_type!=4){
		send_warning_message( user_message( m, "nooooo fooooter!!!" );
	      }
	    }
	  }else{
	    send_warning_message( user_message( m, "data is not ready" );
	  }

	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}
      }
#endif

      vme::SetMasterHeader( ndata, module_num, &data[0] );

      len = ndata;
      {
	vme::RM* m = gVme.GetModule<vme::RM>(0);
	m->WriteRegister( vme::RM::Pulse, 0x1 );
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
