#ifndef REGISTER_HH
#define REGISTER_HH

#include<stdint.h>

namespace HUL{
namespace CITIROC{
// ASIC -------------------------------------------------------------------
namespace ASIC{
  static const uint32_t mid = 0x3;
  enum LocalAddress
    {
     kAddrSlowControlFIFO     = 0x000, // W, [7:0]
     kAddrPinDirectControl    = 0x010, // W, [7:0]
     kAddrCycleControl        = 0x020  // W, [7:0]
    };
};
};

// SDS --------------------------------------------------------------------
namespace SDS{
  static const uint32_t mid = 0xc;
  enum LocalAddress
    {
     kAddrSdsStatus       = 0x000, // R,   [7:0]
     kAddrXadcDrpMode     = 0x010, // W/R, [0:0]
     kAddrXadcDrpAddr     = 0x020, // W/R, [6:0]
     kAddrXadcDrpDin      = 0x030, // W/R, [7:0] (2byte)
     kAddrXadcDrpDout     = 0x040, // R,   [7:0] (2byte)
     kAddrXadcExecute     = 0x0f0, // W,
     kAddrSemCorCount     = 0x100, // R    [7:0] (2byte)
     kAddrSemRstCorSount  = 0x200, // W
     kAddrSemErrAddr      = 0x300, // W    [7:0] (5byte)
     kAddrSemErrStrobe    = 0x400  // W
    };

  enum SdsStatusBit
    {
     kXadcOverTemperature   = 0x01,
     kXadcUserTempAlarm     = 0x02,
     kXadcUserVccIntAlarm   = 0x04,
     kReserve1              = 0x08,
     kSemWatchdogAlarm      = 0x10,
     kSemUncorrectableAlarm = 0x20,
     kReserve2              = 0x40,
     kReserve3              = 0x80
    };
  
  enum DrpMode
    {
     kDrpReadMode  = 0,
     kDrpWriteMode = 1,
    };

  enum DrpAddress
    {
     kAddrDrpTemp    = 0x0,
     kAddrDrpVccInt  = 0x1,
     kAddrDrpVccAux  = 0x2,
     kAddrDrpVccBram = 0x6
    };

  enum SemErrCommand
    {
     kSetIdle    = 0xe,
     kSetObserve = 0xa,
     kSetError   = 0xc,
     kSetReset   = 0xb
     
    };
};

// FMP --------------------------------------------------------------------
namespace FMP{
  static const uint32_t mid = 0xd;
  enum LocalAddress
    {
     kAddrStatus          = 0x000, // R,   [7:0]
     kAddrCtrl            = 0x010, // W/R, [7:0]
     kAddrRegister        = 0x020, // W/R, [7:0] W,R,D,A0,A1,A2,A3,I (8 byte)
     kAddrInstLength      = 0x030, // W/R, [2:0]
     kAddrReadLength      = 0x040, // W/R, [9:0]
     kAddrWriteLength     = 0x050, // W/R, [9:0]
     kAddrReadCountFifo   = 0x060, // R, [9:0]
     kAddrReadFifo        = 0x070, // R, [7:0]
     kAddrWriteCountFifo  = 0x080, // R, [9:0]
     kAddrWriteFifo       = 0x090, // R, [7:0]
     kAddrExecute         = 0x100  // W
    };

  enum IndexRegister
    {
     kIndexInst  = 0,
     kIndexAddr3 = 1,
     kIndexAddr2 = 2,
     kIndexAddr1 = 3,
     kIndexAddr0 = 4,
     kIndexDummy = 5,
     kIndexRead  = 6,
     kIndexWrite = 7
    };

  enum StatusRegister
    {
     kBusyCycle     = 0x1
    };

  enum CtrlRegister
    {
     kCtrlReadMode  = 0x0,
     kCtrlWriteMode = 0x1,
     kCtrlInstMode  = 0x2,
     kCtrlDummyMode = 0x4
    };
};

// BCT --------------------------------------------------------------------
namespace BCT{
  static const uint32_t mid = 0xe;
  enum LocalAddress
    {
     kAddrReset   = 0x0, // W, assert reset signal from BCT
     kAddrVersion = 0x10,// R, [31:0]
     kAddrReConfig= 0x20 // W, Reconfig FPGA by SPI
    };
};
};

#endif
