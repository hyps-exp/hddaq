// -*- C++ -*-

#ifndef EVENT_READER_HH
#define EVENT_READER_HH

#include <bitset>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "DetectorID.hh"

//_____________________________________________________________________________
struct GetHeader
{
  // unsigned char: 8 bit
  uint8_t MetaType;          //1
  uint8_t FrameSize[3];      //4
  uint8_t DataSource;        //5
  uint8_t FrameType[2];      //7
  uint8_t Revision;          //8
  uint8_t HeaderSize[2];     //10
  uint8_t ItemSize[2];       //12
  uint8_t nItems[4];         //16
  uint8_t EventTime[6];      //22
  uint8_t EventIdx[4];       //26
  uint8_t CoboIdx;           //27
  uint8_t AsadIdx;           //28
  uint8_t ReadOffset[2];     //30
  uint8_t Status;            //31
  uint8_t HitPatAGet0[9];    //40
  uint8_t HitPatAGet1[9];    //49
  uint8_t HitPatAGet2[9];    //58
  uint8_t HitPatAGet3[9];    //67
  uint8_t MultiPlAGet0[2];   //69
  uint8_t MultiPlAGet1[2];   //71
  uint8_t MultiPlAGet2[2];   //73
  uint8_t MultiPlAGet3[2];   //75
  uint8_t WindowOut[4];      //79
  uint8_t LastCellAGet0[2];  //81
  uint8_t LastCellAGet1[2];  //83
  uint8_t LastCellAGet2[2];  //85
  uint8_t LastCellAGet3[2];  //87
  uint8_t reserve1byte;      //88
  uint8_t reserve4byte[168]; //256 <---(42*4=168)
  enum EFrameType { kPartialRead = 1, kFullRead = 2 };
};

//_____________________________________________________________________________
struct CoBoMasterHeader
{
  uint32_t m_magic;
  uint32_t m_cobo_id;
  uint32_t m_data_size;
  uint32_t m_nblock;
};

const uint32_t kCoBoMasterMagic = 0x436f426f; // "Cobo"
const uint32_t kAsAdMagic       = 0x41734164; // "AsAd"
//_____________________________________________________________________________
class EventReader
{
public:
  static std::string& ClassName( void );
  EventReader( void );
  ~EventReader( void );

private:
  EventReader( const EventReader& );
  EventReader& operator =( const EventReader& );

public:
  static const std::streamsize HeaderOffset = 12;
  // static const uint32_t MaxDataLen   = 4e9;
  static const uint32_t Revisions           = 5;
  static const uint32_t AGetShift           = 6;
  static const uint32_t AGetMask            = 0x3;
  static const uint32_t AdcHighShiftFull    = 0;
  static const uint32_t AdcHighShiftPartial = 16;
  static const uint32_t AdcHighMask         = 0xf;
  static const uint32_t AdcLowShiftFull     = 8;
  static const uint32_t AdcLowShiftPartial  = 24;
  static const uint32_t AdcLowMask          = 0xff;
  static const uint32_t ChannelHighShift    = 0;
  static const uint32_t ChannelHighMask     = 0x3f;
  static const uint32_t ChannelLowShift     = 15;
  static const uint32_t ChannelLowMask      = 0x1;
  static const uint32_t TimeBucketHighShift = 8;
  static const uint32_t TimeBucketHighMask  = 0x7f;
  static const uint32_t TimeBucketLowShift  = 22;
  static const uint32_t TimeBucketLowMask   = 0x3;
  static const uint32_t TimeBucketHighOffset = 2;
  typedef std::map<uint32_t,std::bitset<NumOfAsAd>> AsadMap;
  typedef std::vector<std::vector<uint32_t>> DataBuf_t; // [asad_id][n_data]

private:
  bool                  m_is_good;
  std::string           m_stream_path;
  std::ifstream*        m_istream;
  uint64_t              m_tellg;
  uint32_t              m_counter;
  uint32_t              m_event_counter;
  uint32_t              m_print_cycle;
  AsadMap               m_asad_flag;
  uint32_t              m_run_number;
  // header
  GetHeader*            m_header;
  uint32_t              m_event_id;
  uint32_t              m_cobo_id;
  uint32_t              m_asad_id;
  uint32_t              m_n_data;
  uint32_t              m_revisions;
  uint32_t              m_event_time;
  uint32_t              m_frame_size;
  uint32_t              m_frame_type;
  uint32_t              m_header_size;
  uint32_t              m_item_size;
  uint32_t              m_padded;
  // data
  bool                  m_increment_event;
  int32_t               m_event_id_offset;
  CoBoMasterHeader*     m_cobo_header;
  std::vector<uint32_t> m_data_buf32; // Partial
  std::vector<uint16_t> m_data_buf16; // Full
  std::vector<uint32_t> m_data_buf;
  std::map<uint32_t, DataBuf_t> m_event_buf; // [event_id]
  std::map<uint32_t, GetHeader*> m_header_buf; // [event_id]

public:
  typedef std::char_traits<char>::pos_type pos_t;

public:
  void CheckHeaderFormat( void );
  void Clear( void );
  const CoBoMasterHeader* CoBoHeader( void ) const { return m_cobo_header; }
  const DataBuf_t& EventBuf( uint32_t event_number ) const
  { return m_event_buf.at( event_number ); }
  const std::vector<uint32_t>& DataBuf( void ) const { return m_data_buf; }
  const std::vector<uint32_t>& DataBuf32( void ) const { return m_data_buf32; }
  void Decode( void );
  void DecodeHeader( void );
  bool eof( void ) const;
  const std::string& GetStreamPath( void ) const { return m_stream_path; }
  bool HasEventBuf( uint32_t event_number ) const
  { return m_event_buf.count( event_number ) == 1; }
  const GetHeader* HeaderBuf( uint32_t evid ) const { return m_header_buf.at(evid); }
  void IncrementEvent( void );
  bool Initialize( void );
  void Join( void );
  bool Next( void );
  void PrintEventNumber( void ) const;
  void PrintHeader( void ) const;
  bool read( void );
  void ReadOneBlock( void );
  void Run( void );
  void seekg( std::streamsize p );
  void SetPrintCycle( uint32_t c ) { m_print_cycle = c; }
  void SetStream( const std::string& stream );
  pos_t tellg( void ) const { return m_tellg; }
  // void operator ++( void );
};

//_____________________________________________________________________________
inline std::string&
EventReader::ClassName( void )
{
  static std::string s_name("EventReader");
  return s_name;
}

#endif
