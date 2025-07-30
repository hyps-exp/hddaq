/* 
 * Copyright 2003 (C) 2003 Yoshiji Yasu <Yoshiji.YASU@kek.jp>.
 * version: 0.1  04-DEC-2002, born
 *          0.9  11-AUG-2003  release for beta version
 *                              LAM & TRIG handling are not available.
 *          0.95 03-OCT-2003  modification of LAM & TRIG handler
 *          0.98 17-OCT-2007  modification for 2.4 -> 2.6 conversion
 */

#define PCI_VENDOR_ID_PCC 0x8642
#define PCI_DEVICE_ID_PCC 0x0630
#define PCC_MAJOR 70
#define CARD_NAME "pcc"

//#define MEASUREMENT 1
//#define POLLING // for debug
// mmzone.h defines MAX_ORDER = 11 in /usr/include/linux/
// Loading the device driver worked as the following value.
//#define MAX_BUFFER_SIZE 1024*4096 //  4 MB
#define MAX_BUFFER_SIZE 128*1024
#define MAX_LENGTH MAX_BUFFER_SIZE/4
#define MAX_FRAME_LENGTH ( MAX_BUFFER_SIZE/8 - 2 )
#define PCC_IRQ_SHARED 1 // This is used for shared IRQ.
#define MIN_SIZE 4 // Minumum transfer size is 4 bytes.
#define PCC_TIMEOUT_PIO 100000
#define PCC_TIMEOUT 600
//#define MAX_FIFO_FRAME 384
#define MAX_FIFO_FRAME 120
#define MAX_FIFO_COUNT MAX_FIFO_FRAME*2 
#define MAX_FIFO_BYTE MAX_FIFO_FRAME*8 
#define NUM_FRAME_SWITCH 20 // 20 frames at this moment

#define PCCIOC_RESET           _IOWR('c', 1, int)
#define PCCIOC_DUMP_REGISTERS  _IOWR('c', 2, struct pccreg)
#define PCCIOC_CLEAR_FIFO      _IO('c', 3)
#define PCCIOC_PUT_DATA        _IOWR('c', 5, int)
#define PCCIOC_GET_DATA        _IOWR('c', 6, int)
#define PCCIOC_GET_INTDATA     _IOWR('c', 7, int)
#define PCCIOC_EXEC_PIO        _IOWR('c', 9, int)
#define PCCIOC_KICK_READ       _IOWR('c', 10, int)
#define PCCIOC_WAIT_INTERRUPT  _IOWR('c', 11, int)
#define PCCIOC_ENABLE_INTERRUPT _IO('c', 12)
#define PCCIOC_DISABLE_INTERRUPT _IO('c', 13)
//----debug
// I/O access
#define PCCIOC_IO_READ 		 _IOWR('c', 14, int)
#define PCCIOC_IO_WRITE  	 _IOWR('c', 15, int)
// DAM+INT Tx,Rx Exec
#define PCCIOC_WR_EXE	  	 _IOWR('c', 16, int)
#define PCCIOC_WR_DRD	  	 _IOWR('c', 17, int)
//
#define PCCIOC_TMP             _IOWR('c', 100, int)

// DAQ functions
#define DAQEXE_CTRL_READ         0x0
#define DAQEXE_CTRL_CLRBSY   0x10000
#define DAQEXE_CTRL_CLRCNT   0x20000
#define DAQEXE_CTRL_ENABLE   0x30000
#define DAQEXE_CTRL_DISABLE  0x40000
#define DAQEXE_CTRL_PLSOUT   0x50000
#define DAQEXE_CTRL_NOPLSOUT 0x60000
#define DAQEXE_CTRL_TRIG_WAIT 0xA0000

// DAQ frame format
#define DAQEXE_CTRL 0x10000
#define DAQEXE_PLS    0x100
#define DAQEXE_STAT       0

// Status bit of DAQ frame
#define DAQEXE_STAT_PLSOUT    8
#define DAQEXE_STAT_TRIGIN    4
#define DAQEXE_STAT_ENTRIGIN  2
#define DAQEXE_STAT_BSY       1

struct pccreg {
  unsigned int TxData1;
  unsigned int TxData2;
  unsigned int TxControl;
  unsigned int TxStatus;
  unsigned int TxAddress;
  unsigned int TxPresetCount;
  unsigned int TxActualCount;
  unsigned int TxFifoCount;
  unsigned int RxData1;
  unsigned int RxData2;
  unsigned int RxControl;
  unsigned int RxStatus;
  unsigned int RxAddress;
  unsigned int RxPresetCount;
  unsigned int RxActualCount;
  unsigned int RxFifoCount;
  unsigned int System;
  unsigned int dummy1[7];
  unsigned int IntData1;
  unsigned int IntData2;
  unsigned int IntControl;
  unsigned int IntStatus;
  unsigned int dummy2[3];
  unsigned int IntFifoCount;
};

#define TXDATA1 0
#define TXDATA2 4
#define TXCONTROL 8
#define TXSTATUS  0xC
#define TXADDRESS 0x10
#define TXPRESETCOUNT 0x14
#define TXACTUALCOUNT 0x18
#define TXFIFOCOUNT 0x1C
#define RXDATA1 0x20
#define RXDATA2 0x24
#define RXCONTROL 0x28
#define RXSTATUS  0x2C
#define RXADDRESS 0x30
#define RXPRESETCOUNT 0x34
#define RXACTUALCOUNT 0x38
#define RXFIFOCOUNT 0x3C
#define SYSTEM 0x40
#define INTDATA1 0x60
#define INTDATA2 0x64
#define INTCONTROL 0x68
#define INTSTATUS 0x6C
#define INTFIFOCOUNT 0x7C

#define TC_INT_ENABLE               0x07000000
#define TC_INT_ENABLE_FORCE_END     0x04000000
#define TC_INT_ENABLE_FULL_FIFO     0x02000000
#define TC_INT_ENABLE_PKT_END       0x01000000
#define TC_INT_CLR                  0x00070000
#define TC_INT_CLR_FORCE_END        0x00040000
#define TC_INT_CLR_FULL_FIFO        0x00020000
#define TC_INT_CLR_PKT_END          0x00010000
#define TC_CLR_FIFO                 0x00000002
#define TC_SRT_DMA                  0x00000001

#define RC_INT_ENABLE               0x3F000000
#define RC_INT_ENABLE_FORCE_END     0x20000000
#define RC_INT_ENABLE_PRESET_FIFO   0x10000000
#define RC_INT_ENABLE_FULL_FIFO     0x08000000
#define RC_INT_ENABLE_HALFFULL_PKT  0x04000000
#define RC_INT_ENABLE_PKT_END       0x02000000
#define RC_INT_ENABLE_INPUT_FRAME   0x01000000
#define RC_INT_CLR                  0x003F0000
#define RC_INT_CLR_FORCE_END        0x00200000
#define RC_INT_CLR_PRESET_FIFO      0x00100000
#define RC_INT_CLR_FULL_FIFO        0x00080000
#define RC_INT_CLR_HALFFULL_PKT     0x00040000
#define RC_INT_CLR_PKT_END          0x00020000
#define RC_INT_CLR_INPUT_FRAME      0x00010000
#define RC_CLR_FIFO                 0x00000002
#define RC_SRT_DMA                  0x00000001

#define TS_RX_INFO                  0xF0000000
#define TS_RX_INFO3                 0x80000000
#define TS_RX_INFO2                 0x40000000
#define TS_RX_INFO1                 0x20000000
#define TS_RX_INFO0                 0x10000000
#define TS_TX_INFO                  0x0F000000
#define TS_TX_INFO3                 0x08000000
#define TS_TX_INFO2                 0x04000000
#define TS_TX_INFO1                 0x02000000
#define TS_TX_INFO0                 0x01000000
#define TS_INT                      0x00070000
#define TS_INT_FORCE_END            0x00040000
#define TS_INT_FULL_FIFO            0x00020000
#define TS_INT_PKT_END              0x00010000
#define TS_FULL_FIFO                0x00000400
#define TS_HALFFULL_FIFO            0x00000200
#define TS_EMPTY_FIFO               0x00000100
#define TS_TIMEOUT_FRAME            0x00000002
#define TS_DONE_FRAME               0x00000001

#define RS_RX_INFO                  0xF0000000
#define RS_RX_INFO3                 0x80000000
#define RS_RX_INFO2                 0x40000000
#define RS_RX_INFO1                 0x20000000
#define RS_RX_INFO0                 0x10000000
#define RS_TX_INFO                  0x0F000000
#define RS_TX_INFO3                 0x08000000
#define RS_TX_INFO2                 0x04000000
#define RS_TX_INFO1                 0x02000000
#define RS_TX_INFO0                 0x01000000
#define RS_INT                      0x003F0000
#define RS_INT_FORCE_END            0x00200000
#define RS_INT_PRESET_FIFO          0x00100000
#define RS_INT_FULL_FIFO            0x00080000
#define RS_INT_HALFFULL_PKT         0x00040000
#define RS_INT_PKT_END              0x00020000
#define RS_INT_INPUT_FRAME          0x00010000
#define RS_FULL_FIFO                0x00000400
#define RS_HALFFULL_FIFO            0x00000200
#define RS_EMPTY_FIFO               0x00000100
#define RS_TIMEOUT_FRAME            0x00000001

#define SYS_READY                   0x80000000
#define SYS_RESET                   0x40000000
#define SYS_CAMAC_FRAME_SIZE        0x03000000

#define IC_INT_ENABLE               0x01000000
#define IC_INT_CLR                  0x00010000
#define IC_CLR_FIFO                 0x00000002

#define IS_INT                      0x00010000
#define IS_FULL_FIFO                0x00000400
#define IS_HALFFULL_FIFO            0x00000200
#define IS_EMPTY_FIFO               0x00000100
#define IS_TIMEOUT_FRAME            0x00000001
