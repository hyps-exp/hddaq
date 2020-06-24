// -*- C++ -*-

// Author: Shuhei Hayakawa

#include "userdevice.h"

#include <sstream>

#include "Header.hh"
#include "OptlinkManager.hh"
#include "OptCaenV1724.hh"

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
  gOpt.AddModule( new opt::CaenV1724( link_num=0, conet_node=0, 0xAAD10000 ) );
  gOpt.AddModule( new opt::CaenV1724( link_num=1, conet_node=0, 0xAAD20000 ) );

  gOpt.Open();

  ////////// V1724
  {
    // Channel DC offset
    uint32_t dc_offset[][opt::CaenV1724::NofCh] = 
      {
	{0x1D00, 0x1D00, 0x1F00, 0x1A00, 0x1B00, 0x1F00, 0x1B00, 0x1B00},
	{0x1D00, 0x1B00, 0x1C00, 0x1F00, 0x2000, 0x1B00, 0x2000, 0x1B00}
      };

    // Zero suppression samples
    uint32_t n_lfwd[] = { 0x0a, 0x0a };
    uint32_t n_lbk[] =  { 0x14, 0x14 }; // 16bit shift to left

    // Zero suppression Threshold
    uint32_t zs_threshold[][opt::CaenV1724::NofCh] = 
      {
	{15820, 15470, 15430, 16170, 16240, 15240, 16050, 15930},
	{15920, 15980, 15970, 15440, 15380, 16200, 15540, 16020}
      };
    uint32_t zs_threshold_weight = 0; // 0:fine, 1:coarse.       30bit shift to left
    uint32_t zs_operation_logic  = 1; // 0:positive, 1:negative. 31bit shift to left

    const int n = gOpt.GetNumOfModule<opt::CaenV1724>();
    for( int i=0; i<n; ++i ){

      opt::CaenV1724* m = gOpt.GetModule<opt::CaenV1724>(i);

      m->WriteRegister( opt::CaenV1724::SoftClear,   0x1  );
      m->WriteRegister( opt::CaenV1724::SoftReset,   0x1  );
      m->WriteRegister( opt::CaenV1724::ChEnMask,    0xff );
      m->WriteRegister( opt::CaenV1724::BufOrg,      0xa  );
      m->WriteRegister( opt::CaenV1724::PostTrig,    0x0  );

      // DC offset ----------------------------------------------------------------
      for(int ch = 0; ch<opt::CaenV1724::NofCh; ++ch){
	m->WriteRegister( opt::CaenV1724::DcOffset + (ch)*opt::CaenV1724::IReg,  dc_offset[i][ch] );
      }// for(ch)

      // Zero suppression ---------------------------------------------------------
      uint32_t reg_board_conf = m->ReadRegister( opt::CaenV1724::BoardConf );
      reg_board_conf = reg_board_conf | (0x2 << 16) | 0x2;
      m->WriteRegister( opt::CaenV1724::BoardConf,   reg_board_conf );
      
      for(int ch = 0; ch<opt::CaenV1724::NofCh; ++ch){
	m->WriteRegister( opt::CaenV1724::ZeroSuppThre + ch*opt::CaenV1724::IReg,
			  (zs_threshold[i][ch] )             |
			  (zs_threshold_weight & 0x1) << 30  |
			  (zs_operation_logic  & 0x1) << 31  );

	m->WriteRegister( opt::CaenV1724::ZeroSuppSample + ch*opt::CaenV1724::IReg,
			  (n_lfwd[i])       |
			  (n_lbk[i]) << 16  );

      }// for(ch)

      for(int ch = 0; ch<opt::CaenV1724::NofCh; ++ch){
       	m->WriteRegister( opt::CaenV1724::DcOffset + (ch)*opt::CaenV1724::IReg,  dc_offset[i][ch] );
	
	uint32_t read;
	do{
	  read = m->ReadRegister(opt::CaenV1724::ChStatus + ch*opt::CaenV1724::IReg);
	  usleep(10);
	}while((read & 0x4) != 0x0);
      }// for(ch)

      m->WriteRegister( opt::CaenV1724::MemoryAfullLv,   0x3ef  );
      usleep(10*1000);
      //m->WriteRegister( opt::CaenV1724::CustomSize,      0x4b   );
      //m->WriteRegister( opt::CaenV1724::CustomSize,      0x5a   ); // 180 sample
      m->WriteRegister( opt::CaenV1724::CustomSize,      0x8c   ); // 280 sample
      m->WriteRegister( opt::CaenV1724::IOCtrl,          0x40   );
      uint32_t reg_gpo_en_mask = m->ReadRegister( opt::CaenV1724::GPOEnMask );
      reg_gpo_en_mask = reg_gpo_en_mask | (0x1 << 30);
      m->WriteRegister( opt::CaenV1724::GPOEnMask,       reg_gpo_en_mask);
      m->WriteRegister( opt::CaenV1724::AcqCtrl,         0x24   );

      uint32_t dready = 0;
      do{
	dready = m->ReadRegister( opt::CaenV1724::AcqStatus);
      }while(!(dready & 0x100));
      std::cout << "#D : Vme v1724 no" << i << " is ready for acquisition" << std::endl;

#ifdef DebugPrint
      m->Print();
#endif
    }// for(i)
  }// V1724

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
      {// V1724
	gOpt.Open();
	
	sleep(1);

	const int n = gOpt.GetNumOfModule<opt::CaenV1724>();
	for(int i_try = 0; i_try<3; ++i_try){
	  for( int i=0; i<n; ++i ){
	    opt::CaenV1724* m = gOpt.GetModule<opt::CaenV1724>(i);
	    m->WriteRegister( opt::CaenV1724::SoftClear, 0x1 );
	  }// for(i)
	}// for(i_try)
      }// V1724

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
  gOpt.Close();
  return;
}

//____________________________________________________________________________
void
close_device( NodeProp& nodeprop )
{

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
      
      static const int n = gOpt.GetNumOfModule<opt::CaenV1724>();
      for( int i=0; i<n; ++i ){
	int dready = 0;
	opt::CaenV1724* m = gOpt.GetModule<opt::CaenV1724>(i);

	for(int j=0;j<max_try;j++){
	  dready = (m->ReadRegister( opt::CaenV1724::AcqStatus ) >> 3) & 0x1;
	  if(dready==1) break;
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

      ////////// V1724
      {
	static const int n = gOpt.GetNumOfModule<opt::CaenV1724>();
	for( int i=0; i<n; ++i ){
	  opt::CaenV1724* m = gOpt.GetModule<opt::CaenV1724>(i);
	  int module_header_start = ndata;
	  ndata += vme::ModuleHeaderSize;

	  const unsigned int max_blt_len = opt::OptlinkManager::BltBufLen();
	    
	  unsigned int data_len = 0;
	  do{
	    data_len = m->ReadRegister(opt::CaenV1724::EventSize);
	  }while(data_len == 0);

	  while(data_len > max_blt_len){
	    gOpt.ReadBLT( m->Handle(), opt::CaenV1724::DataBuffer, max_blt_len );
	    for(unsigned int j=0;j<max_blt_len;j++){
	      data[ndata++] = gOpt.GetBltBuf(j);
	    } // for(j)

	    data_len -= max_blt_len;
	  }// while

	  gOpt.ReadBLT( m->Handle(), opt::CaenV1724::DataBuffer, data_len );
	  for(unsigned int j=0;j<data_len;j++){
	    data[ndata++] = gOpt.GetBltBuf(j);
	  }// for(j)

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
