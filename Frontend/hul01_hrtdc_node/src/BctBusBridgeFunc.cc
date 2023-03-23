#include"BctBusBridgeFunc.hh"
#include"RegisterMap.hh"

using namespace HUL::HRTDC_BASE;

// WriteModuleIn2ndryFPGA ______________________________________
void
WriteModuleIn2ndryFPGA( HUL::FPGAModule& fmodule, uint32_t base_addr_prim,
			uint32_t addr_second,  uint32_t wd,  
			int32_t n_byte        
			)
{
  for(int32_t i = 0; i<n_byte; ++i){
    uint32_t bct_wd = (wd >> 8*i) & 0xff;
    bct_wd += ((addr_second +i ) << kShiftAddr);
    bct_wd += (kComWrite         << kShiftCom);

    fmodule.WriteModule(base_addr_prim + BBP::kAddrTxd, bct_wd, kNByte);
    fmodule.WriteModule(base_addr_prim + BBP::kAddrExec, 0);
  }
}

// ReadModuleIn2ndryFPGA _______________________________________
uint32_t
ReadModuleIn2ndryFPGA(HUL::FPGAModule& fmodule, uint32_t base_addr_prim,
		      uint32_t addr_second, int32_t  n_byte
		      )
{
  uint32_t mif_rd = 0;
  for(int32_t i = 0; i<n_byte; ++i){
    uint32_t bct_wd = 0;
    bct_wd += ((addr_second +i ) << kShiftAddr);
    bct_wd += (kComRead          << kShiftCom);

    fmodule.WriteModule(base_addr_prim + BBP::kAddrTxd, bct_wd, kNByte);
    fmodule.WriteModule(base_addr_prim + BBP::kAddrExec, 0);

    
    uint32_t rd = fmodule.ReadModule(base_addr_prim + BBP::kAddrRxd, 1);
    mif_rd += (rd << 8*i);
  }// for(i)

  return mif_rd;
}
