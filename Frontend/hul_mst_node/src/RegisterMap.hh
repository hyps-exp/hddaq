#ifndef REGISTERH_
#define REGISTERH_

namespace HUL_MST{
//-------------------------------------------------------------------------
// TRM Module
//-------------------------------------------------------------------------
namespace TRM{
  static const int mid = 0x0;
  enum LocalAddress{
    laddr_sel_trig   = 0x000 // W/R, [7:0] select trigger line
  };

  enum command_sel_trig{
    reg_L1Ext = 0x1,
    reg_L1J0  = 0x2,
    reg_L1RM  = 0x4,
    reg_L2Ext = 0x8,
    reg_L2J0  = 0x10,
    reg_L2RM  = 0x20,
    reg_ClrExt= 0x40,
    reg_ClrJ0 = 0x80,
    reg_ClrRM = 0x100,
    reg_EnL2  = 0x200,
    reg_EnJ0  = 0x400,
    reg_EnRM  = 0x800
  };
};

//-------------------------------------------------------------------------
// DCT Module
//-------------------------------------------------------------------------
namespace DCT{
  static const int mid = 0x1;
  enum LocalAddress{
    laddr_gate       = 0x000, // W/R, [0:0] Set DAQ Gate reg
    laddr_evb_reset  = 0x010  // W,   Assert EVB reset (self counter reset)
  };
};

//-------------------------------------------------------------------------
// IOM Module
//-------------------------------------------------------------------------
namespace IOM{
  static const int mid = 0x2;
  enum LocalAddress{
    laddr_nimout1     = 0x000, // W/R, [3:0]
    laddr_nimout2     = 0x010, // W/R, [3:0]
    laddr_nimout3     = 0x020, // W/R, [3:0]
    laddr_nimout4     = 0x030, // W/R, [3:0]
    laddr_extL1       = 0x040, // W/R, [2:0]
    laddr_extL2       = 0x050, // W/R, [2:0]
    laddr_extClr      = 0x060, // W/R, [2:0]
    laddr_extBusy     = 0x070, // W/R, [2:0]
    laddr_extRsv2     = 0x080  // W/R, [2:0]
  };

  enum OutputSubbAddress{
    reg_o_ModuleBusy = 0x0,
    reg_o_CrateBusy  = 0x1,
    reg_o_RML1       = 0x2,
    reg_o_RML2       = 0x3,
    reg_o_RMClr      = 0x4,
    reg_o_RMRsv1     = 0x5,
    reg_o_RMSnInc    = 0x6,
    reg_o_DaqGate    = 0x7,
    reg_o_DIP8       = 0x8,
    reg_o_clk1MHz    = 0x9,
    reg_o_clk100kHz  = 0xA,
    reg_o_clk10kHz   = 0xB,
    reg_o_clk1kHz    = 0xC,
    reg_o_FERAMode   = 0xD
  };

  enum InputSubbAddress{
    reg_i_nimin1 = 0x0,
    reg_i_nimin2 = 0x1,
    reg_i_nimin3 = 0x2,
    reg_i_nimin4 = 0x3
  };
};

// BCT --------------------------------------------------------------------
namespace BCT{
  static const int mid = 0xe;
  enum LocalAddress{
    laddr_Reset   = 0x0, // W, assert reset signal from BCT
    laddr_Version = 0x10,// R, [31:0]
    laddr_ReConfig= 0x20 // W, Reconfig FPGA by SPI
  };
};
};
#endif
