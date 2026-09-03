#ifndef UTILITY_HH
#define UTILITY_HH

#include<string>
#include<string_view>
#include<stdint.h>

namespace Utility
{
  void     PrintProgressBar(const double max, const double current);
  void     FlashMessage(const std::string message);
  uint32_t BitReverse(uint32_t reg);

  void     PrintError(std::string_view func_name, std::string_view message);
  void     PrintNormal(std::string_view func_name, std::string_view message);
};

#endif
