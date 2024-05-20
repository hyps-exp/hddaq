#ifndef REGISTER_MAP_TRGFW_HH
#define REGISTER_MAP_TRGFW_HH

namespace LBUS{
  //-------------------------------------------------------------------------
  // TRM Module
  //-------------------------------------------------------------------------
  namespace TRM{
    enum LocalAddress
      {
	kAddrSelectTrigger = 0x10000000 // W/R, [11:0] select trigger line
      };

    enum command_sel_trig
      {
	kRegL1Ext = 0x1,
	kRegL1J0  = 0x2,
	kRegL1RM  = 0x4,
	kRegL2Ext = 0x8,
	kRegL2J0  = 0x10,
	kRegL2RM  = 0x20,
	kRegClrExt= 0x40,
	kRegClrJ0 = 0x80,
	kRegClrRM = 0x100,
	kRegEnL2  = 0x200,
	kRegEnJ0  = 0x400,
	kRegEnRM  = 0x800
      };
  };

  //-------------------------------------------------------------------------
  // DCT Module
  //-------------------------------------------------------------------------
  namespace DCT{
    enum LocalAddress
      {
	kAddrDaqGate   = 0x20000000, // W/R, [0:0] Set DAQ Gate reg
	kAddrResetEvb  = 0x20100000  // W,   Assert EVB reset (self counter reset)
      };
  };

  //-------------------------------------------------------------------------
  // TDC Module
  //-------------------------------------------------------------------------
  namespace TDC{
    enum LocalAddress
      {
	kAddrEnableBlock   = 0x30000000, // W/R, [7:0]  Block enable (0:3-Leading, 4:7-Trailing)
	kAddrPtrOfs        = 0x30100000, // W/R, [10:0] Offset read pointer
	kAddrWindowMax     = 0x30200000, // W/R, [10:0] Search window max
	kAddrWindowMin     = 0x30300000  // W/R, [10:0] Search window min
      };

    enum TdcBlock
      {
	kEnLeading  = 0x1,
	kEnTrailing = 0x2
      };
  };

  //-------------------------------------------------------------------------
  // IOM Module
  //-------------------------------------------------------------------------
  namespace IOM{
    enum LocalAddress
      {
	kAddrNimout1     = 0x40000000, // W/R, [3:0]
	kAddrNimout2     = 0x40100000, // W/R, [3:0]
	kAddrNimout3     = 0x40200000, // W/R, [3:0]
	kAddrNimout4     = 0x40300000, // W/R, [3:0]
	kAddrExtL1       = 0x40400000, // W/R, [2:0]
	kAddrExtL2       = 0x40500000, // W/R, [2:0]
	kAddrExtClr      = 0x40600000, // W/R, [2:0]
	kAddrExtBusy     = 0x40700000, // W/R, [2:0]
	kAddrExtRsv2     = 0x40800000  // W/R, [2:0]
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
	kReg_o_clk1kHz    = 0xC,
        kReg_o_NC         = 0xE,
	kReg_o_Default    = 0xF
      };

    enum InputSubbAddress
      {
	kReg_i_Nimin1 = 0x0,
	kReg_i_Nimin2 = 0x1,
	kReg_i_Nimin3 = 0x2,
	kReg_i_Nimin4 = 0x3,
	kReg_i_NC     = 0x6,
	kReg_i_Default= 0x7
      };
  };

  //-------------------------------------------------------------------------
  // ADC Module
  //-------------------------------------------------------------------------
  namespace ADC{
    enum LocalAddress
      {
	kAddrPtrOfs        = 0x50000000, // W/R, [10:0] Offset read pointer
	kAddrWindowMax     = 0x50100000, // W/R, [10:0] Search window max
	kAddrWindowMin     = 0x50200000, // W/R, [10:0] Search window min
	kAddrAdcRoReset    = 0x50300000, // W/R, [0:0]  AdcRo reset signal
	kAddrAdcRoIsReady  = 0x50400000, // R,   [3:0]  AdcRo IsReady signals
      };
  };

};

#endif
