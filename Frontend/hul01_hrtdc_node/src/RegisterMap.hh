#ifndef REGISTER_HH
#define REGISTER_HH

#include"RegisterMapCommon.hh"

namespace HUL{
// ------------------------------------------------------------------------
// HR-TDC BASE
// ------------------------------------------------------------------------

namespace HRTDC_BASE{

//-------------------------------------------------------------------------
// TRM Module
//-------------------------------------------------------------------------
namespace TRM{
  enum LocalAddress
    {
     kAddrSelectTrigger = 0x00000000 // W/R, [11:0] select trigger line
    };

  enum command_sel_trig
    {
     kRegL1Ext = 0x1,
     kRegL1J0  = 0x2,
     //     kRegL1RM  = 0x4,
     kRegL2Ext = 0x8,
     kRegL2J0  = 0x10,
     //     kRegL2RM  = 0x20,
     kRegClrExt= 0x40,
     kRegClrJ0 = 0x80,
     //     kRegClrRM = 0x100,
     kRegEnL2  = 0x200,
     kRegEnJ0  = 0x400,
     //     kRegEnRM  = 0x800
    };
};

//-------------------------------------------------------------------------
// DCT Module
//-------------------------------------------------------------------------
namespace DCT{
  enum LocalAddress
    {
     kAddrDaqGate   = 0x10000000, // W/R, [0:0] Set DAQ Gate reg
     kAddrResetEvb  = 0x10100000, // W,         Assert EVB reset (self counter reset)
     kAddrInitDDR   = 0x10200000, // W,         Assert DDR Receiver initialize
     kAddrCtrlReg   = 0x10300000, // W/R, [3:0] DDR receiver controll register
     kAddrRcvStatus = 0x10400000  // R,   [3:0] DDR receiver status
    };

  enum Ctrl
    {
     kRegTestModeU  =0x1,
     kRegTestModeD  =0x2,
     kRegEnableU    =0x4,
     kRegEnableD    =0x8
    };

  enum Status
    {
     kRegBitAlignedU = 0x1,
     kRegBitAlignedD = 0x2,
     kRegBitErrorU   = 0x4,
     kRegBitErrorD   = 0x8
    };
};

//-------------------------------------------------------------------------
// IOM Module
//-------------------------------------------------------------------------
namespace IOM{
  enum LocalAddress
    {
     kAddrNimout1     = 0x20000000, // W/R, [3:0]
     kAddrNimout2     = 0x20100000, // W/R, [3:0]
     kAddrNimout3     = 0x20200000, // W/R, [3:0]
     kAddrNimout4     = 0x20300000, // W/R, [3:0]
     kAddrExtL1       = 0x20400000, // W/R, [2:0]
     kAddrExtL2       = 0x20500000, // W/R, [2:0]
     kAddrExtClr      = 0x20600000, // W/R, [2:0]
     kAddrExtBusy     = 0x20700000, // W/R, [2:0]
     //     kAddrExtRsv2     = 0x20A00000  // W/R, [2:0]
     kAddrCntRst      = 0x20900000  // W/R, [2:0]
    };

  enum OutputSubbAddress
    {
     kReg_o_ModuleBusy = 0x0,
     //    kReg_o_CrateBusy  = 0x1,
     //    kReg_o_RML1       = 0x2,
     //    kReg_o_RML2       = 0x3,
     //    kReg_o_RMClr      = 0x4,
     //    kReg_o_RMRsv1     = 0x5,
     //    kReg_o_RMSnInc    = 0x6,
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
     kReg_i_Nimin4 = 0x3
    };
};

//-------------------------------------------------------------------------
// MIF
//-------------------------------------------------------------------------
namespace MIF{
  enum BaseAddress
    {
     kUp    = 0x30000000,
     kDown  = 0x40000000
    };

  enum LocalAddress
    {
     kAddrConnect    = 0x00000000,
     kAddrReg        = 0x00100000,
     kAddrForceReset = 0x01000000 // W [0:0] force reset of MZN
    };
};

};


// ------------------------------------------------------------------------
// Mezzanine HR-TDC
// ------------------------------------------------------------------------
namespace HRTDC_MZN{
// ------------------------------------------------------------------------
// DCT
// ------------------------------------------------------------------------
namespace DCT{
  enum LocalAddress
    {
     kAddrTestMode  = 0x000, // W/R [0:0] enable ddr test mode
     kAddrExtraPath = 0x010, // W/R [0:0] enable clock caliblation
     kAddrGate      = 0x020, // W/R [0:0] DAQ gate
     kAddrEnBlocks  = 0x030  // W/R [1:0] 0: leading, 1: trailing
    };

  enum EnableBlocks
    {
     kEnLeading  = 1,
     kEnTrailing = 2
    };
};

// ------------------------------------------------------------------------
// TDC
// ------------------------------------------------------------------------
namespace TDC{
  enum LocalAddress
    {
     kAddrControll   = 0x110,  // W/R [2:0] Controll bits
     kAddrReqSwitch  = 0x120,  // W,  Assert manual switch pulse
     kAddrStatus     = 0x130,  // R,  [0:0] Read status
     kAddrPtrOfs     = 0x140,  // W/R [10:0] Pointer offset
     kAddrWinMax     = 0x150,  // W/R [10:0] Search window high
     kAddrWinMin     = 0x160   // W/R [10:0] Search window low
    };

  enum ControllBits
    {
     kRegThrough     = 0x1,
     kRegAutosw      = 0x2,
     kRegStopDout    = 0x4
    };

  enum StatusBits
    {
     kRegReadyLut    = 0x1
    };
};

// ------------------------------------------------------------------------
// SDS
// ------------------------------------------------------------------------
namespace SDS{
  enum LocalAddress
    {
     kAddrSdsStatus      = 0xC00,  // R   [7:0]

     kAddrXadcDrpMode    = 0xC10,  // W/R [0:0]
     kAddrXadcDrpAddr    = 0xC20,  // W/R [6:0]
     kAddrXadcDrpDin     = 0xC30,  // W/R [15:0]
     kAddrXadcDrpDout    = 0xC40,  // R   [15:0]
     kAddrXadcExecute    = 0xC50,  // W

     kAddrSemCorCount    = 0xCA0,  // R   [15:0]
     kAddrSemRstCorCount = 0xCB0,  // W
     kAddrSemErrAddr     = 0xCC0,  // W   [39:0]
     kAddrSemErrStrobe   = 0xCD0   // W
    };

};

// ------------------------------------------------------------------------
// BCT
// ------------------------------------------------------------------------
namespace BCT{
  enum LocalAddress
    {
     kAddrReset   = 0xE00, // W, assert reset signal from BCT
     kAddrVersion = 0xE10,// R, [31:0]
     kAddrReConfig= 0xE20 // W, Reconfig FPGA by SPI
    };
};
};
};
#endif
