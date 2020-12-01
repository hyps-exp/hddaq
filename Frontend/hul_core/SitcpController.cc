#include"SitcpController.hh"
#include"errno.h"
#include"UDPRBCP.hh"
#include"Utility.hh"

#include<iostream>
#include<cstdio>
#include<csignal>
#include<string>

namespace RBCP{

const uint32_t kAddrBase        = 0xffffff00;
const uint32_t kAddrCtrlSitcp   = 0x10;

const uint8_t  kCmdSitcpReset   = 0x80;
const uint8_t  kCmdKeepAlive    = 0x04;
const uint8_t  kCmdFastRetrains = 0x02;
const uint8_t  kCmdNagleBuffer  = 0x01;

// Reset SiTCP -------------------------------------------------------------
void
ResetSiTCP(const std::string ip)
{
  UDPRBCP udp_man(ip, gUdpPort);
  const uint32_t addr   = kAddrBase + kAddrCtrlSitcp;
  const uint8_t  wd_def = kCmdNagleBuffer; // default;
  const uint8_t  wd     = wd_def + kCmdSitcpReset;

  std::cout << "#D : ##### SiTCP Controll #####" << std::endl;
  std::cout << "#D : Trying Reset SiTCP" 
	    << std::endl; 

  udp_man.SetWD(addr, 1, &wd);
  udp_man.DoRBCP();

  udp_man.SetWD(addr, 1, &wd_def);
  udp_man.DoRBCP();
}

// Write SiTCP -------------------------------------------------------------
void
WriteSiTCP(const std::string ip, const uint32_t addr_ofs, const uint32_t reg)
{
  UDPRBCP udp_man(ip, gUdpPort);
  const uint32_t addr     = kAddrBase + addr_ofs;
  const uint8_t  reg_8bit = static_cast<const uint8_t>(reg & 0xff);

  std::cout << "#D : ##### SiTCP Controll #####" << std::endl;
  std::cout << std::hex 
	    << "#D : Trying Write Register (" << reg << ") to Address (" << addr << ")" 
	    << std::dec << std::endl; 

  udp_man.SetWD(addr, 1, &reg_8bit);
  udp_man.DoRBCP();
}

// Read SiTCP  -------------------------------------------------------------
void
ReadSiTCP(const std::string ip, const uint32_t addr_ofs)
{
  UDPRBCP udp_man(ip, gUdpPort);
  const uint32_t addr   = kAddrBase + addr_ofs;

  std::cout << "#D : ##### SiTCP Controll #####" << std::endl;
  std::cout << std::hex 
	    << "#D : Trying Read Register from Address (" << addr << ")" 
	    << std::dec << std::endl; 

  udp_man.SetRD(addr, 1);
  udp_man.DoRBCP();
}

// Erase EEPROM ------------------------------------------------------------
void
EraseEEPROM(const std::string ip)
{
  std::cout << "#D : ##### SiTCP Controll #####" << std::endl;
  std::cout << "#D : Trying Erase EEPROM" 
	    << std::endl; 

  UDPRBCP udp_man(ip, gUdpPort, UDPRBCP::kNoDisp);
  uint32_t addr = 0xfffffcff;
  uint8_t wd    = 0x0;
  udp_man.SetWD(addr, 1, &wd);
  udp_man.DoRBCP();

  // fill 128byte from 0xfffffc00 with 0xff 
  addr = 0xfffffc00;
  wd   = 0xff;
  for(uint32_t i = 0; i<128; ++i){
    //    std::cout << "." << std::flush;
    Utility::FlashMessage(" Erasing EEPROM");
    udp_man.SetWD(addr+i, 1, &wd);
    udp_man.DoRBCP();
    
  }// for(i)
  std::cout << std::endl;
  std::cout << "#D : Finish Erase EEPROM" << std::endl;
}
};
