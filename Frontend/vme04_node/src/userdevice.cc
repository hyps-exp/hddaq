// vme04_node: userdevice.cc

#include "userdevice.h"
#include "vme_xvb.h"

#define DMA_V792   1
#define DMA_TDC64M 0

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
  ////////// V792
  // uint32_t overflow_suppression = 1; // 0:enable 1:disable
  // uint32_t zero_suppression     = 1; // 0:enable 1:disable
  // int iped[] = { 200, 255, 255, 255 };
  // //int iped[] = { 155, 200, 220, 220 };
  // for(int i=0;i<V792_NUM;i++){
  //   *(v792[i].bitset1) = __bswap_16(0x80);
  //   *(v792[i].bitclr1) = __bswap_16(0x80);
  //   *(v792[i].bitset2) = __bswap_16( (overflow_suppression&0x1)<<3 |
  // 				     (zero_suppression&0x1)<<4 );
  //   *(v792[i].iped)    = __bswap_16(iped[i]); // 0x0-0xff
  // }
  ////////// TDC64M
  uint32_t reset         = 1; // clear local event counter
  uint32_t dynamic_range = 1; // 0-7, 2^n[us]
  uint32_t edge_mode     = 0; // 0:leading 1:leading&trailing
  uint32_t module_id     = 0; // 5bit, 0-511
  uint32_t search_window = 2000/8; // 2us
  //uint32_t search_window = 0x3E80; // 8ns unit, 0x0-0x3E80(0-128us)
  uint32_t mask_window   = 0x0; // 8ns unit, 0x0-0x3E80(0-128us)
  for(int i=0;i<TDC64M_NUM;i++){
    module_id = i;
    *(tdc64m[i].ctr) = __bswap_32( (reset&0x1 ) |
				   ((dynamic_range&0x7)<<1) |
				   ((edge_mode&0x1)<<4) |
				   ((module_id&0x1f)<<5) );
    *(tdc64m[i].enable1) = __bswap_32(0xffffffff);
    *(tdc64m[i].enable2) = __bswap_32(0xffffffff);
    *(tdc64m[i].window)  = __bswap_32( (search_window&0xffff) |
				       ((mask_window&0xffff)<<16) );
    *(tdc64m[i].str) = 0;// tdc64m clear    
  }
  return;
}

void init_device(NodeProp& nodeprop)
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      *(rpv130[0].csr1)  = __bswap_16(0x01); // io clear
      *(rpv130[0].pulse) = __bswap_16(0x01); // busy off
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
  vme_close();
  return;
}

int wait_device(NodeProp& nodeprop)
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
      for(int i=0;i<max_polling;i++){
	reg = __bswap_16(*(rpv130[0].rsff));
	if( (reg>>0)&0x1 ){
	  *(rpv130[0].csr1)  = __bswap_16(0x01); // io clear
	  return 0;
	}
      }
      // TimeOut
      std::cout<<"wait_device() Time Out"<<std::endl;
      //send_warning_message("vme04: wait_device() Time Out");
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
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      int ndata      = 0;
      int module_num = 0;
      ndata += VME_MASTER_HSIZE;
      ////////// VME_RM
      {
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
      
      ////////// TDC64M
      {
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
#if DMA_TDC64M // DmaRead ... not supported yet
	    if(data_len>DMA_BUF_LEN){
	      sprintf(message, "vme04: TDC64M[%08llx] data_len is too much -- %d/%d",
		      tdc64m[i].addr, data_len, DMA_BUF_LEN);
	      send_error_message(message);
	      data_len = DMA_BUF_LEN;
	    }
	    int status = gefVmeReadDmaBuf(dma_hdl, &tdc64m[i].addr_param, 0x4000, 4*data_len);
	    if(status!=0){
	      sprintf(message, "vme04: TDC64M[%08llx] gefVmeReadDmaBuf() failed -- %d",
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
	  }else{
	    sprintf(message, "vme04: TDC64M[%08llx] data is not ready", tdc64m[i].addr );
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

      ////////// V792
      //       {
      // 	for(int i=0;i<V792_NUM;i++){
      // 	  int vme_module_header_start = ndata;
      // 	  ndata += VME_MODULE_HSIZE;
      // 	  int data_len = 34;
      // 	  int dready   = 0;
      // 	  for(int j=0;j<max_try;j++){
      // 	    dready = __bswap_16(*(v792[i].str1))&0x1;
      // 	    if(dready==1) break;
      // 	  }
      // 	  if(dready==1){
      // #if DMA_V792
      // 	    int status = gefVmeReadDmaBuf(dma_hdl, &v792[i].addr_param, 0, 4*data_len);
      // 	    if(status!=0){
      // 	      sprintf(message, "vme04: V792[%08llx] gefVmeReadDmaBuf() failed -- %d",
      // 		      v792[i].addr, GEF_GET_ERROR(status));
      // 	      send_error_message(message);
      // 	    }else{
      // 	      for(int j=0;j<data_len;j++){
      // 		data[ndata++] = __bswap_32(dma_buf[j]);
      // 	      }
      // 	    }
      // #else
      // 	  for(int j=0;j<data_len;j++){
      // 	    data[ndata++] = __bswap_32(*(v792[i].addr+j));
      // 	  }
      // #endif
      // 	  }else{
      // 	    sprintf(message, "vme04: V792[%08llx] data is not ready", v792[i].addr );
      // 	    send_warning_message(message);
      // 	  }
      // 	  VME_MODULE_HEADER vme_module_header;
      // 	  init_vme_module_header( &vme_module_header,v792[i].addr,
      // 				  ndata - vme_module_header_start );
      // 	  memcpy( &data[vme_module_header_start],
      // 		  &vme_module_header, VME_MODULE_HSIZE*4 );
      // 	  module_num++;
      // 	}//for(i)
      //       }
      
      VME_MASTER_HEADER vme_master_header;
      init_vme_master_header( &vme_master_header, ndata, module_num );
      memcpy( &data[0], &vme_master_header, VME_MASTER_HSIZE*4 );
      
      len = ndata;
      *(rpv130[0].pulse) = __bswap_16(0x01); // busy off
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
