#ifndef UDPRBCP_HH
#define UDPRBCP_HH

#include<vector>
#include<algorithm>
#include<string>
#include<stdint.h>

#include"rbcp.hh"

namespace RBCP{
  
  constexpr uint32_t gUdpPort = 4660;

  enum class DebugMode : int32_t
    {
      kNoDisp, kInteractive, kDebug,
      kSizeDebugMode
    };


  // __________________________________________________________________
  class UDPRBCP
  {
  public:
    static constexpr uint32_t kSizeUdpBuf_ {4096};
    static constexpr uint32_t kRbcpVer_    {0xFF};  
  
  private:
    static constexpr uint32_t kRbcpCmdWr_  {0x80};  
    static constexpr uint32_t kRbcpCmdRd_  {0xC0};  

    const std::string  ip_addr_;
    const uint32_t     port_;
    RbcpHeader         send_header_;

    int32_t            length_rd_;
    uint8_t            wd_buffer_[kSizeUdpBuf_];
    uint8_t            rd_buffer_[kSizeUdpBuf_];

    DebugMode          mode_;

    // __________________________________________________________________
  public:
    UDPRBCP() = delete;
    UDPRBCP(const UDPRBCP&) = delete;
    UDPRBCP(const std::string ipAddr, const uint32_t port,
	    const DebugMode mode = DebugMode::kInteractive)
      :
      ip_addr_(ipAddr),
      port_(port),
      length_rd_(0),
      mode_(mode)
    {
      send_header_.type = RBCP::UDPRBCP::kRbcpVer_;
      send_header_.id   = 0;
    }

    virtual ~UDPRBCP() = default;
    UDPRBCP& operator=(const UDPRBCP&) = delete;

    // Member methods ___________________________________________________
    void CopyRD(std::vector<uint8_t>& buffer)
    {
      std::copy(rd_buffer_, rd_buffer_ +length_rd_,
		back_inserter(buffer)
		);
    }
    
    int  DoRBCP();
    void SetDispMode(const DebugMode mode)
    {
      mode_ = mode;
    }

    void SetRbcpVer(const uint8_t version)
    {
      send_header_.type = version;
    }
    
    void SetRbcpId(const uint8_t id)
    {
      send_header_.id = id;
    }

    void SetRD(const uint32_t address, const uint32_t legnth);
    void SetWD(const uint32_t address, const uint32_t length,
	       const uint8_t* send_data);


  }; // End of Class definition

}; // End of namespace RBCP

#endif
