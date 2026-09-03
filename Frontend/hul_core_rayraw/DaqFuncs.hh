#ifndef DAQFUNCS_HH
#define DAQFUNCS_HH

#include<string>
#include<stdint.h>

namespace HUL::DAQ{

  static const int32_t kRecvNormal   {1};
  static const int32_t kRecvZero     {0};
  static const int32_t kRecvError   {-1};
  static const int32_t kRecvTimeOut {-4};
  
  void    UserStop_FromCtrlC(int signal);
  
  void    DoTrgDaq(std::string ip, int32_t runno, int32_t event_num, uint32_t daq_gate_address);
  void    DoStrDaq(std::string ip, int32_t runno);

  int32_t ConnectSocket(std::string ip);
  int32_t DoEventCycle(int sock, uint32_t* buffer);
  int32_t Receive(int sock, uint8_t* data_buf, uint32_t length, uint32_t& num_recieved_bytes);
  
};
#endif
