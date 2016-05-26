// vme01_node: userdevice.cc

#include "userdevice.h"
#include "vme_xvb.h"

#define DMA_CHAIN 1
#define DMA_V792  1 // if DMA_CHAIN 0

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
  {
    int      geo_addr[]  = { 0x2, 0x4, 0x6, 0x8 };
    int      chain_set[] = { 0x2, 0x3, 0x3, 0x3 };
    uint32_t overflow_suppression = 1; // 0:enable 1:disable
    uint32_t zero_suppression     = 1; // 0:enable 1:disable
    int      iped[] = { 255, 255, 255, 255 }; // 0x0-0xff
    for(int i=0;i<V792_NUM;i++){
      *(v792[i].geo_addr)   = __bswap_16( geo_addr[i] );
      *(v792[i].bitset1)    = __bswap_16( 0x80 );
      *(v792[i].bitclr1)    = __bswap_16( 0x80 );
      *(v792[i].chain_addr) = __bswap_16( 0xAA );
      *(v792[i].chain_ctrl) = __bswap_16( chain_set[i] );
      *(v792[i].bitset2)    = __bswap_16( (overflow_suppression&0x1)<<3 |
					  (zero_suppression&0x1)<<4 );
      *(v792[i].iped)       = __bswap_16( iped[i] );
    }
  }
  ////////// V775
  {
    int      geo_addr[]  = { 0xa, 0xc, 0xe, 0x10, 0x12 };
    int      chain_set[] = { 0x3, 0x3, 0x3, 0x3, 0x1 };
    uint16_t range = 0xff;     // 0x18-0xff, range: 1200-140[ns]
    uint16_t common_input = 0; // 0:common start, 1:common stop
    uint16_t empty_prog   = 1; // 0: if data is empty, no header and footer
                               // 1: add header and footer always
    for(int i=0;i<V775_NUM;i++){
      *(v775[i].geo_addr)   = __bswap_16( geo_addr[i] );
      *(v775[i].bitset1)    = __bswap_16( 0x80 );
      *(v775[i].bitclr1)    = __bswap_16( 0x80 );
      *(v775[i].chain_addr) = __bswap_16( 0xAA );
      *(v775[i].chain_ctrl) = __bswap_16( chain_set[i] );
      *(v775[i].range)      = __bswap_16( range );
      *(v775[i].bitset2)    = __bswap_16( ( empty_prog  <<12 ) |
					  ( common_input<<10 ) );
    }
  }

  return;
}

void init_device(NodeProp& nodeprop)
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      *(vme_rm[0].reset) = __bswap_32(0x1); // input clear
      *(vme_rm[0].pulse) = __bswap_32(0x1); // busy off
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
	reg = __bswap_32(*(vme_rm[0].input));
	if( (reg>>8)&0x1 == 0x1 ){
	  *(vme_rm[0].reset) = __bswap_32(0x1); // input clear
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
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
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
	  data[ndata++] = __bswap_32(*(vme_rm[i].event));
	  data[ndata++] = __bswap_32(*(vme_rm[i].spill));
	  data[ndata++] = __bswap_32(*(vme_rm[i].serial));
	  data[ndata++] = __bswap_32(*(vme_rm[i].time));
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, vme_rm[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}
      }

#if DMA_CHAIN
      int dready = 0;
      for(int j=0;j<max_try;j++){
	dready = __bswap_16(*(v792[0].str1))&0x1;
	if(dready==1) break;
      }

      GEF_STATUS status = gefVmeReadDmaBuf( dma_hdl, &dma_addr, 0, 4*9*34 );
      if( status!=GEF_STATUS_SUCCESS ){
      	sprintf(message, "vme01: gefVmeReadDmaBuf() failed -- %d",
      		GEF_GET_ERROR(status ) );
      	send_fatal_message(message);      
      	std::exit(EXIT_FAILURE);
      }

      for( int i=0; i<DMA_BUF_LEN; ){
      	if( dma_buf[i]==0x0 ||
	    dma_buf[i]==0xffffffff ){
	  break;
	}
	uint64_t vme_addr;
      	int vme_module_header_start = ndata;
      	ndata += VME_MODULE_HSIZE;
	// header
	uint32_t buf = __bswap_32( dma_buf[i++] );
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
	  sprintf( message, "vme01: unknown GEO_ADDRESS %d", geo_addr );
	  send_fatal_message( message );
	  std::exit( EXIT_FAILURE );
	}

	data[ndata++] = buf;
      	for( int j=0; j<ncount+1; ++j ){
      	  data[ndata++] = __bswap_32( dma_buf[i++] );
      	}
	VME_MODULE_HEADER vme_module_header;
	init_vme_module_header( &vme_module_header, vme_addr,
				ndata - vme_module_header_start );
	memcpy( &data[vme_module_header_start],
		&vme_module_header, VME_MODULE_HSIZE*4 );
	module_num++;
      }
#else
      ////////// V792
      {
	for(int i=0;i<V792_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 34;
	  int dready   = 0;
	  for(int j=0;j<max_try;j++){
	    dready = __bswap_16(*(v792[i].str1))&0x1;
	    if(dready==1) break;
	  }
	  if(dready==1){
# if DMA_V792
	    int status = gefVmeReadDmaBuf(dma_hdl, &v792[i].addr_param, 0, 4*data_len);
	    if(status!=0){
	      sprintf(message, "vme01: V792[%08llx] gefVmeReadDmaBuf() failed -- %d",
		      v792[i].addr, GEF_GET_ERROR(status));
	      send_error_message(message);
	    }else{
	      for(int j=0;j<data_len;j++){
		data[ndata++] = __bswap_32(dma_buf[j]);
	      }
	    }
# else
	    for(int j=0;j<data_len;j++){
	      data[ndata++] = __bswap_32(*(v792[i].addr+j));
	    }
# endif
	  }else{
	    sprintf(message, "vme01: V792[%08llx] data is not ready", v792[i].addr );
	    send_warning_message(message);
	  }
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header,v792[i].addr,
				  ndata - vme_module_header_start );
	  memcpy( &data[vme_module_header_start],
		  &vme_module_header, VME_MODULE_HSIZE*4 );
	  module_num++;
	}//for(i)
      }
      ////////// v775
      {
	for(int i=0;i<V775_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int data_len = 34;
	  int dready   = 0;
          for(int j=0;j<max_try;j++){
            dready = __bswap_16(*(v775[i].str1))&0x1;
            if(dready==1) break;
          }
	  if(dready==1){
	    for(int k=0;k<data_len;k++){
	      uint32_t data_buf = __bswap_32(*(v775[i].data_buf));
	      data[ndata++] = data_buf;
	      int data_type = (data_buf>>24)&0x7; // 2:header, 0:data, 4:footer
	      if(data_type==4)  break;
	      if(k+1==data_len && data_type!=4){
		sprintf(message, "vme01: V775[%08llx] nooooo fooooter!!!", v775[i].addr);
		send_warning_message(message);
	      }
	    }
	  }else{
	    sprintf(message, "vme01: V775[%08llx] data is not ready", v775[i].addr );
            send_warning_message(message);
	  }
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, v775[i].addr,
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
      *(vme_rm[0].pulse) = __bswap_32(0x1); // busy off
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
