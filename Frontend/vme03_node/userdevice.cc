// vme03_node: userdevice.cc

#include "userdevice.h"
#include "vme_v7807.h"

#define DMA_V830 0 // not supported

static const int max_polling   = 2000000;     //maximum count until time-out
static const int max_try       = 100;         //maximum count to check data ready
static const int max_data_size = 4*1024*1024; //maximum datasize by byte unit

int get_maxdatasize()
{
  return max_data_size;
}

int open_device()
{
  vme_open();

  ////////// V830
  for(int i=0;i<V830_NUM;i++){
    *(v830[i].reset)  = 0x0;
    *(v830[i].enable) = 0x0;  // for MEB, v830 only
    *(v830[i].clr)    = 0x01; // [acq mode] 00: disabled (default)
                              //            01: external or from VME
                              //            10: periodical
  }
  ////////// V775
  int range = 0xff;     // 0x18-0xff, range: 1200-140[ns]
  int common_input = 0; // 0:common start, 1:common stop
  int empty_prog   = 1; // 0: if data is empty, no header and footer
                        // 1: add header and footer always
  for(int i=0;i<V775_NUM;i++){
    *(v775[i].bitset1) = 0x80;
    *(v775[i].bitclr1) = 0x80;
    *(v775[i].range)   = range;
    *(v775[i].bitset2) = ( empty_prog<<12 ) | ( common_input<<10 );
  }
  
  return 0;
}

int init_device(DaqMode daq_mode)
{
  switch(daq_mode){
  case DM_NORMAL:
    {
      *(rpv130[0].csr1)  = 0x1; // clear
      *(rpv130[0].pulse) = 0x1; // busy off
      return 0;
    }
  case DM_DUMMY:
    {
      return 0;
    }
  default:
    return 0;
  }
}

int finalize_device(DaqMode daq_mode)
{
  return 0;
}

int close_device()
{
  vme_close();
  return 0;
}


int wait_device(DaqMode daq_mode)
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  switch(daq_mode){
  case DM_NORMAL:
    {
      ////////// Polling
      int reg = 0;
      for(int i=0;i<max_polling;i++){
	reg = *(rpv130[0].rsff);
	if( (reg>>0)&0x1 ){
	  *(rpv130[0].csr1)  = 0x1; // clear
	  return 0;
	}
      }
      // TimeOut
      send_warning("vme03: wait_device() Time Out");
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

int read_device(DaqMode daq_mode, unsigned int* data, int& len)
/*
  return -1: Do Not Send data to EV
  return  0: Send data to EV
*/
{
  char pbuf[256];
  switch(daq_mode){
  case DM_NORMAL:
    {
      int ndata  = 0;
      int module_num = 0;
      ndata += VME_MASTER_HSIZE;
      ////////// VME_RM
      {
	for(int i=0;i<VME_RM_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  data[ndata++] = *(vme_rm[i].event);
	  data[ndata++] = *(vme_rm[i].spill);
	  data[ndata++] = *(vme_rm[i].serial);
	  data[ndata++] = 0x0; // spill_end_flag
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, vme_rm[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
      }
      ////////// v830
      {
	for(int i=0;i<V830_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 32;
#if DMA_V830
	  int status = vme_dma_read( bus_hdl, dma_hdl, 0, v830[i].addr+0x1000,
				     V830_AM, 4*data_len, 0 );
	  if(status!=0){
	    sprintf(pbuf, "vme03: V830[%08llx] vme_dma_read() failed", v830[i].addr);
	    send_error(pbuf);
	  }else{
	    memcpy( &data[ndata], dma_buf, 4*data_len );
	    ndata += data_len;
	  }
#else
	  for(int j=0; j<data_len; j++){
	    data[ndata++] = *(v830[i].counter[j]);
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
      ////////// v775
      {
	for(int i=0;i<V775_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 34;
	  for(int k=0;k<data_len;k++){
	    uint32_t data_buf = *(v775[i].data_buf);
	    data[ndata++] = data_buf;
	    int data_type = ( data_buf >> 24 ) & 0x7; // 2:header, 0:data, 4:footer
	    if(data_type==4)  break;
	    if(k+1==data_len && data_type!=4){
	      sprintf(pbuf, "vme03: V775[%08llx] nooooo fooooter!!!", v775[i].addr);
	      send_error(pbuf);
	    }
	  }
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, v775[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
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
