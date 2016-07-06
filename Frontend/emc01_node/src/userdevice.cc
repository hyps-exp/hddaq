// emc01_node: userdevice.cc

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

#include "userdevice.h"
#include "vme_bit3.h"

static const int max_polling   = 2000000;     //maximum count until time-out
static const int max_try       = 100;         //maximum count to check data ready
static const int max_data_size = 4*1024*1024; //maximum datasize by byte unit

DaqMode g_daq_mode = DM_NORMAL;

int get_maxdatasize()
{
  return max_data_size;
}

void open_device(NodeProp& nodeprop)
{
  vme_open();
  return;
}

void init_device(NodeProp& nodeprop)
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      *(rpv130[0].csr1)  = 0x1; // io clear
      *(rpv130[0].pulse) = 0x1; // busy off
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
      ////////// Polling
      int reg = 0;
      for(int i=0;i<max_polling;i++){
	reg = *(rpv130[0].rsff);
	if( (reg>>0)&0x1 ){
	  *(rpv130[0].csr1) = 0x1; // io clear
	  return 0;
	}
      }
      // TimeOut
      std::cout<<"wait_device() Time Out"<<std::endl;
      //send_warning_message("vme01: wait_device() Time Out");
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
  char message[256];
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int ndata = 0;
      int module_num = 0;
      ndata += VME_MASTER_HSIZE;

      ////////// VME_RM
      for(int i=0; i<VME_RM_NUM; i++){
	int vme_module_header_start = ndata;
	ndata += VME_MODULE_HSIZE;
	data[ndata++] = *(vme_rm[0].event);
	data[ndata++] = *(vme_rm[0].spill);
	data[ndata++] = *(vme_rm[0].serial);
	data[ndata++] = *(vme_rm[0].time);
	VME_MODULE_HEADER vme_module_header;
	init_vme_module_header( &vme_module_header, vme_rm[i].addr,
				ndata - vme_module_header_start );
	memcpy( &data[vme_module_header_start],
		&vme_module_header, VME_MODULE_HSIZE*4 );
	module_num++;
      }
      ////////// EMC
      for(int i=0; i<EMC_NUM; i++){
	///// file read start
	bool ifs_flag = false;
	std::ifstream emc_ifs( emc_file.c_str() );
	if( emc_ifs.fail() ){
	  sprintf(message, "emc01: EMC[%08llx] failed to read %s",
		  emc[i].addr, emc_file.c_str() );
	  send_fatal_message(message);
	  std::exit(EXIT_FAILURE);
	}
	while( emc_ifs.good() ){
	  std::string line;
	  std::getline( emc_ifs, line );
	  if(line.empty()) continue;
	  std::istringstream input_line( line );
	  std::istream_iterator<std::string> line_begin( input_line );
	  std::istream_iterator<std::string> line_end;
	  std::vector<std::string> emc_param( line_begin, line_end );
	  if( emc_param.size() != k_emc_param_size )  continue;
	  if( emc_param[k_magic] != emc_param_magic ) continue;
	  emc[i].serial = std::strtoul( emc_param[k_serial].c_str(), NULL, 0 );
	  emc[i].xpos   = (uint32_t)std::strtod( emc_param[k_xpos].c_str(), NULL ) + x_offset;
	  emc[i].ypos   = (uint32_t)std::strtod( emc_param[k_ypos].c_str(), NULL ) + y_offset;
	  emc[i].state  = std::strtoul( emc_param[k_state].c_str(), NULL, 0 );
	  uint64_t time = std::strtoull( emc_param[k_time].c_str(), NULL, 0 );
	  emc[i].utime = (time>>36) & k_data_mask;
	  emc[i].ltime = (time>>8) & k_data_mask;
	  ifs_flag = true;
	}
	emc_ifs.close();
	///// file read end 
	
	int vme_module_header_start = ndata;
	ndata += VME_MODULE_HSIZE;

	if(!ifs_flag) std::cout<<"#W emc01: file is busy"<<std::endl;
	data[ndata++] = ( ifs_flag & k_data_mask ) | ( k_header_magic << k_word_type_shift );
	data[ndata++] = ( emc[i].serial & k_data_mask ) | ( k_serial_magic << k_word_type_shift );
	data[ndata++] = ( emc[i].xpos & k_data_mask ) | ( k_xpos_magic << k_word_type_shift );
	data[ndata++] = ( emc[i].ypos & k_data_mask ) | ( k_ypos_magic << k_word_type_shift );
	data[ndata++] = ( emc[i].state & k_data_mask ) | ( k_state_magic << k_word_type_shift );
	data[ndata++] = ( emc[i].utime & k_data_mask ) | ( k_utime_magic << k_word_type_shift );
	data[ndata++] = ( emc[i].ltime & k_data_mask ) | ( k_ltime_magic << k_word_type_shift );
	data[ndata++] = ( 0x0 & k_data_mask ) | ( k_footer_magic << k_word_type_shift );

	VME_MODULE_HEADER vme_module_header;
	init_vme_module_header( &vme_module_header, emc[i].addr,
				ndata - vme_module_header_start );
	memcpy( &data[vme_module_header_start],
		&vme_module_header, VME_MODULE_HSIZE*4 );
	module_num++;
      }

      VME_MASTER_HEADER vme_master_header;
      init_vme_master_header( &vme_master_header, ndata, module_num );
      memcpy( &data[0], &vme_master_header, VME_MASTER_HSIZE*4 );
      
      len = ndata;
      *(rpv130[0].pulse) = 0x1; // busy off
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
