#ifndef MIFFUNC_
#define MIFFUNC_

#include"FPGAModule.hh"
#include"RegisterMap.hh"
#include"rbcp.h"
#include"network.hh"

#include<iostream>

static const int shift_mif_addr = 8;
static const int shift_mif_mid  = 16;

void
WriteMIFModule(FPGAModule& fModule, unsigned int mid_base,
	       unsigned int mid_mif, unsigned int addr_mif, unsigned int wd,
	       int n_cycle
	       );

unsigned int 
ReadMIFModule(FPGAModule& fModule, unsigned int mid_base,
	      unsigned int mid_mif, unsigned int addr_mif, int n_cycle);

#endif
