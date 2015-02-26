#ifndef USERDEVICE_H
#define USERDEVICE_H

#include "Message/GlobalMessageClient.h"
#include "nodeprop.h"

int get_maxdatasize();
int open_device();
int close_device();
int init_device(DaqMode daq_mode);
int wait_device(DaqMode daq_mode);
int read_device(DaqMode daq_mode, unsigned int *data, int& len);
int finalize_device(DaqMode daq_mode);

inline static void send_normal(const char *message)
{
  static GlobalMessageClient& global_message_client = GlobalMessageClient::getInstance();
  global_message_client.sendString(MT_NORMAL, message);
  std::cout<<"#D "<<message<<std::endl;
}
inline static void send_warning(const char *message)
{
  static GlobalMessageClient& global_message_client = GlobalMessageClient::getInstance();
  global_message_client.sendString(MT_WARNING, message);
  std::cerr<<"#W "<<message<<std::endl;
}
inline static void send_error(const char *message)
{
  static GlobalMessageClient& global_message_client = GlobalMessageClient::getInstance();
  global_message_client.sendString(MT_ERROR, message);
  std::cerr<<"#E "<<message<<std::endl;
};
inline static void send_fatal(const char *message)
{
  static GlobalMessageClient& global_message_client = GlobalMessageClient::getInstance();
  std::string fatal;
  const char *fatal_head = "\n############################## FATAL ##############################\n";
  const char *fatal_foot = "\n###################################################################\n";
  fatal += fatal_head; fatal += message; fatal += fatal_foot;
  global_message_client.sendString(MT_FATAL, fatal);
  std::cerr<<fatal<<std::endl;
};

inline static void send_normal (std::string message){ send_normal(message.c_str());  }
inline static void send_warning(std::string message){ send_warning(message.c_str()); }
inline static void send_error  (std::string message){ send_error(message.c_str());   }
inline static void send_fatal  (std::string message){ send_fatal(message.c_str());   }

#endif
