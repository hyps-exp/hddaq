#ifndef ADC_FUNCS_HH
#define ADC_FUNCS_HH

#include<stdint.h>

#include"FPGAModule.hh"
#include"RegisterMap.hh"

namespace HUL::DAQ{

  void
  SetAdcWindow(uint32_t wmax, uint32_t wmin, HUL::FPGAModule& fpga_module)
  {
    using namespace LBUS;
  
    static const uint32_t kCMax       = 2047;
    static const uint32_t kPtrDiffWr = 2;

    uint32_t ptr_ofs  = kCMax - wmax + kPtrDiffWr;

    fpga_module.WriteModule(ADC::kAddrPtrOfs,  ptr_ofs, 2);
    fpga_module.WriteModule(ADC::kAddrWindowMax, wmax,  2);
    fpga_module.WriteModule(ADC::kAddrWindowMin, wmin,  2);

  }
};

#endif
