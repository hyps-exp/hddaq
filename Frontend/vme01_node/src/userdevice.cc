// vme01_node: userdevice.cc

#include "userdevice.h"
#include "vme_v7807.h"

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
  ////////// SMP
  for(int i=0;i<SMP_NUM;i++){
    *(smp[i].cmr)   = 0x40;// SMP reset
    usleep(10000);
    *(smp[i].snccr) = 0x02;// snc clear
    *(smp[i].bcr)   = 0x23;// hardware switch enable
    *(smp[i].cmr)   = 0x00;// set cmr
  }
  return;
}

void init_device(NodeProp& nodeprop)
{
  g_daq_mode = nodeprop.getDaqMode();
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      for(int i=0;i<SMP_NUM;i++){
	*(smp[i].snccr) = 0x02; // snc clear
      }
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
  vme_close();
  return;
}


int wait_device(NodeProp& nodeprop)
/*
  return -1: TIMEOUT or FAST CLEAR -> continue
  return  0: TRIGGED -> go read_device
*/
{
  //char message[256];
  switch(g_daq_mode){
  case DM_NORMAL:
    {
      ////////// Polling
      int reg = 0;
      for(int i=0;i<max_polling;i++){
	reg = *(rpv130[0].rsff);
	if( (reg>>0)&0x1 ){
#if 0
	  for(int j=0;j<SMP_NUM;j++){
	    int seq_busy;
	    do {
	      uint32_t buf32 = *(smp[j].cmr);
	      seq_busy = (buf32>>5)&0x1;
	    }while(seq_busy==1);
	  }		
#endif
	  *(rpv130[0].pulse) = 0xfc; // SMP switch
	  *(rpv130[0].csr1)  = 0x1;  // io clear
	  *(rpv130[0].pulse) = 0x1;  // busy off
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
      ////////// SMP
      {
	for(int i=0;i<SMP_NUM;i++){
	  int vme_module_header_start = ndata;
	  ndata += VME_MODULE_HSIZE;
	  int evnum = (*(smp[i].enr)) & 0xffff;
	  int dsize = (*(smp[i].dsr)) & 0x7fffffff;
	  int data_len = dsize/4;
	  if(evnum!=1||data_len<=0){
	    sprintf(message, "vme01: SMP[%08llx] invalid data!!! (event:%d, data_len:%d)",
		    smp[i].addr, evnum, data_len);
	    send_fatal_message(message);
	    std::exit(EXIT_FAILURE);
	  }else{
	    if(data_len>DMA_BUF_LEN){
	      sprintf(message, "vme01: SMP[%08llx] data_len is too much: %d/%d",
		      smp[i].addr, data_len, DMA_BUF_LEN);
	      send_warning_message(message);
	    }else{
	      int status = vme_dma_read( bus_hdl, dma_hdl, 0, smp[i].addr,
					 SMP_AM, 4*data_len, 0 );
	      if(status==0){
		data[ndata++] = 0x0;  //compatibility for privious data format
		memcpy( &data[ndata], dma_buf, 4*data_len );
		ndata += data_len;
	      }else{
		sprintf(message, "vme01: SMP[%08llx] vme_dma_read() failed", smp[i].addr);
		send_warning_message(message);
	      }
	    }
	  }
	  VME_MODULE_HEADER vme_module_header;
	  init_vme_module_header( &vme_module_header, smp[i].addr,
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
