#ifndef REGISTER_MAP_HH
#define REGISTER_MAP_HH

namespace HUL{
namespace Reset{
    
//------------------------------------------------------------------------------
// MTM Module
//------------------------------------------------------------------------------
namespace TRM{
  enum LocalAddress
    {
      kAddrSelectTrigger = 0x00000000 // R/W [11:0] select trigger line
    };
  enum command_sel_trig
    {
      kRegL1Ext  = 0x1,
      kRegL1J0   = 0x2,
      kRegL1RM   = 0x4,
      kRegL2Ext  = 0x8,
      kRegL2J0   = 0x10,
      kRegL2RM   = 0x20,
      kRegClrExt = 0x40,
      kRegClrJ0  = 0x80,
      kRegClrRM  = 0x100,
      kRegEnL2   = 0x200,
      kRegEnJ0   = 0x400,
      kRegEnRM   = 0x800
    };
};

//------------------------------------------------------------------------------
// DCT Module
//------------------------------------------------------------------------------
namespace DCT{
  enum LocalAddress
    {
      kAddrDaqGate  = 0x10000000, // R/W [0:0] Set DAQ Gate reg
      kAddrResetEvb = 0x10100000  // -/W Assert EVB reset (self counter reset)
    };
};

//------------------------------------------------------------------------------
// RTR Module
//------------------------------------------------------------------------------
namespace RTR{
  enum LocalAddress
    {
      kAddrCounterReset  = 0x20000000, // -/W [0:0] Assert counter reset
      kAddrEnableBlock   = 0x20100000,  // R/W [3:0] Set block enable
      kAddrEnableHdrst   = 0x20200000  // R/W [3:0] Set block enable
    };
};

//------------------------------------------------------------------------------
// IOM Module
//------------------------------------------------------------------------------
namespace IOM{
  enum LocalAddress
    {
      kAddrNimout1      = 0x30000000, // R/W [3:0]
      kAddrNimout2      = 0x30100000, // R/W [3:0]
      kAddrNimout3      = 0x30200000, // R/W [3:0]
      kAddrNimout4      = 0x30300000, // R/W [3:0]
      kAddrExtL1        = 0x30400000, // R/W [2:0]
      kAddrExtL2        = 0x30500000, // R/W [2:0]
      kAddrExtClr       = 0x30600000, // R/W [2:0]
      kAddrExtSpillGate = 0x30700000, // R/W [2:0]
      kAddrExtCCRst     = 0x30800000, // R/W [2:0]
      kAddrExtBusy      = 0x30900000, // R/W [2:0]
      kAddrExtRsv2      = 0x30A00000  // R/W [2:0]
    };
  enum OutputSubbAddress
    {
      kReg_o_ModuleBusy = 0x0,
      kReg_o_CrateBusy  = 0x1,
      kReg_o_RML1       = 0x2,
      kReg_o_RML2       = 0x3,
      kReg_o_RMClr      = 0x4,
      kReg_o_RMRsv1     = 0x5,
      kReg_o_RMSnInc    = 0x6,
      kReg_o_DaqGate    = 0x7,
      kReg_o_DIP8       = 0x8,
      kReg_o_clk1MHz    = 0x9,
      kReg_o_clk100kHz  = 0xA,
      kReg_o_clk10kHz   = 0xB,
      kReg_o_clk1kHz    = 0xC
    };
  enum InputSubbAddress
    {
      kReg_i_Nimin1 = 0x0,
      kReg_i_Nimin2 = 0x1,
      kReg_i_Nimin3 = 0x2,
      kReg_i_Nimin4 = 0x3,
      kReg_i_default = 0x7
    };
};

//------------------------------------------------------------------------------
// Bus Controller
//------------------------------------------------------------------------------
/*
namespace BCT
{
  static const int mid = 0xe;
  enum LocalAddress
    {
      laddr_Reset   = 0x0, // -/W Assert module reset signal
      laddr_Version = 0x10,// R/- [31:0]
      laddr_ReConfig= 0x20 // -/W Reconfig FPGA by SPI
    };
}
*/
};
};

#endif
