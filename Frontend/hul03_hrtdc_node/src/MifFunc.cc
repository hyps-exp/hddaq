#include"MifFunc.hh"
#include"RegisterMap.hh"

using namespace HUL::HRTDC_BASE;

void
WriteMIFModule(HUL::FPGAModule& fmodule, uint32_t addr_base,
	       uint32_t addr_mif, uint32_t wd,
	       int n_cycle
	       )
{
  uint32_t mid_mzn  = (addr_mif & kMaskMznMid) >> kShiftMznMid;
  uint32_t addr_mzn = addr_mif & kMaskMznAddr;

  for(int32_t i = 0; i<n_cycle; ++i){
    uint32_t bct_wd = (wd >> 8*i) & 0xff;
    bct_wd += ((addr_mzn +i ) << kShiftMifAddr);
    bct_wd += (mid_mzn        << kShiftMifMid);

    fmodule.WriteModule(addr_base + MIF::kAddrReg, bct_wd, kNByte);
    fmodule.WriteModule(addr_base + MIF::kAddrConnect, 0);
  }
}

uint32_t 
ReadMIFModule(HUL::FPGAModule& fmodule, uint32_t addr_base,
	      uint32_t addr_mif, int n_cycle
	      )
{
  uint32_t mid_mzn  = (addr_mif & kMaskMznMid) >> kShiftMznMid;
  uint32_t addr_mzn = addr_mif & kMaskMznAddr;

  uint32_t mif_rd = 0;
  for(int32_t i = 0; i<n_cycle; ++i){
    uint32_t bct_wd = 0;
    bct_wd += ((addr_mzn + i) << kShiftMifAddr);
    bct_wd += (mid_mzn        << kShiftMifMid);

    fmodule.WriteModule(addr_base + MIF::kAddrReg, bct_wd, kNByte);
    uint32_t rd = fmodule.ReadModule(addr_base + MIF::kAddrConnect, 1);
    mif_rd += (rd << 8*i);
  }// for(i)

  return mif_rd;
}
