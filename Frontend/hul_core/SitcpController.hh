#ifndef SITCPCTRL_HH
#define SITCPCTRL_HH

#include<stdint.h>
#include<string>

#include"rbcp.hh"
#include"network.hh"

namespace RBCP{

void ResetSiTCP(const std::string ip);
void WriteSiTCP(const std::string ip, const uint32_t addr_ofs, const uint32_t reg);
void ReadSiTCP(const std::string ip, const uint32_t addr_ofs);
void EraseEEPROM(const std::string ip);

};

#endif
