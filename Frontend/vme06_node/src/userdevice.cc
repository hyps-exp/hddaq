// vme06_node: userdevice.cc

#include "userdevice.h"
#include "vme_xvb.h"
#include "spill_off.h"

#define DMA_TDC64M 0 // not supported
#define DMA_V830   0 // not supported

static const int max_polling   = 2000000;     //maximum count until time-out
static const int max_try       = 100;         //maximum count to check data ready
static const int max_data_size = 4*1024*1024; //maximum datasize by byte unit

DaqMode g_daq_mode = DM_NORMAL;

////////// spill off
volatile int spill_flag = k_init;
std::string  off_data_name;
gzFile      *off_data_file;
int off_event_number;

int get_maxdatasize()
{
  return max_data_size;
}

void open_device(NodeProp& nodeprop)
{
  char message[256];
  vme_open();
  ////////// V830
  for(int i=0;i<V830_NUM;i++){
    *(v830[i].reset)  = __bswap_16(0x00);
    *(v830[i].enable) = __bswap_16(0x00); // for MEB, v830 only
    ///// acq_mode
    // 00: disabled (default)
    // 01: external or from VME
    // 10: periodical
    uint32_t acq_mode = 0x01;
    *(v830[i].clr)    = __bswap_16(acq_mode);
  }
  ////////// CAMAC(FERA)
  *(rpv130[0].pulse) = __bswap_16(0x02); // fera clear
  ////////// UMEM
  for(int i=0;i<UMEM_NUM;i++){
    *(umem_reg[i].flexs) = __bswap_16(0x0); // init
    *(umem_reg[i].clr)   = __bswap_16(0x1); // clear
    *(umem_reg[i].mode)  = __bswap_16(0x1); // gate open
  }
  ////////// TDC64M
  uint32_t reset           = 1;                 // clear local event counter
  uint32_t dynamic_range[] = { 3, 7 };          // 0-7, 2^n[us]
  uint32_t edge_mode       = 0;                 // 0:leading 1:leading&trailing
  uint32_t module_id       = 0;                 // 5bit, 0-511
  uint32_t search_window[] = { 0x3E8, 0x3E80 }; // 8ns unit, 0x0-0x3E80(0-128us)
  uint32_t mask_window[]   = { 0x0, 0x0 };      // 8ns unit, 0x0-0x3E80(0-128us)
  for(int i=0;i<TDC64M_NUM;i++){
    module_id = i +20;
    *(tdc64m[i].ctr) = __bswap_32( (reset&0x1 ) |
				   ((dynamic_range[i]&0x7)<<1) |
				   ((edge_mode&0x1)<<4) |
				   ((module_id&0x1f)<<5) );
    *(tdc64m[i].enable1) = __bswap_32(0xffffffff);
    *(tdc64m[i].enable2) = __bswap_32(0xffffffff);
    *(tdc64m[i].window)  = __bswap_32( (search_window[i]&0xffff) |
				       ((mask_window[i]&0xffff)<<16) );
    *(tdc64m[i].str) = 0;// tdc64m clear    
  }
  ////////// spill off
  struct stat st;
  int status = stat(off_data_dir.c_str(), &st);
  if(status!=0){
    sprintf(message, "vme06: spill off data directory does not exist -- %s", off_data_dir.c_str());
    send_fatal_message(message);
    std::exit(-1);
  }
  return;
}

void init_device(NodeProp& nodeprop)
{
  char message[256];
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      
      ////////// spill off
      char name[256];
      sprintf(name, "%s/off_run%05d.dat.gz", off_data_dir.c_str(), nodeprop.getRunNumber());
      off_data_name = name;

      struct stat st;
      int status = stat(off_data_name.c_str(), &st);
      off_data_file = (gzFile *)gzopen(off_data_name.c_str(), "wb6");
      if(status==0){
	sprintf(message, "vme06: spill off data file already exists: %s", off_data_name.c_str());
	send_fatal_message(message);
	std::exit(-1);
      }else{
	sprintf(message, "vme06: create spill off data file: %s", off_data_name.c_str());
	send_normal_message(message);
      }
      off_event_number = 0;
     
      *(rst_flag[0].clear) = __bswap_16(0x0); // clear reset flag register
      
      *(rpv130[0].csr1)  = __bswap_16(0x01);// io clear
      *(rpv130[0].pulse) = __bswap_16(0x03);// spill on/off busy off
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
  char message[256];
  for(int i=0;i<UMEM_NUM;i++){
    *(umem_reg[i].mode) = __bswap_16(0x0); // gate close
    *(umem_reg[i].clr)  = __bswap_16(0x1); // clear
  }
  *(rpv130[0].pulse) = __bswap_16(0x02); // fera clear

  ////////// spill off
  sprintf(message, "vme06: close spill off data file: %s", off_data_name.c_str());
  send_normal_message(message);
  gzclose(off_data_file);
  chmod(off_data_name.c_str(), S_IRUSR | S_IRGRP | S_IROTH );
  return;
}

void close_device(NodeProp& nodeprop)
{
  vme_close();
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
      spill_flag = k_init;
      ////////// Polling
      int reg = 0;
      for(int i=0;i<max_polling;i++){
	reg = __bswap_16(*(rpv130[0].rsff));
	if( (reg>>0)&0x1 && (reg>>1)&0x1 ){
	  send_fatal_message("vme06: accept both spill on/off flags!!!");
	  std::exit(-1);
	};
	///// spill on
	if( (reg>>0)&0x1 ){
	  spill_flag = k_spill_on;
	  *(rpv130[0].csr1)  = __bswap_16(0x01);// io clear
	  return 0;
	}
	///// spill off
	if( (reg>>1)&0x1 ){
	  spill_flag = k_spill_off;
	  *(rpv130[0].csr1)  = __bswap_16(0x01);// io clear
	  return 0;
	}
      }
      // TimeOut
      std::cout<<"wait_device() Time Out"<<std::endl;
      //send_warning_message_message("vme06: wait_device() Time Out");
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
      int ndata      = 0;
      int module_num = 0;
      ndata += VME_MASTER_HSIZE;
      ////////// VME_RM
      if(spill_flag==k_spill_on){
	for(int i=0;i<VME_RM_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  data[ndata++] = __bswap_32(*(vme_rm[i].event));
	  data[ndata++] = __bswap_32(*(vme_rm[i].spill));
	  data[ndata++] = __bswap_32(*(vme_rm[i].serial));
	  data[ndata++] = 0x0; // spill_end_flag
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, vme_rm[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
      }
      ////////// RST_FLAG
      if(spill_flag==k_spill_off){
	for(int i=0;i<RST_FLAG_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  data[ndata++] = __bswap_32(*(rst_flag[i].flag));
	  *(rst_flag[0].clear) = __bswap_16(0x0); // clear reset flag register
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, rst_flag[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
      }

      ////////// TDC64M
      if(spill_flag==k_spill_on){
	for(int i=0;i<TDC64M_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 0;
	  int dready   = 0;
	  for(int j=0;j<max_try;j++){
	    uint32_t buf32 = __bswap_32(*(tdc64m[i].str));
	    data_len = buf32 & 0xfff;
	    dready   = (buf32>>12) & 0x1;
	    if(dready==1 && data_len>0) break;
	  }
	  if(dready==1){
#if DMA_TDC64M // DmaRead ... not supported
	    if(data_len>DMA_BUF_LEN){
	      sprintf(message, "vme06: TDC64M[%08llx] data is too much -- %d/%d",
		      tdc64m[i].addr, data_len, DMA_BUF_LEN);
	      send_error_message(message);
	      data_len = DMA_BUF_LEN;
	    }
	    int status = gefVmeReadDmaBuf(dma_hdl, &tdc64m[i].addr_param, 0x4000, 4*data_len);
	    if(status!=0){
	      sprintf(message, "vme06: TDC64M[%08llx] gefVmeReadDmaBuf() failed -- %d",
		      tdc64m[i].addr, GEF_GET_ERROR(status));
	      send_error_message(message);
	    }else{
	      for(int j=0;j<data_len;j++) data[ndata++] = __bswap_32(dma_buf[j]);
	    }
#else
	    for(int j=0;j<data_len;j++){
	      data[ndata++] = __bswap_32(tdc64m[i].data_buf[j]);
	    }		
#endif
	  }else{// dready!=1
	    sprintf(message, "vme06: TDC64M[%08llx] data is not ready", tdc64m[i].addr);
	    send_warning_message(message);
	  }
	  *(tdc64m[i].str) = 0;// tdc64m clear
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, tdc64m[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}//for(i)
      }

      ////////// V830
      if(spill_flag==k_spill_on){
	for(int i=0;i<V830_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 32;
#if DMA_V830
	  int dready   = 0;
	  for(int j=0;j<max_try;j++){
	    dready = __bswap_16(*(v830[i].clr))&0x1;
	    if(dready==1) break;
	  }
	  if(dready==1){
	    int status = gefVmeReadDmaBuf(dma_hdl, &v830[i].addr_param, 0, 4*data_len);
	    if(status!=0){
	      sprintf(message, "vme06: V830[%08llx] gefVmeReadDmaBuf() failed -- %d",
		      v830[i].addr, GEF_GET_ERROR(status));
	      send_error_message(message);
		}else{
	      for(int j=0;j<data_len;j++){
		data[ndata++] = __bswap_32(dma_buf[j]);
	      }
	    }
	  }else{
	    sprintf(message, "vme06: V830[%08llx] data is not ready", v830[i].addr);
	    send_warning_message(message);
	  }
#else
	  for(int j=0;j<data_len;j++){
	    data[ndata++] = __bswap_32(*(v830[i].counter[j]));
	  }
#endif
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, v830[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
      }

      ////////// UMEM
      if(spill_flag==k_spill_on || spill_flag==k_spill_off){
	for(int i=0;i<UMEM_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  *(umem_reg[i].mode) = __bswap_16(0x0); // gate close
	  uint32_t data_len = __bswap_16(*(umem_reg[i].addl));
	  for(uint32_t j=0;j<data_len;j++){
	    data[ndata++] = __bswap_32(umem_dat[i].data_buf[j]);
	  }
	  *(umem_reg[i].clr)  = __bswap_16(0x1); // clear
	  *(umem_reg[i].mode) = __bswap_16(0x1); // gate open
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, umem_dat[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}//for(i)
      }
      
      VME_MASTER_HEADER vme_master_header;
      init_vme_master_header( &vme_master_header, ndata, module_num );
      memcpy( &data[0], &vme_master_header, VME_MASTER_HSIZE*4 );
      
      len = ndata;

      switch(spill_flag){
      case k_spill_on:
	*(rpv130[0].pulse) = __bswap_16(0x01);// spill on busy off
	return 0;
      case k_spill_off:
      	write_spill_off(off_data_file, data, len,
      			nodeprop.getRunNumber(), off_event_number++);
      	*(rpv130[0].pulse) = __bswap_16(0x02);// spill off busy off
      	return -1;// return to wait_device()
      default:
	sprintf(message, "vme06: spill_flag is wrong: %d", spill_flag);
	send_error_message(message);
	return -1;
      }
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

