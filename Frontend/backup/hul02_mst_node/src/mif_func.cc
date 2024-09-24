#include"mif_func.hh"

using namespace HUL_MsT;

void
WriteMIFModule(FPGAModule& fModule, unsigned int mid_base,
	       unsigned int mid_mif, unsigned int addr_mif, unsigned int wd,
	       int n_cycle
	       )
{
  for(int i = 0; i<n_cycle; ++i){
    unsigned int bct_wd = (wd >> 8*i) & 0xff;
    bct_wd += ((addr_mif +i ) << shift_mif_addr);
    bct_wd += (mid_mif        << shift_mif_mid);

    fModule.WriteModule(mid_base, MIF::laddr_reg_mif, bct_wd);
    fModule.WriteModule(mid_base, MIF::laddr_connect, 0);
  }
}

unsigned int 
ReadMIFModule(FPGAModule& fModule, unsigned int mid_base,
	      unsigned int mid_mif, unsigned int addr_mif, int n_cycle
	      )
{
  unsigned int mif_rd = 0;
  for(int i = 0; i<n_cycle; ++i){
    unsigned int bct_wd = 0;
    bct_wd += ((addr_mif + i) << shift_mif_addr);
    bct_wd += (mid_mif        << shift_mif_mid);

    fModule.WriteModule(mid_base, MIF::laddr_reg_mif, bct_wd);
    unsigned int rd = fModule.ReadModule(mid_base, MIF::laddr_connect, 1);
    mif_rd += (rd << 8*i);
  }// for(i)

  return mif_rd;
}
