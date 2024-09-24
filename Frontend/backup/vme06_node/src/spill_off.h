// vme06_node: spill_off.h
#ifndef SPILL_OFF_H
#define SPILL_OFF_H

#include <zlib.h>
#include <sys/stat.h>

void write_spill_off(gzFile gzfile, unsigned int* data, int& len,
		     int run_number, int event_number);

//static const std::string off_data_dir = "/misc/data3/subdata_jun/spill_off/bench";
static const std::string off_data_dir = "/misc/data3/subdata_jun/spill_off/data";

enum e_spill_flag
  {
    k_init,
    k_spill_on,
    k_spill_off
  };

#define FE_NODE_MAGIC 0x45564e54U
enum e_fe_node_header
  {
    k_magic,
    k_data_size,
    k_event_number,
    k_run_number,
    k_node_id,
    k_event_type,
    k_nblocks,
    k_unix_time,
    FE_NODE_HSIZE
  };

#define EB_NODE_ID    0x1
#define VME06_NODE_ID 0x206

#endif
