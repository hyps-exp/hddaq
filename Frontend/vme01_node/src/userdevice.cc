// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "userdevice.h"

#include <sstream>

#include "Header.hh"
#include "VmeManager.hh"

#include <sys/time.h>
#include <iostream>

#define DMA_CHAIN 0
#define DMA_V792  1 // if DMA_CHAIN 0

// software counter to ignore V792 local event tag slip (2025.04.11, R.Kurata)
#define SOFTWARE_COUNTER 1

#define USE_RM 0
#define USE_RMME 1

#define USE_V775 0

#define DebugPrint 0

#define RED "\033[31m"
#define RESET "\033[0m"


namespace
{
  vme::VmeManager& gVme = vme::VmeManager::GetInstance();
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100;         //maximum count to check data ready
  const int max_data_size = 4*1024*1024; //maximum datasize by byte unit
  DaqMode g_daq_mode = DM_NORMAL;

  // software counter for event tag slip (2025.04.11, R.Kurata)
  int local_counter = 0;

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

  gVme.AddModule( new vme::CaenV792( 0xad010000 ) );
  gVme.AddModule( new vme::CaenV792( 0xad020000 ) );
  gVme.AddModule( new vme::CaenV792( 0xad030000 ) );
  //  gVme.AddModule( new vme::CaenV792( 0xad040000 ) );
  //  gVme.AddModule( new vme::CaenV792( 0x00120000 ) );
  //  gVme.AddModule( new vme::CaenV792( 0x00130000 ) );
  // gVme.AddModule( new vme::CaenV792( 0xad060000 ) );
  // gVme.AddModule( new vme::CaenV792( 0xad070000 ) );


#if USE_RMME
  gVme.AddModule( new vme::RMME( 0xffff0000 ) );
#elif USE_RM
  gVme.AddModule( new vme::RM( 0xffff0000 ) );
#endif

  gVme.SetDmaAddress( 0xaa000000 );

  gVme.Open();

  ////////// V792
  {
    GEF_UINT16 geo_addr[]  = { 0x2, 0x4, 0x6 };
    GEF_UINT16 chain_set[] = { 0x2, 0x3, 0x1 };
    // GEF_UINT16 fast_clear_window = 0x3f0; // 31.5 + 7 us
    GEF_UINT16 fast_clear_window = 0x100; // 31.5 + 7 us
    GEF_UINT16 overflow_suppression = 1; // 0:enable 1:disable
    GEF_UINT16 zero_suppression     = 1; // 0:enable 1:disable
    GEF_UINT16 all_trigger          = 0; // 0:accepted 1:all
    GEF_UINT16 iped[] = { 255, 255, 255 }; // 0x0-0xff
    const int n = gVme.GetNumOfModule<vme::CaenV792>();
    for( int i=0; i<n; ++i ){
      vme::CaenV792* m = gVme.GetModule<vme::CaenV792>(i);

      //      GEF_UINT16 firmware = m->ReadRegister( vme::CaenV792::FWRevision );
      //      std::ostringstream oss1;
      //      oss1 << "FW: " << std::hex << firmware;
      //      send_normal_message(oss1.str());

      //      std::ostringstream oss;
      //      oss << "FW version: Rev. "
      //	  << (firmware & 0xf000)
      //	  << (firmware & 0x0f00) << "."
      //	  << (firmware & 0x00f0)
      //	  << (firmware & 0x000f);
      //      send_normal_message(oss.str());

      m->WriteRegister( vme::CaenV792::GeoAddr,   geo_addr[i]  );
      m->WriteRegister( vme::CaenV792::BitSet1,   0x80         );
      m->WriteRegister( vme::CaenV792::BitClr1,   0x80         );
      m->WriteRegister( vme::CaenV792::ChainAddr, 0xaa         );
      m->WriteRegister( vme::CaenV792::ChainCtrl, chain_set[i] );
      m->WriteRegister( vme::CaenV792::FCLRWin, fast_clear_window );
      m->WriteRegister( vme::CaenV792::BitSet2,
			( overflow_suppression & 0x1 ) <<  3 |
			( zero_suppression     & 0x1 ) <<  4 |
			( all_trigger          & 0x1 ) << 14 );
      m->WriteRegister( vme::CaenV792::BitClr2, ( !all_trigger  & 0x1 ) << 14 );
      m->WriteRegister( vme::CaenV792::Iped, iped[i] );
#ifdef DebugPrint
      m->Print();
#endif
    }
  }
#if USE_V775
  ////////// V775
  {
    GEF_UINT16 geo_addr[]   = { 0xc, 0xe, 0x10};
    GEF_UINT16 chain_set[]  = { 0x3, 0x3, 0x1 };
    // GEF_UINT16 fast_clear_window = 0x3f0; // 31.5 + 7 us
    GEF_UINT16 common_input = 0; // 0:common start 1:common stop
    GEF_UINT16 empty_prog   = 1; // 0: if data is empty, no header and footer
                                 // 1: add header and footer always
    GEF_UINT16 all_trigger  = 0; // 0:accepted 1:all
    GEF_UINT16 range        = 0xff; // 0x18-0xff, range: 1200-140[ns]
    const int n = gVme.GetNumOfModule<vme::CaenV775>();
    for( int i=0; i<n; ++i ){
      vme::CaenV775* m = gVme.GetModule<vme::CaenV775>(i);
      m->WriteRegister( vme::CaenV775::GeoAddr,   geo_addr[i] );
      m->WriteRegister( vme::CaenV775::BitSet1,   0x80        );
      m->WriteRegister( vme::CaenV775::BitClr1,   0x80        );
      m->WriteRegister( vme::CaenV775::ChainAddr, 0xaa        );
      m->WriteRegister( vme::CaenV775::ChainCtrl, chain_set[i] );
      // m->WriteRegister( vme::CaenV775::FCLRWin, fast_clear_window );
      m->WriteRegister( vme::CaenV775::BitSet2,
			( common_input & 0x1 ) << 10 |
			( empty_prog   & 0x1 ) << 12 |
			( all_trigger  & 0x1 ) << 14 );
      m->WriteRegister( vme::CaenV775::BitClr2, ( !all_trigger  & 0x1 ) << 14 );
      m->WriteRegister( vme::CaenV775::Range,     range );
#ifdef DebugPrint
      m->Print();
#endif
    }
  }
#endif

  {
#if USE_RMME
    vme::RMME* m = gVme.GetModule<vme::RMME>(0);
    int reg = vme::RMME::regSelNIM4; // Busy out from NIM4
    // int reg = 0; // Reserve1 out from NIM4 / Reserve2 in from NIM4
    reg = reg | vme::RMME::regFifoReset | vme::RMME::regInputReset | vme::RMME::regSerialReset;
    m->WriteRegister( vme::RMME::Control, reg );

    //    m->WriteRegister( vme::RMME::Pulse, 0x1 );
    m->WriteRegister( vme::RMME::FifoDepth, 0x20); // 0x1d->0x20, R.Kurata(2025.03.31)
#ifdef DebugPrint
    m->Print();
#endif

#elif USE_RM
    vme::RM* m = gVme.GetModule<vme::RM>(0);
    m->WriteRegister( vme::RM::Reset, 0x1 );
    m->WriteRegister( vme::RM::Pulse, 0x1 );

#ifdef DebugPrint
    m->Print();
#endif

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
      {
	const int n = gVme.GetNumOfModule<vme::CaenV792>();
	for( int i=0; i<n; ++i ){
	  vme::CaenV792* m = gVme.GetModule<vme::CaenV792>(i);
	  m->WriteRegister( vme::CaenV792::BitSet2, 0x4 );
	  m->WriteRegister( vme::CaenV792::BitClr2, 0x4 );
	  m->WriteRegister( vme::CaenV792::EvReset, 0x0 );
	}
      }
#if USE_V775
      {
	const int n = gVme.GetNumOfModule<vme::CaenV775>();
	for( int i=0; i<n; ++i ){
	  vme::CaenV775* m = gVme.GetModule<vme::CaenV775>(i);
	  m->WriteRegister( vme::CaenV775::BitSet2, 0x4 );
	  m->WriteRegister( vme::CaenV775::BitClr2, 0x4 );
	  m->WriteRegister( vme::CaenV775::EvReset, 0x0 );
	}
      }
#endif

#if USE_RMME
      vme::RMME* m = gVme.GetModule<vme::RMME>(0);
      int reg = vme::RMME::regSelNIM4 | vme::RMME::regDaqGate;
      reg = reg | vme::RMME::regFifoReset | vme::RMME::regInputReset | vme::RMME::regSerialReset;
      m->WriteRegister( vme::RMME::Control, reg );
      //      m->WriteRegister( vme::RMME::Pulse, 0x1 );
      m->WriteRegister( vme::RMME::Level, 0x2 );
#elif USE_RM
      vme::RM* m = gVme.GetModule<vme::RM>(0);
      m->WriteRegister( vme::RM::Reset, 0x1 );
      m->WriteRegister( vme::RM::Pulse, 0x1 );
      m->WriteRegister( vme::RM::Level, 0x2 );
#endif
      local_counter = 0;
      sleep(5);
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
  m->WriteRegister( vme::RMME::Level, 0x0 );
#elif USE_RM
  vme::RM* m = gVme.GetModule<vme::RM>(0);
  m->WriteRegister( vme::RM::Level, 0x0 );
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
      bool RM_is_ready =false;
      vme::RMME* m = gVme.GetModule<vme::RMME>(0);
      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RMME::WriteCount );
	if( (reg & 0x3ff) != 0 ){
	  RM_is_ready = true; // FIFO is not empty
	  break;
	}

#if 0
	// R.Kurata, 2025/04/01
	if( (reg & 0x3ff) != 0 ){
	  std::cout << "RMME Write count: " << (reg & 0x3ff) << std::endl;
	  int n_mod = gVme.GetNumOfModule<vme::CaenV792>();
	  for(int i=0; i<n_mod; ++i){
	    vme::CaenV792* mm = gVme.GetModule<vme::CaenV792>(i);
	    int full = mm->ReadRegister(vme::CaenV792::Status2);
	    if(full){
	      std::cout << "!!! Mod #" << i << " : Buffer Full !!!" << std::endl;
	    }else{
	      std::cout << "Mod #" << i << " : Not buffer Full" << std::endl;
	    }
	  }
	  return 0; // FIFO is not empty
	}
	// R.Kurata, 2025/04/01
#endif

      }
#elif USE_RM
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

#if USE_RMME
      if ( !RM_is_ready ) return -1;
#endif

      //#if DMA_CHAIN
      static const int n = gVme.GetNumOfModule<vme::CaenV792>();
      int dready = 0;
      for( int i=0; i<n; ++i ){
      	for(int j=0;j<max_try;j++){
	  vme::CaenV792* m = gVme.GetModule<vme::CaenV792>(i);
	  dready += m->ReadRegister( vme::CaenV792::Str1 ) & 0x1;
	  if(dready==i+1) break;
	}
      }
      if( dready==n ){
	return 0;
      }
      //#endif

//       // TimeOut
//       std::cout << "wait_device() Time Out" << std::endl;
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
#elif USE_RM
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
#endif
      }

#if DMA_CHAIN
      {
	int ndata_RM = ndata;
	static const int n = gVme.GetNumOfModule<vme::CaenV792>();
	gVme.ReadDmaBuf( 4*n*34 );
	int dready = 0;
        for( int i=0; i<n; ++i ){
	  vme::CaenV792* mm = gVme.GetModule<vme::CaenV792>(i);
          for(int j=0;j<max_try;j++){
            dready += mm->ReadRegister( vme::CaenV792::Str1 ) & 0x1;
            if(dready==i+1) break;
          }
	}
	if(dready != n)
	  {
	    std::ostringstream oss;
	    oss << "DATA is NOT ready in DMA CHAIN";
	    send_fatal_message( oss.str() );
	    std::exit( EXIT_FAILURE );
	  }

	//gVme.ReadDmaBuf( gVme.DmaBufLen() );
	for( int i=0; i<gVme.DmaBufLen(); ){
	  GEF_UINT32 buf = gVme.GetDmaBuf(i);
	  //n	  printf("buf= %08x\n", buf);
	  if( buf==0x0 || buf==0xffffffff )
	    break;

	  GEF_UINT64 vme_addr;
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  // header
	  int geo_addr = (buf>>27) & 0x1f;
	  int ncount   = (buf>> 8) & 0x3f;
	  switch( geo_addr ){
	  case 0x2: case 0x4: case 0x6:
	    vme_addr = 0xad000000 | (geo_addr<<15);
	    break;
	  // case 0xc: case 0xe: case 0x10:
	  //   vme_addr = 0xBD000000 | ( (geo_addr-0x8)<<15 );
	  //   break;
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

#if SOFTWARE_COUNTER
	    int data_type = (data[ndata-1] >> 24) & 0x7;
	    switch( data_type )
	      {
	      case 4: // footer
		{
		  int geo = (data[ndata-1] >> 27) & 0x1f;
		  int count_792 = data[ndata-1] & 0xffffff;
		  std::cout << "Geo:" << geo << ", EVNT: " << count_792 << std::endl;
		  data[ndata-1] = (data[ndata-1] & 0xff000000 ) | ( local_counter & 0xffffff );
		  break;
		}		
	      default: // data or header
		std::cout << "Data" << data[ndata-1] << std::endl;
		break;
	      }
#endif // SOFTWARE_COUNTER

	  }
	  vme::SetModuleHeader( vme_addr,
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}
	if(ndata - ndata_RM == 0){
	  return -1;
	}else{
	  std::cout << "ndata: " << ndata << std::endl;
}
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
#if DMA_V792
	    gVme.ReadDmaBuf( m->AddrParam(), 4*data_len );
	    for(int j=0;j<data_len;j++){
	      data[ndata++] = gVme.GetDmaBuf(j);

#if SOFTWARE_COUNTER
	    int data_type = (data[ndata-1] >> 24) & 0x7;
	    switch( data_type )
	      {
	      case 4: // footer
		{
		  int geo = (data[ndata-1] >> 27) & 0x1f;
		  int count_792 = data[ndata-1] & 0xffffff;

		  if(local_counter < 10){
		    std::cout << "Geo:" << geo << ", EVNT: " << count_792 << std::endl;
		    std::cout << "-------------------------------" << std::endl;
		  }
		  
		  data[ndata-1] = (data[ndata-1] & 0xff000000 ) | ( local_counter & 0xffffff );
		  break;
		}
	      default: // data or header
		//		if(local_counter < 10)
		//		std::cout << "Data: " << data[ndata-1] << std::endl;
		break;
	      }
	    //	      if(j==33){
	    //data[ndata-1] = (data[ndata-1] & 0xff000000 ) | ( local_counter & 0xffffff );
	    //}
#endif // SOFTWARE_COUNTER

	    }
#else
	    for(int j=0;j<data_len;j++){
	      data[ndata++] = m->DataBuf();
	    }
#endif
	  }else{
	    send_warning_message( user_message( m, "data is not ready" ) );
	  }
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}//for(i)
      }
#if USE_V775
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
		send_warning_message( user_message( m, "nooooo fooooter!!!" ) );
	      }
	    }
	  }else{
	    send_warning_message( user_message( m, "data is not ready" ) );
	  }

	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}
      }
#endif
#endif
      if (module_num == 0)
	{
	  len = 0;
	  return -1;
	}
      vme::SetMasterHeader( ndata, module_num, &data[0] );

      len = ndata;
      {
#if USE_RMME
	vme::RMME* m = gVme.GetModule<vme::RMME>(0);
	m->WriteRegister( vme::RMME::Pulse, 0x1 );
#elif USE_RM
	vme::RM* m = gVme.GetModule<vme::RM>(0);
	m->WriteRegister( vme::RM::Pulse, 0x1 );
#endif
      }

#if SOFTWARE_COUNTER
      local_counter++;
#endif

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
