// -*- C++ -*-

// Author: Shuhei Hayakawa
// Edit  : Rintaro Kurata (2024.11.30, Add local counter by software)

#include "userdevice.h"

#include <sstream>

#include "Header.hh"
#include "VmeManager.hh"

#define RMME_COUNTER 0
#define SOFTWARE_COUNTER 1 // if RMME_COUNTER = 0;

#define RED "\033[31m"
#define RESET "\033[0m"

namespace
{
  vme::VmeManager& gVme = vme::VmeManager::GetInstance();
  const int max_polling   = 2000000;     //maximum count until time-out
  const int max_try       = 100000;         //maximum count to check data ready
  const int max_data_size = 4*1024*1024; //maximum datasize by byte unit
  DaqMode g_daq_mode = DM_NORMAL;
  int evnum = 0; // for debug

  unsigned int        serial;     // Event counter (using Serial bit of RMME)
  unsigned int        local_cnt;  // Software event counter (instead of Serial of RMME)
  unsigned int mask_strange_bit = 0xFFFFFBFF;
  bool                is_slip_local_cnt = false;

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

  gVme.AddModule( new vme::CaenV1724( 0xAAD00000 ) );
  gVme.AddModule( new vme::RMME( 0xff010000 ) );

  gVme.SetDmaAddress( 0xaa000000 );

  gVme.Open();

  ////////// V1724
  {
    // Channel DC offset
    GEF_UINT32 dc_offset[][vme::CaenV1724::NofCh] = 
      {
	{0x1A00, 0x1800, 0x1900, 0x1A00, 0x1A00, 0x1D00, 0x1900, 0x1900}
      };

    // Zero suppression samples
    GEF_UINT32 n_lfwd[] = { 0x0a };
    GEF_UINT32 n_lbk[] =  { 0x14 }; // 16bit shift to left

    // Zero suppression Threshold
    GEF_UINT32 zs_threshold[][vme::CaenV1724::NofCh] = 
      {
	{/*15736*/15800, 15772, 15880, 15990, 15960, 15960, 15840, 15900}
      };
    GEF_UINT32 zs_threshold_weight = 0; // 0:fine, 1:coarse.       30bit shift to left
    GEF_UINT32 zs_operation_logic  = 1; // 0:positive, 1:negative. 31bit shift to left

    const int n = gVme.GetNumOfModule<vme::CaenV1724>();
    for( int i=0; i<n; ++i ){

      vme::CaenV1724* m = gVme.GetModule<vme::CaenV1724>(i);

      m->WriteRegister( vme::CaenV1724::SoftClear,   0x1  );
      m->WriteRegister( vme::CaenV1724::SoftReset,   0x1  );
      m->WriteRegister( vme::CaenV1724::ChEnMask,    0xff );
      m->WriteRegister( vme::CaenV1724::BufOrg,      0xa  );
      m->WriteRegister( vme::CaenV1724::PostTrig,    0x0  );

      // DC offset
      for(int ch = 0; ch<vme::CaenV1724::NofCh; ++ch){
	m->WriteRegister( vme::CaenV1724::DcOffset + (ch)*vme::CaenV1724::IReg,  dc_offset[i][ch] );
      }
      usleep(1000*1000);

      for(int ch = 0; ch<vme::CaenV1724::NofCh; ++ch){
	m->WriteRegister( vme::CaenV1724::DcOffset + (ch)*vme::CaenV1724::IReg,  dc_offset[i][ch] );

	GEF_UINT32 read;
	do{
	  read = m->ReadRegister(vme::CaenV1724::ChStatus + ch*vme::CaenV1724::IReg);
	  usleep(10);
	}while((read & 0x4) != 0x0);
      }// for(i)

      // Zero suppression
      m->WriteRegister( vme::CaenV1724::BoardConf,   0x20010 );
      for(int ch = 0; ch<vme::CaenV1724::NofCh; ++ch){
	m->WriteRegister( vme::CaenV1724::ZeroSuppThre + ch*vme::CaenV1724::IReg,
			  (zs_threshold[i][ch] )             |
			  (zs_threshold_weight & 0x1) << 30  |
			  (zs_operation_logic  & 0x1) << 31  );

	m->WriteRegister( vme::CaenV1724::ZeroSuppSample + ch*vme::CaenV1724::IReg,
			  (n_lfwd[i])       |
			  (n_lbk[i]) << 16  );

      }// for(i)

      m->WriteRegister( vme::CaenV1724::MemoryAfullLv,   0x3ef  );
      usleep(10*1000);
      //m->WriteRegister( vme::CaenV1724::CustomSize,      0x4b   );
      //m->WriteRegister( vme::CaenV1724::CustomSize,      0x5a   ); // 180 sample
      m->WriteRegister( vme::CaenV1724::CustomSize,      0x8c   ); // 280 sample
      m->WriteRegister( vme::CaenV1724::IOCtrl,          0x40   );
      m->WriteRegister( vme::CaenV1724::GPOEnMask,       0x1<<30);
      m->WriteRegister( vme::CaenV1724::AcqCtrl,         0x24   );

      GEF_UINT32 dready = 0;
      do{
	dready = m->ReadRegister( vme::CaenV1724::AcqStatus);
      }while(!(dready & 0x100));
      std::cout << "#D : Vme v1724 no" << i << " is ready for acquisition" << std::endl;

#ifdef DebugPrint
      m->Print();
#endif
    }
  }

  {
    vme::RMME* m = gVme.GetModule<vme::RMME>(0);
    int reg = vme::RMME::regSelNIM4;
    reg = reg | vme::RMME::regFifoReset | vme::RMME::regInputReset | vme::RMME::regSerialReset;
    m->WriteRegister( vme::RMME::Control, reg );

    //#if SOFTWARE_COUNTER
#if 0
    // Local counter by software
    serial = m->ReadRegister( vme::RMME::Serial );
    //    mask_check_serial = ~(serial) + 1;
    //    serial_cnt = serial + mask_check_serial;
    serial_cnt = serial & mask_check_serial;

    std::cout << "-----------------" << std::endl;
    std::cout << "[" << __func__ << "] Serial (after RESET) : " << serial << std::endl;
    std::cout << "[" << __func__ << "] Serial (calculated)  : " << serial_cnt << std::endl;
    std::cout << "-----------------" << std::endl;
    sleep(3);

#endif

    //    m->WriteRegister( vme::RMME::Pulse, 0x1 );
    //m->WriteRegister( vme::RMME::FifoDepth, 0x1d);
    m->WriteRegister( vme::RMME::FifoDepth, 0x3E8);

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
      {// V1724
	const int n = gVme.GetNumOfModule<vme::CaenV1724>();
	for(int i_try = 0; i_try<3; ++i_try){
	  for( int i=0; i<n; ++i ){
	    vme::CaenV1724* m = gVme.GetModule<vme::CaenV1724>(i);
	    m->WriteRegister( vme::CaenV1724::SoftClear, 0x1 );
	  }// for(i)
	}// for(i_try)
      }// V1724

      vme::RMME* m = gVme.GetModule<vme::RMME>(0);
      int reg = vme::RMME::regSelNIM4 | vme::RMME::regDaqGate;
      reg = reg | vme::RMME::regFifoReset | vme::RMME::regInputReset | vme::RMME::regSerialReset;
      m->WriteRegister( vme::RMME::Control, reg );
      local_cnt = 0;
      //      m->WriteRegister( vme::RMME::Pulse, 0x1 );
      m->WriteRegister( vme::RMME::Level, 0x2 );

      //#if SOFTWARE_COUNTER
#if 0
      // Local counter by software
    serial = m->ReadRegister( vme::RMME::Serial );
    //    mask_check_serial = ~(serial) + 1;
    //    serial_cnt = serial + mask_check_serial;
    serial_cnt = serial & mask_check_serial;

    std::cout << "-----------------" << std::endl;
    std::cout << "[" << __func__ << "] Serial (after RESET)    : " << serial << std::endl;
    std::cout << "[" << __func__ << "] Serial (masked 11th bit): " << serial_cnt << std::endl;
    std::cout << "-----------------" << std::endl;
    sleep(3);

    if( serial_cnt != 1){
      isSlip_local_cnt = true;
      send_error_message("Serial of RMME is not reset");
      while(isSlip_local_cnt){
	std::cout << "#E [" << __func__ << "]: Serial of RMME is not reset" << std::endl;
	sleep(3);
      }
      return;
    }
#endif
	    
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
  vme::RMME* m = gVme.GetModule<vme::RMME>(0);
  int reg = vme::RMME::regSelNIM4;
  m->WriteRegister( vme::RMME::Control, reg );
  m->WriteRegister( vme::RMME::Level, 0x0 );

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
      vme::RMME* m = gVme.GetModule<vme::RMME>(0);
      //vme::CaenV1724* m_fadc = gVme.GetModule<vme::CaenV1724>(0);
      //int dready = 0;

      for( int i=0; i<max_polling; ++i ){
	reg = m->ReadRegister( vme::RMME::WriteCount );

	//	std::cout << "RM reg(WriteCount) : " << reg << std::endl;

	//	// check Sireal Register for debug
	//	int reg_serial = 0;
	//	reg_serial = (m->ReadRegister( vme::RMME::WriteCount ) > 10 ) & 0x1;
	//	std::cout << "\033[31m" << "wait) 10th bit of Serial : " << reg_serial << "\033[0m" << std::endl;
	//	//dready = (m_fadc->ReadRegister( vme::CaenV1724::AcqStatus ) >> 3) & 0x1;

	//if( ((reg & 0x3ff) != 0) && (dready == 1)) return 0; // FIFO is not empty
	if( (reg & 0x3ff) != 0 ) return 0; // FIFO is not empty
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
      ++evnum; //for debug

      int ndata  = 0;
      int module_num = 0;
      ndata += vme::MasterHeaderSize;
      ////////// VME_RM
      {
	static const int n = gVme.GetNumOfModule<vme::RMME>();
	for( int i=0; i<n; ++i ){
	  ++local_cnt; // Local counter by software
	  vme::RMME* m = gVme.GetModule<vme::RMME>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;
	  unsigned int fifo_data = m->ReadRegister( vme::RMME::FifoData  );
	  unsigned int lock_bit  = fifo_data & vme::RMME::mask_lock;
	  unsigned int enc_data  = lock_bit + ((fifo_data >> vme::RMME::shift_enc) & vme::RMME::mask_enc);
	  unsigned int snc_data  = lock_bit + ((fifo_data >> vme::RMME::shift_snc) & vme::RMME::mask_snc);
	  data[ndata++] = enc_data;
	  data[ndata++] = snc_data;

#if SOFTWARE_COUNTER
	  // Local counter by software
	  serial     = m->ReadRegister( vme::RMME::Serial );

	  if( (serial & mask_strange_bit) == (local_cnt & mask_strange_bit) ){
	    data[ndata++] = local_cnt;
	      std::cout << "----------------" << std::endl;
	      std::cout << "[" << __func__ << "]Serial         : " << std::dec << serial << std::endl;
	      std::cout << "[" << __func__ << "]Software       : " << std::dec << local_cnt << std::endl;
	      std::cout << "[" << __func__ << "]mask           : " << std::hex << mask_strange_bit << std::endl;
	      std::cout << "[" << __func__ << "]Serial count   (masked): " << (serial & mask_strange_bit) << std::endl;
	      std::cout << "[" << __func__ << "]Software count (masked): " << (local_cnt & mask_strange_bit) << std::endl;
	      std::cout << "----------------" << std::endl;

	  }else{
	    is_slip_local_cnt = true;

	    std::ostringstream oss;
	    oss << "[" << __func__ << "]: Local count of RMME slipped";
	    send_error_message(oss.str());
	    

	    while(is_slip_local_cnt){
	      std::cout << "----------------" << std::endl;
	      std::cout << RED << "#E [" << __func__ << "]: Local count of RMME slipped" << RESET << std::endl;
	      std::cout << "----------------" << std::endl;
	      std::cout << "[" << __func__ << "]Serial         : " << std::dec << serial << std::endl;
	      std::cout << "[" << __func__ << "]Software       : " << std::dec << local_cnt << std::endl;
	      std::cout << "[" << __func__ << "]mask           : " << std::hex << mask_strange_bit << std::endl;
	      std::cout << "[" << __func__ << "]Serial count   (masked): " << (serial & mask_strange_bit) << std::endl;
	      std::cout << "[" << __func__ << "]Software count (masked): " << (local_cnt & mask_strange_bit) << std::endl;
	      std::cout << "----------------" << std::endl;
	      sleep(3);
	    }
	    	  
	  }

	  /*
	  if( ( local_cnt >> 10 ) == 1 ){ // 11th bit of local_cnt is 1
	    serial_cnt = serial;
	  }else{
	    serial_cnt = serial & 0xFFFFFCFF; // mask 11th bit
	  }

	  if( serial_cnt == local_cnt){
	    data[ndata++] = local_cnt;
	    if(local_cnt % 100 == 0) {
	      std::cout << "----------------" << std::endl;
	      std::cout << "[" << __func__ << "]Serial         : " << serial << std::endl;
	      std::cout << "[" << __func__ << "]mask           : " << 0xFFFFFCFF << std::endl;
	      std::cout << "[" << __func__ << "]Serial count   : " << serial_cnt << std::endl;
	      std::cout << "[" << __func__ << "]Software count : " << local_cnt << std::endl;
	      std::cout << "----------------" << std::endl;
	    }
	  }else{
	    isSlip_local_cnt = true;
	    std::ostringstream oss;
	    oss << "[" << __func__ << "]: Local count of RMME slipped";
	    send_error_message(oss.str());
	    while(isSlip_local_cnt){
	      std::cout << "----------------" << std::endl;
	      std::cout << RED << "#E [" << __func__ << "]: Local count of RMME slipped" << RESET << std::endl;
	      std::cout << "Serial         : " << serial << std::endl;
	      std::cout << "mask           : " << 0xFFFFFCFF << std::endl;
	      std::cout << "Serial count   : " << serial_cnt << std::endl;
	      std::cout << "Software count : " << local_cnt << std::endl;
	      std::cout << "----------------" << std::endl;
	      sleep(3);
	    }
	    return -1;
	  }
	  */
#endif

#if RMME_COUNTER
	  data[ndata++] = m->ReadRegister( vme::RMME::Serial );
#endif
	  data[ndata++] = 0x1; // m->ReadRegister( vme::RM::Time   );
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;

	  //	std::cout << "fifo_data : " << fifo_data << std::endl;

	}
      }

      ////////// V1724
      {
	static const int n = gVme.GetNumOfModule<vme::CaenV1724>();
	int dready = 0;
	for( int i=0; i<n; ++i ){
	  vme::CaenV1724* m = gVme.GetModule<vme::CaenV1724>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;

	  if(i==0){
	    for(int j=0;j<max_try;j++){
	      dready = (m->ReadRegister( vme::CaenV1724::AcqStatus ) >> 3) & 0x1;
	      if(dready==1) break;
	    }
	  }// first module

	  if(dready==1){
	    const unsigned int max_dma_len = vme::VmeManager::DmaBufLen();
	    
	    unsigned int data_len = 0;
	    do{
	      data_len = m->ReadRegister(vme::CaenV1724::EventSize);
	    }while(data_len == 0);

	    while(data_len > max_dma_len){
	      gVme.ReadDmaBuf( m->AddrParam(), 4*max_dma_len );
	      for(unsigned int j=0;j<max_dma_len;j++){
		data[ndata++] = gVme.GetDmaBuf(j);
	      } // for(j)

	      data_len -= max_dma_len;
	    }// while

	    gVme.ReadDmaBuf( m->AddrParam(), 4*data_len );
	    for(unsigned int j=0;j<data_len;j++){
	      data[ndata++] = gVme.GetDmaBuf(j);
	    }// for(j)
	    
	  }else{
	    send_warning_message( user_message( m, "data is not ready" ) );
	  }// if(dready == 1)
	  
	  vme::SetModuleHeader( m->Addr(),
				ndata - module_header_start,
				&data[module_header_start] );
	  module_num++;
	}//for(i)
      }

      vme::SetMasterHeader( ndata, module_num, &data[0] );

      len = ndata;
      {
	vme::RMME* m = gVme.GetModule<vme::RMME>(0);
	m->WriteRegister( vme::RMME::Pulse, 0x1 );
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
