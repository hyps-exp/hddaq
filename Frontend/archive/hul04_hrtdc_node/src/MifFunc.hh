#ifndef MIFFUNC_HH
#define MIFFUNC_HH

#include<stdint.h>

#include"FPGAModule.hh"

static const int32_t kNByte = 3;

// Base BCT register
static const int32_t kShiftMifAddr = 8;
static const int32_t kShiftMifMid  = 16;

// MZN BCT register
static const int32_t kMaskMznAddr = 0xff;
static const int32_t kMaskMznMid  = 0xf00;
static const int32_t kShiftMznMid = 8;


void
WriteMIFModule(HUL::FPGAModule& fmodule, uint32_t addr_base,
	       uint32_t addr_mif, uint32_t wd,
	       int n_cycle
	       );

uint32_t 
ReadMIFModule(HUL::FPGAModule& fmodule, uint32_t addr_base,
	      uint32_t addr_mif,
	      int n_cycle
	      );

#endif
