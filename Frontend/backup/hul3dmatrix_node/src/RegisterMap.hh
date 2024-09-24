#ifndef REGISTERH_
#define REGISTERH_

//-------------------------------------------------------------------------
// LED Module
//-------------------------------------------------------------------------
namespace LED{
  static const int mid = 0x0;
  enum LocalAddress{
    laddr_led   = 0x000 // W/R, [3:0] LED on/off
  };
};

//-------------------------------------------------------------------------
// TDC Module
//-------------------------------------------------------------------------
namespace TDC{
  static const int mid = 0x1;
  enum LocalAddress{
    laddr_start   = 0x000, // W, send dummy event packet
    laddr_lowth   = 0x001, // W, [7:0] search window (low)
    laddr_highth  = 0x002  // W, [7:0] search window (high)
  };
};

//-------------------------------------------------------------------------
// SEL Module
//-------------------------------------------------------------------------
namespace SEL{
  static const int mid = 0x2;
  enum LocalAddress{
    laddr_nim1  = 0x000, // W, [3:0]
    laddr_nim2  = 0x001, // W, [3:0]
    laddr_nim3  = 0x002, // W, [3:0]
    laddr_nim4  = 0x003  // W, [3:0]
  };
};

//-------------------------------------------------------------------------
// MTX3D Module
//-------------------------------------------------------------------------
namespace MTX3D{
  static const int mid = 0x3;
  enum LocalAddress{
    laddr_low   = 0x000, // W,
    laddr_high  = 0x001, // W,
    laddr_exec  = 0x002  // W,
  };
};

//-------------------------------------------------------------------------
// MTX2D Module
//-------------------------------------------------------------------------
namespace MTX2D{
  static const int mid = 0x4;
  enum LocalAddress{
    laddr_low   = 0x000, // W,
    laddr_high  = 0x001, // W,
    laddr_exec  = 0x002  // W,
  };
};

// BCT --------------------------------------------------------------------
namespace BCT{
  static const int mid = 0xe;
  enum LocalAddress{
    laddr_Reset   = 0x0, // W, arise reset signal from BCT
    laddr_Version = 0x10,// R, [31:0]
    laddr_ReConfig= 0x20 // W, Reconfig FPGA by SPI
  };
};

#endif
