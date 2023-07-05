#include <iostream>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include "rbcp.h"
#include "my_endian.h"

struct RBCPHeader{
  static const uint8_t READ = 0xC0;
  static const uint8_t WRITE = 0x80;
  static const size_t SIZE = 8;

  uint8_t varType;
  uint8_t cmdFlag;
  uint8_t id;
  uint8_t dataLength;
  uint32_t address;
  uint8_t buf[8];

  RBCPHeader(uint8_t rw, uint8_t id, uint32_t dataLength, uint32_t address) :
    varType(0xff),
    cmdFlag(rw),
    id(id),
    dataLength(dataLength),
    address(address)
  {
    makeBuf();
  }

  RBCPHeader(const uint8_t* str) :
    varType(0xff),
    cmdFlag(0),
    id(0),
    dataLength(0),
    address(0)
  {
    if(!str) {
      std::cerr << __FILE__ << ":" << __LINE__ << "str is NULL!";
      return;
    }

    cmdFlag = str[1];
    id = str[2];
    dataLength = str[3];
    address = Endian::getBigEndian32(str + 4);
    makeBuf();
  }

  const uint8_t* getBuf()
  {
    return buf;
  }

private:
  void makeBuf()
  {
    buf[0] = varType;
    buf[1] = cmdFlag;
    buf[2] = id;
    buf[3] = dataLength;
    Endian::setBigEndian32(buf + 4, address);
  }
};

RBCP::RBCP(const std::string& host, int port) :
  host_(host),
  port_(port),
  id_(0)
{
}

void RBCP::read(uint8_t* buf, uint32_t address, size_t dataLength)
{
  size_t totalReceivedDataSize = 0;
  while(totalReceivedDataSize < dataLength) {
    int dataLengthOnePacket = (dataLength > 255) ? 255 : dataLength;
    com_(RBCPHeader::READ, address, dataLengthOnePacket, NULL,
	 buf + totalReceivedDataSize);
    dataLength -= dataLengthOnePacket;
    address += dataLengthOnePacket;
    totalReceivedDataSize += dataLengthOnePacket;
  }
}

void RBCP::write(const uint8_t* buf, uint32_t address, size_t dataLength)
{
  size_t remainingDataLength = dataLength;
  int dataIndex = 0;
  while(remainingDataLength > 0) {
    int dataLengthOnePacket = (remainingDataLength > 255) ? 255 : remainingDataLength;
    uint8_t dataLengthToCom = static_cast<uint8_t>(dataLengthOnePacket);
    const uint8_t* dataToWrite = buf + dataIndex;
    com_(RBCPHeader::WRITE, address + dataIndex, dataLengthToCom, dataToWrite, NULL);
    remainingDataLength -= dataLengthOnePacket;
    dataIndex += dataLengthOnePacket;
  }
}

size_t RBCP::com_(uint8_t rw, uint32_t address, uint8_t dataLength,
                  const uint8_t* dataToBeSendBuf, uint8_t* receivedDataBuf)
{
  int retries = 0;
  const int maxRetries = 3;
  while(true) {
    try {
      return comSub_(rw, address, dataLength, dataToBeSendBuf, receivedDataBuf);
    }catch(RBCPError &e) {
      std::cerr << e.what() << std::endl;
      ++retries;
      if(retries < maxRetries) continue;
      throw e;
    }
  }
}

size_t RBCP::comSub_(uint8_t rw, uint32_t address, uint8_t dataLength,
                     const uint8_t* dataToBeSendBuf, uint8_t* receivedDataBuf)
{
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    throw RBCPError("Failed to create socket");
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  addr.sin_addr.s_addr = inet_addr(host_.c_str());

  RBCPHeader rbcp_header(rw, id_, dataLength, address);
  const uint8_t* header = rbcp_header.getBuf();
  uint8_t dataToBeSent[256] = {};
  size_t commandPacketLength = RBCPHeader::SIZE + dataLength;
  ::memcpy(dataToBeSent, header, RBCPHeader::SIZE);
  ::memcpy(dataToBeSent + RBCPHeader::SIZE, dataToBeSendBuf, dataLength);

  ssize_t sentDataLength = sendto(sock, (char*)dataToBeSent, commandPacketLength, 0,
				  (struct sockaddr*)&addr, sizeof(addr));
  if(static_cast<size_t>(sentDataLength) != commandPacketLength) {
    perror("sendto");
  }

#if 0
  {
    int j=0;
    for(int i=0, n=commandPacketLength; i<n; i++){
      if(j==0) {
	printf("\t[%.3x]:%.2x ",i,(uint8_t)dataToBeSent[i]);
	j++;
      }else if(j==3){
	printf("%.2x\n",(uint8_t)dataToBeSent[i]);
	j=0;
      }else{
	printf("%.2x ",(uint8_t)dataToBeSent[i]);
	j++;
      }
    }
    if(j!=3) printf("\n");
    printf("\n");
  }
#endif

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);

  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  int select_ret = select(sock + 1, &rfds, NULL, NULL, &tv);
  if(select_ret < 0) {
    char err_buf[256];
    strerror_r(errno, err_buf, sizeof(err_buf));
    ++id_;
    close(sock);
    throw RBCPError(err_buf);
  }else if(select_ret == 0) {
    ++id_;
    close(sock);
    throw RBCPError("Timeout");
  }

  uint8_t receivedData[2048];
  ssize_t receivedDataLength = recvfrom(sock, receivedData, 2048, 0, NULL, NULL);
  if(receivedDataLength < 0) {
    char err_buf[256];
    strerror_r(errno, err_buf, sizeof(err_buf));
    close(sock);
    throw RBCPError(err_buf);
  }
  try {
    validate_(rw, address, dataLength, receivedData, receivedDataLength);
  }catch(std::exception &e) {
    ++id_;
    close(sock);
    throw e;
  }

  if(receivedDataBuf != NULL) {
    ::memcpy(receivedDataBuf, receivedData, receivedDataLength);
  }
  ++id_;
  close(sock);

  return receivedDataLength;
}

void RBCP::validate_(uint8_t rw, uint32_t address, uint8_t dataLength,
                     const uint8_t *receivedData, ssize_t receivedDataLength)
{

  if(receivedDataLength < 8) {
    throw RBCPError("Too short ACK packet");
  }

  RBCPHeader header(receivedData);
  if(header.varType != 0xff) {
    throw RBCPError("Invalid Ver Type");
  }
  if(header.cmdFlag != (rw | 0x08)) {
    if(header.cmdFlag & 0x01) {
      throw RBCPError("Bus Error");
    }else {
      throw RBCPError("Invalid CMD Flag");
    }
  }
  if(header.id != id_) {
    throw RBCPError("Invalid ID");
  }
  if(header.dataLength != dataLength) {
    throw RBCPError("Invalid DataLength");
  }
  if(header.address != address) {
    throw RBCPError("Invalid Address");
  }
  if(header.dataLength != receivedDataLength - RBCPHeader::SIZE) {
    throw RBCPError("Frame Error");
  }
}
