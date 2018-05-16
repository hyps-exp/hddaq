#ifndef RBCP_INCLUDED
#define RBCP_INCLUDED

#include <string>
#include <exception>
#include <stdexcept>

static const int udp_port = 4660;

class RBCP {
 public:
  RBCP(const std::string& host, int port = udp_port);
  void read(uint8_t* buf, uint32_t address, size_t dataLength);
  void write(const uint8_t* buf, uint32_t address, size_t dataLength);

 private:
  std::string host_;
  int port_;
  uint8_t id_;

  size_t com_(uint8_t rw, uint32_t address, uint8_t dataLength,
	      const uint8_t* dataToBeSendBuf, uint8_t* receivedDataBuf);
  size_t comSub_(uint8_t rw, uint32_t address, uint8_t dataLength,
		 const uint8_t* dataToBeSendBuf, uint8_t* receivedDataBuf);
  void validate_(uint8_t rw, uint32_t address, uint8_t dataLength,
		 const uint8_t *receivedData, ssize_t receivedDataLength);

};

class RBCPError : public std::runtime_error
{
 public:
 RBCPError(const std::string& what_arg) :
  std::runtime_error("RBCP Error:" + what_arg)
    {}
};

#endif
