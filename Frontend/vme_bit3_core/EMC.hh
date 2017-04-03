// -*- C++ -*-

// Author: Shuhei Hayakawa

#ifndef VME_EMC_HH
#define VME_EMC_HH

#include <stdint.h>
#include <string>
#include <vector>

#include "VmeModule.hh"

namespace vme
{

//______________________________________________________________________________
class EMC : public VmeModule
{
public:
  EMC( uint32_t addr );
  ~EMC( void );

  enum DataType
    {
      Header,
      Serial,
      Xpos,
      Ypos,
      State,
      Utime,
      Ltime,
      Footer,
      NDataType
    };

private:
  EMC( const EMC& );
  EMC& operator =( const EMC& );

private:
  std::vector<uint32_t> m_data;

  enum FileParam
    {
      kSerial,
      kXpos,
      kYpos,
      kState,
      kTime,
      kMagic,
      kNFileParam
    };

  static const uint32_t OffsetX = 500000 - 303300;
  static const uint32_t OffsetY = 500000 + 164000;

  static const uint32_t kDataMask = 0xfffffffU;
  static const uint32_t kWordTypeShift = 28U;
  static const uint32_t kWordTypeMask  = 0xfU;
  static const uint32_t kHeaderMagic = 0xaU;
  static const uint32_t kSerialMagic = 0x0U;
  static const uint32_t kXposMagic   = 0x1U;
  static const uint32_t kYposMagic   = 0x2U;
  static const uint32_t kStateMagic  = 0x3U;
  static const uint32_t kUtimeMagic  = 0x4U;
  static const uint32_t kLtimeMagic  = 0x5U;
  static const uint32_t kFooterMagic = 0xeU;

public:

  static const std::string& ClassName( void );
  void     Close( void );
  void     Open( void );
  uint32_t ReadData( uint32_t i ) const;
  bool     ReadFile( void );
  void     WriteRegister( uint32_t reg, uint32_t val );
  void     Print( void ) const;

};

//______________________________________________________________________________
inline const std::string&
EMC::ClassName( void )
{
  static std::string g_name("EMC");
  return g_name;
}

//______________________________________________________________________________
inline uint32_t
EMC::ReadData( uint32_t i ) const
{
  return m_data[i];
}

}

#endif
