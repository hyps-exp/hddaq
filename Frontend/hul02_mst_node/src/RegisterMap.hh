#ifndef REGISTERH_
#define REGISTERH_

namespace HUL_MsT{
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
    laddr_evb_reset  = 0x010, // W,   Assert EVB reset (self counter reset)
    laddr_init_ddr   = 0x020, // W,   Assert DDR Receiver initialize
    laddr_ctrl_reg   = 0x030, // W/R, [3:0] DDR receiver controll register
    laddr_rcv_status = 0x040  // R,   [1:0] DDR receiver status
  };

  enum Ctrl{
    reg_test_mode    = 0x1
  };

  enum Status{
    reg_bit_aligned  = 0x1,
    reg_bit_error    = 0x2
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
    laddr_extRsv2     = 0x080, // W/R, [2:0]
    laddr_cntRst      = 0x090, // W/R, [2:0]
    laddr_pikTrig     = 0x0A0  // W/R, [2:0]
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
    reg_o_accept     = 0xD,
    reg_o_clear      = 0xE,
    reg_o_consolation_accept = 0xF,
    reg_o_final_clear        = 0x10,
    reg_o_level2             = 0x11,
    reg_o_fast_clear         = 0x12,
    reg_o_no_decision        = 0x13
  };

  enum InputSubbAddress{
    reg_i_nimin1 = 0x0,
    reg_i_nimin2 = 0x1,
    reg_i_nimin3 = 0x2,
    reg_i_nimin4 = 0x3
  };
};

//-------------------------------------------------------------------------
// MIF 
//-------------------------------------------------------------------------
namespace MIF{
  enum LocalAddress{
    laddr_connect = 0x000,
    laddr_reg_mif = 0x010,
    laddr_frst    = 0x100 // W [0:0] force reset of MZN
  };  
};

//-------------------------------------------------------------------------
// MIFD 
//-------------------------------------------------------------------------
namespace MIFD{
  static const int mid = 0x3;
};

//-------------------------------------------------------------------------
// LR-TDC
//-------------------------------------------------------------------------
namespace LRTDC{
  static const int mid = 0x4;
  enum LocalAddress{
    laddr_enblock = 0x000, // W/R, [3:0] Block enable
    laddr_ptrofs  = 0x010, // W/R, [10:0] pointer offset
    laddr_win_max = 0x020, // W/R, [10:0] search window (max)
    laddr_win_min = 0x030  // W/R, [10:0] search window (max)
  };
};

//-------------------------------------------------------------------------
// MsT
//-------------------------------------------------------------------------
namespace MsT{
  static const int mid = 0x5;
  enum LocalAddress{
    laddr_clear_preset  = 0x000, // W/R, [6:0] Clear pre-scale
    laddr_timer_preset  = 0x010, // W/R, [8:0] Decision timer preset
    laddr_win_max       = 0x020, // W, [23:0] search window (max)
    laddr_win_min       = 0x030, // W, [23:0] search window (max)
    laddr_exec          = 0x040, // W, Execute shift register
    laddr_bypass        = 0x050  // W/R, Level1 bypass setting
  };
};

// ------------------------------------------------------------------------
// BCT 
// ------------------------------------------------------------------------
namespace BCT{
  static const int mid = 0xe;
  enum LocalAddress{
    laddr_Reset   = 0x0, // W, assert reset signal from BCT
    laddr_Version = 0x10,// R, [31:0]
    laddr_ReConfig= 0x20 // W, Reconfig FPGA by SPI
  };
};
};

namespace HRTDC_MZN{
// ------------------------------------------------------------------------
// DCT
// ------------------------------------------------------------------------
namespace DCT{
  static const int mid = 0x0;
  enum LocalAddress{
    laddr_test_mode  = 0x00, // W/R [0:0] enable ddr test mode
    laddr_extra_path = 0x10, // W/R [0:0] enable clock caliblation
    laddr_gate       = 0x20  // W/R [0:0] DAQ gate
  };
};

// ------------------------------------------------------------------------
// TDC
// ------------------------------------------------------------------------
namespace TDC{
  static const int mid = 0x1;
  enum LocalAddress{
    laddr_controll    = 0x10,  // W/R [2:0] Controll bits
    laddr_req_switch  = 0x20,  // W,  Assert manual switch pulse
    laddr_status      = 0x30,  // R,  [0:0] Read status
    laddr_ptrofs      = 0x40,  // W/R [10:0] Pointer offset
    laddr_win_max     = 0x50,  // W/R [10:0] Search window high
    laddr_win_min     = 0x60   // W/R [10:0] Search window low
  };

  enum ControllBits{
    reg_through       = 0x1,
    reg_autosw        = 0x2,
    reg_stop_dout     = 0x4
  };

  enum StatusBits{
    reg_ready_lut     = 0x1
  };
};

// ------------------------------------------------------------------------
// BCT
// ------------------------------------------------------------------------
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
