// vme06_node: spill_off.cc

#include "userdevice.h"
#include "spill_off.h"

void write_spill_off(gzFile gzfile, unsigned int* data, int& len,
		     int run_number, int event_number)
{
  time_t t = time(0);
  
  unsigned int eb_node_header[FE_NODE_HSIZE];
  eb_node_header[k_magic]        = FE_NODE_MAGIC;
  eb_node_header[k_data_size]    = len + FE_NODE_HSIZE*2;
  eb_node_header[k_event_number] = event_number;
  eb_node_header[k_run_number]   = run_number;
  eb_node_header[k_node_id]      = EB_NODE_ID;
  eb_node_header[k_event_type]   = 0;
  eb_node_header[k_nblocks]      = 1;
  eb_node_header[k_unix_time]    = (unsigned int)t;
  gzwrite(gzfile, (char*)eb_node_header, FE_NODE_HSIZE*sizeof(unsigned int));
  
  unsigned int vme06_node_header[FE_NODE_HSIZE];
  vme06_node_header[k_magic]        = FE_NODE_MAGIC;
  vme06_node_header[k_data_size]    = len + FE_NODE_HSIZE;
  vme06_node_header[k_event_number] = event_number;
  vme06_node_header[k_run_number]   = run_number;
  vme06_node_header[k_node_id]      = VME06_NODE_ID;
  vme06_node_header[k_event_type]   = 0;
  vme06_node_header[k_nblocks]      = 0;
  vme06_node_header[k_unix_time]    = (unsigned int)t;
  gzwrite(gzfile, (char*)vme06_node_header, FE_NODE_HSIZE*sizeof(unsigned int));

  gzwrite(gzfile, (char*)data, len*sizeof(unsigned int));
}
