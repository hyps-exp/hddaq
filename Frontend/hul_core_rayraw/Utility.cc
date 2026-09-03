#include<cstdio>
#include<iostream>
#include<unistd.h>

#include"Utility.hh"

namespace Utility{

  void
  PrintProgressBar(const double max, const double current)
  {
    const int    kNumCycle = 20;
    const double kFactor   = 5.;
  
    int percent = static_cast<int>(current/max*100);
    printf("\r[");
    for(int i = 0; i<kNumCycle; ++i){
      printf("%s", (i+1)*kFactor > percent? "  " : "##");
    }
    printf("]%3d%%", percent);
    fflush(stdout);

    if(percent == 100) printf("\n");
  }

  void
  FlashMessage(const std::string message)
  {
    static bool display_on = true;
    printf("\r");
    if(display_on){
      printf("%s", message.c_str());
      display_on = false;
    }else{
      std::string blank(message.length(), ' ');
      printf("%s", blank.c_str());
      display_on = true;
    }

    fflush(stdout);
  }


  uint32_t
  BitReverse(uint32_t reg)
  {
    reg = (reg & 0x55555555) << 1  | (reg & 0xAAAAAAAA) >> 1;
    reg = (reg & 0x33333333) << 2  | (reg & 0xCCCCCCCC) >> 2;
    reg = (reg & 0x0F0F0F0F) << 4  | (reg & 0xF0F0F0F0) >> 4;
    reg = (reg & 0x00FF00FF) << 8  | (reg & 0xFF00FF00) >> 8;
    reg = (reg & 0x0000FFFF) << 16 | (reg & 0xFFFF0000) >> 16;
    return reg;
  }

  void
  PrintError(std::string_view func_name, std::string_view message)
  {
    std::cerr << "#E: " << func_name << " " << message << std::endl;
  }

  void
  PrintNormal(std::string_view func_name, std::string_view message)
  {
    std::cerr << "#D: " << func_name << " " << message << std::endl;
  }

};
