// vme03_node: userdevice.cc

#include "userdevice.h"
#include "vme_xvb.h"

#define DMA_CHAIN 0
#define DMA_V830  0

static bool g_spill_end   = false;

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
  ////////// V830
  uint16_t geo_addr[]  = { 0x2, 0x4 };
  uint16_t chain_set[] = { 0x2, 0x1 };
  for( int i=0; i<V830_NUM; ++i ){
    *(v830[i].geo_addr)   = __bswap_16( geo_addr[i] );
    *(v830[i].reset)      = __bswap_16( 0x0 );
    *(v830[i].chain_addr) = __bswap_16( 0xAA );
    *(v830[i].chain_ctrl) = __bswap_16( chain_set[i] );
    *(v830[i].enable)     = __bswap_32( 0x0 );// for MEB, v830 only
    // *(v830[i].enable)     = __bswap_32( 0xffffffff );// for MEB, v830 only
    ///// acq mode
    // 00: disabled (default)
    // 01: external or from VME
    // 10: periodical
    uint16_t acq_mode = 0x01;
    *(v830[i].clr)    = __bswap_16(acq_mode);
  }

  return;
}

void init_device(NodeProp& nodeprop)
{
  g_spill_end   = false;
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      *(rpv130[0].csr1)  = __bswap_16(0x1); // io clear
      *(rpv130[0].pulse) = __bswap_16(0x1); // busy off
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
	if( (reg>>1)&0x1 ){
	  *(rpv130[0].csr1)  = __bswap_16(0x01); // io clear
	  g_spill_end = true;
	}
	if( (reg>>0)&0x1 ){
	  *(rpv130[0].csr1)  = __bswap_16(0x01); // io clear
	  return 0;
	}
      }
      // TimeOut
      std::cout<<"wait_device() Time Out"<<std::endl;
      //send_warning_message("vme03: wait_device() Time Out");
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
      const int event_number = nodeprop.getEventNumber();
      if( g_spill_end ){
	send_normal_message("vme03: spill end");
      }
      // if( !g_spill_end && event_number%10!=0 ){
      // 	len = 0;
      // 	*(rpv130[0].pulse) = __bswap_16(0x1); // busy off
      // 	g_spill_end = false;
      // 	return 0;
      // }

      int ndata  = 0;
      int module_num = 0;
      ndata += VME_MASTER_HSIZE;
      ////////// VME_RM
      // {
      // 	for(int i=0;i<VME_RM_NUM;i++){
      // 	  int vme_module_header_start = ndata;
      // 	  ndata += VME_MODULE_HSIZE;
      // 	  data[ndata++] = __bswap_32(*(vme_rm[i].event));
      // 	  data[ndata++] = __bswap_32(*(vme_rm[i].spill));
      // 	  data[ndata++] = __bswap_32(*(vme_rm[i].serial));
      // 	  data[ndata++] = __bswap_32(*(vme_rm[i].time));
      // 	  VME_MODULE_HEADER vme_module_header;
      // 	  init_vme_module_header( &vme_module_header, vme_rm[i].addr,
      // 				  ndata - vme_module_header_start );
      // 	  memcpy( &data[vme_module_header_start],
      // 		  &vme_module_header, VME_MODULE_HSIZE*4 );
      // 	  module_num++;
      // 	}
      // }

#if DMA_CHAIN
      int dready   = 0;
      for( int i=0; i<max_try; ++i ){
	dready = __bswap_16((*(v830[0].clr)))&0x1;
	if( dready==1 ) break;
      }
      if( dready==1 ){
	GEF_STATUS status = gefVmeReadDmaBuf( dma_hdl, &dma_addr, 0, 4*2*32 );
	if( status!=GEF_STATUS_SUCCESS ){
	  sprintf( message, "vme03: gefVmeReadDmaBuf() failed -- %d",
		   GEF_GET_ERROR( status ) );
	  send_error_message( message );
	}
	for( int i=0; i<DMA_BUF_LEN; ){
	  if( dma_buf[i]==0x0 || dma_buf[i]==0xffffffff ) break;
	  data[ndata++] = __bswap_32( dma_buf[i++] );
	  printf("vme03: V830[%08llx] %03d %08x \n", v830[i].addr, i, data[ndata-1] );
	}
      }else{
	send_warning_message("vme03: data is not ready");
      }
#else
      ////////// v830
      {
	for(int i=0;i<V830_NUM;i++){
	  if( i==1 ) continue;
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 32;
# if DMA_V830
	  int dready   = 0;
	  for(int j=0;j<max_try;j++){
	    dready = __bswap_16((*(v830[i].clr)))&0x1;
	    if(dready==1) break;
	  }
	  if(dready==1){
	    int status = gefVmeReadDmaBuf( dma_hdl, &v830[i].addr_param, 0, 4*data_len );
	    if(status!=0){
	      sprintf(message, "vme03: V830[%08llx] gefVmeReadDmaBuf() failed -- %d",
		      v830[i].addr, GEF_GET_ERROR(status));
	      send_error_message(message);
	    }else{
	      for(int j=0;j<data_len;j++){
		data[ndata++] = __bswap_32(dma_buf[j]);
		printf("vme03: V830[%08llx] %03d %08x \n", v830[i].addr, j, data[ndata-1] );
	      }
	    }
	  }else{
	    sprintf(message, "vme03: V830[%08llx] data is not ready", v830[i].addr);
	    send_warning_message(message);
	  }
# else
	  for(int j=0; j<data_len; j++){
	    data[ndata++] = __bswap_32(*(v830[i].counter[j]));
	  }
# endif
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, v830[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
      }
#endif

      VME_MASTER_HEADER vme_master_header;
      init_vme_master_header( &vme_master_header, ndata, module_num );
      memcpy( &data[0], &vme_master_header, VME_MASTER_HSIZE*4 );
      
      len = ndata;
      *(rpv130[0].pulse) = __bswap_16(0x1); // busy off
      g_spill_end = false;
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
