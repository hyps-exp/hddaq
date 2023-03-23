#ifndef BUSBRIDGEFUNC_HH
#define BUSBRIDGEFUNC_HH

#include<stdint.h>

#include"FPGAModule.hh"

// Bus Brige Frame structure
// | 8-bit Command | 16-bit address | 8-bit data |
const int32_t kNByte = 4;
// Commands
const uint32_t kComWrite = 0x10;
const uint32_t kComRead  = 0x20;
const uint32_t kComAck   = 0x30;

const uint32_t kShiftAddr = 8;
const uint32_t kShiftCom  = 24;

// WriteModuleIn2ndryFPGA ______________________________________
void
WriteModuleIn2ndryFPGA
(
 // Primary FPGA -----------------------------------------------
 HUL::FPGAModule& fmodule,
 uint32_t base_addr_prim, // Base address (module ID) for BusBridgePrimary
 
 // Secondary FPGA ---------------------------------------------
 uint32_t addr_second, // BCT Address in secondary FPGA
 uint32_t wd,          // BCT data to be written to the module in secondary FPGA
 int32_t n_byte        // Set 1-4 bytes
 );


// ReadModuleIn2ndryFPGA _______________________________________
// Retuned values is the read register value
uint32_t
ReadModuleIn2ndryFPGA
(
 // Primary FPGA ------------------------------------------------
 HUL::FPGAModule& fmodule,
 uint32_t base_addr_prim, // Base address (module ID) for BusBridgePrimary

  // Secondary FPGA ---------------------------------------------
 uint32_t addr_second, // BCT Address in secondary FPGA
 int32_t  n_byte       // Set 1-4 bytes
 );

#endif
