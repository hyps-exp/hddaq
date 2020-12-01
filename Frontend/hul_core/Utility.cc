#include<cstdio>
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
  
};
