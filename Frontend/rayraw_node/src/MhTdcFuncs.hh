#ifndef MHTDC_FUNCS_HH
#define MHTDC_FUNCS_HH

#include<stdint.h>

#include"FPGAModule.hh"
#include"RegisterMap.hh"

namespace HUL::DAQ{

  void
  SetTdcWindow(uint32_t wmax, uint32_t wmin, HUL::FPGAModule& fpga_module)
  {
    using namespace LBUS;
  
    static const uint32_t kCMax       = 2047;
    static const uint32_t kPtrDiffWr = 2;

    uint32_t ptr_ofs  = kCMax - wmax + kPtrDiffWr;

    fpga_module.WriteModule(TDC::kAddrPtrOfs,  ptr_ofs, 2);
    fpga_module.WriteModule(TDC::kAddrWindowMax, wmax,  2);
    fpga_module.WriteModule(TDC::kAddrWindowMin, wmin,  2);

  }
};

#endif
