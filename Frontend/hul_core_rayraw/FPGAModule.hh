#ifndef FPGAMODULE_HH
#define FPGAMODULE_HH

#include<vector>
#include<string>
#include<stdint.h>

namespace RBCP{
  struct RbcpHeader;
  class  UDPRBCP;
};

namespace HUL{

  //________________________________________________________________
  class FPGAModule
  {
  public:
    using DataType    = std::vector<uint8_t>;
    using DataTypeItr = DataType::const_iterator;

    FPGAModule() = delete;
    FPGAModule(const FPGAModule&) = delete;
    FPGAModule(RBCP::UDPRBCP& udp_rbcp)
      :
      udp_rbcp_(udp_rbcp){ };
    virtual ~FPGAModule() = default;
    FPGAModule& operator=(const FPGAModule&) = delete;

    // 32-bit length regiser ----------------------------------
    // n cycle write rbcp by inlrementing laddr
    int32_t WriteModule(const uint32_t local_address,
			const uint32_t write_data,
			const int32_t  n_cycle = 1
			);
  
    // n cycle read rbcp by inlrementing laddr
    // (Return value is read register.)
    uint32_t ReadModule(const uint32_t local_address,
			const int32_t  n_cycle = 1
			);

    // 64-bit length regiser ----------------------------------
    // n cycle write rbcp by inlrementing laddr
    int32_t WriteModule64(const uint32_t local_address,
			  const uint64_t write_data,
			  const int32_t  n_cycle = 1
			  );
  
    // n cycle read rbcp by inlrementing laddr
    // (Return value is read register.)
    uint64_t ReadModule64(const uint32_t local_address,
			  const int32_t  n_cycle = 1
			  );

  
    // n byte write rbcp
    int32_t WriteModule_nByte(const uint32_t local_address,
			      const uint8_t* write_data,
			      const uint32_t n_byte
			      );

    // n byte read rbcp
    // (data are stored in rd_data_)
    int32_t ReadModule_nByte(const uint32_t local_address,
			     const uint32_t n_byte
			     );

    uint32_t GetReadWord(){return rd_word_;};

    DataTypeItr GetDataIteratorBegin(){ return rd_data_.begin(); };
    DataTypeItr GetDataIteratorEnd(){ return rd_data_.end(); };
  
  private:
    // RBCP data structure
    // RBCP_ADDR [31:0]
    // RBCP_WD   [7:0]
    //
    // Module ID     : RBCP_ADDR[31:28]
    // Local address : RBCP_ADDR[27:16]
    // LocalBus data : RBCP_WD[7:0]

    static constexpr uint32_t    kDataMask       {0xFF};
    static constexpr  int32_t    kMaxCycle       {4};
    static constexpr  int32_t    kMaxCycle64     {8};
    static constexpr  int32_t    kDataSize       {8};
    static constexpr  int32_t    kShiftMultiByteOffset {16};
  
    RBCP::UDPRBCP&    udp_rbcp_;

    DataType          rd_data_;
    uint32_t          rd_word_;
    uint64_t          rd_word64_;
  }; // End of Class definition
};// End of namespace HUL

#endif
