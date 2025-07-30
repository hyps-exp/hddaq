// -*- C++ -*-

#include "EventReader.hh"

#include <iostream>
#include <regex>
#include <sstream>

#include "FuncName.hh"
#include "MessageHelper.h"

//_____________________________________________________________________________
EventReader::EventReader( void )
  : m_is_good( true ),
    m_stream_path(),
    m_istream(),
    m_tellg( 0 ),
    m_counter( 0 ),
    m_event_counter( 0 ),
    m_print_cycle( 500 ),
    m_asad_flag(),
    m_run_number(),
    m_header( new GetHeader ),
    m_event_id(),
    m_cobo_id(),
    m_asad_id(),
    m_n_data(),
    m_revisions(),
    m_event_time(),
    m_frame_size(),
    m_frame_type(),
    m_header_size(),
    m_item_size(),
    m_padded(),
    m_increment_event( false ),
    m_event_id_offset( -1 ),
    m_cobo_header( new CoBoMasterHeader ),
    m_event_buf(),
    m_header_buf()
{
  m_cobo_header->m_magic = kCoBoMasterMagic;
}

//_____________________________________________________________________________
EventReader::~EventReader( void )
{
  if( m_istream ){
    m_istream->close();
    delete m_istream;
  }
  if( m_header ){
    delete m_header;
  }
}

//_____________________________________________________________________________
void
EventReader::CheckHeaderFormat( void )
{
  if( m_revisions != Revisions ){
    std::ostringstream oss;
    oss << "unknown revisions=" << m_revisions
	<< " (must be " << Revisions << ")";
    send_warning_message( oss.str() );
    m_is_good = false;
  }
  if( NumOfCobo < m_cobo_id ){
    std::ostringstream oss;
    oss << " unknown CoboId=" << m_cobo_id;
    send_warning_message( oss.str() );
    m_is_good = false;
  }
  if( NumOfAsAd < m_asad_id ){
    std::ostringstream oss;
    oss << " unknown AsAdId=" << m_asad_id;
    send_warning_message( oss.str() );
    m_is_good = false;
  }
  if( m_is_good && m_event_id_offset < 0 ){
    m_event_id_offset = m_event_id - 1;
  }
}

//_____________________________________________________________________________
void
EventReader::Clear( void )
{
  m_is_good = true;
  m_tellg = 0;
  m_counter = 0;
  m_event_counter = 0;
  m_increment_event = false;
  m_event_id_offset = -1;
  m_asad_flag.clear();
  m_event_buf.clear();
}

//_____________________________________________________________________________
void
EventReader::Decode( void )
{
}

//_____________________________________________________________________________
void
EventReader::DecodeHeader( void )
{
  m_event_id = (m_header->EventIdx[0] << 24)
    | (m_header->EventIdx[1] << 16)
    | (m_header->EventIdx[2] << 8)
    | (m_header->EventIdx[3]);
  m_cobo_id = m_header->CoboIdx;
  m_asad_id = m_header->AsadIdx;
  m_n_data = (m_header->nItems[0] << 24)
    | (m_header->nItems[1] << 16)
    | (m_header->nItems[2] << 8)
    | (m_header->nItems[3]);
  m_event_time = (m_header->EventTime[0] << 24)
    | (m_header->EventTime[1] << 16)
    | (m_header->EventTime[2] << 8)
    | (m_header->EventTime[3]);
  m_revisions = m_header->Revision;
  m_frame_size = (m_header->FrameSize[0] << 16)
    | (m_header->FrameSize[1] << 8)
    | (m_header->FrameSize[2]);
  m_header_size = (m_header->HeaderSize[0] << 8)
    | (m_header->HeaderSize[1]);
  m_item_size = (m_header->ItemSize[0] << 8)
    | (m_header->ItemSize[1]);
  m_padded = (m_frame_size - m_header_size)*256 - m_item_size*m_n_data;
  m_frame_type = m_header->FrameType[1];
}

//_____________________________________________________________________________
bool
EventReader::eof( void ) const
{
  bool is_end = !m_is_good
    || !m_istream
    || !m_istream->is_open()
    || m_istream->fail()
    || m_istream->eof();
  return is_end;
}

//_____________________________________________________________________________
void
EventReader::IncrementEvent( void )
{
  if( eof() )
    return;

  // m_cobo_header->m_magic = kCoBoMasterMagic;
  m_cobo_header->m_cobo_id = m_cobo_id;
  m_cobo_header->m_data_size = sizeof(CoBoMasterHeader)/sizeof(uint32_t);
  m_cobo_header->m_nblock = 0;
  uint32_t evnum = m_event_id-m_event_id_offset-1;
  for( int i=0; i<NumOfAsAd; ++i ){
    const uint32_t s = m_event_buf[evnum][i].size();
    m_cobo_header->m_data_size += s;
    m_cobo_header->m_nblock += ( s > 0 );
  }

  m_increment_event = true;
  m_event_counter++;
  if( evnum > 1 ){
    m_event_buf.erase( evnum-1 );
  }
  if( m_print_cycle > 0 && m_event_counter > 0 &&
      ( m_event_counter % m_print_cycle ) == 0 ){
    PrintEventNumber();
  }
}

//_____________________________________________________________________________
bool
EventReader::Initialize( void )
{
  Clear();
  // ++(*this);
  // std::cout << FUNC_NAME << " " << m_stream_path
  // 	    << " ... initialized" << std::endl;
  return true;
}

//_____________________________________________________________________________
void
EventReader::PrintEventNumber( void ) const
{
  std::stringstream ss;
  ss << m_event_id << " (" << std::hex << std::showbase << m_event_id << ")";
  std::cout << FUNC_NAME << " " << m_stream_path << std::endl
	    << "   (Cobo,AsAd)     " << m_cobo_id << ","
	    << m_asad_flag.find( m_event_id )->second << std::endl
            << "   (get event id)  " << ss.str() << " (offset="
	    << m_event_id_offset << ")"
	    << std::endl
            << "   (event counter) " << m_event_counter
            << " (" << std::hex << std::showbase << m_event_counter
            << std::dec << std::noshowbase << ")" << std::endl
            << "   (counter)       " << m_counter
            << " (" << std::hex << std::showbase << m_counter
	    << std::dec << std::noshowbase << ")" << std::endl
	    << "   (tellg)         " << m_tellg << std::endl;
}

//_____________________________________________________________________________
void
EventReader::PrintHeader( void ) const
{
  if( !m_header )
    return;
  std::string frame_type = ( m_header->FrameType[1] == 1 ) ? "Partial"
    : ( m_header->FrameType[1] == 2 ) ? "Full" : "Unknown";
  std::ostringstream oss;
  oss << FUNC_NAME << std::endl
      << "   MetaType = " << (int)m_header->MetaType << std::endl
      << "   FrameSize = 0x"
      << std::hex << (int)m_header->FrameSize[0]
      << (int)m_header->FrameSize[1]
      << (int)m_header->FrameSize[2] << std::dec
      << " Blocks (256 bytes/block)" << std::endl
      << "   DataSource = " << (int)m_header->DataSource << std::endl
      << "   FrameType = " << (int)m_header->FrameType[1] << " "
      << frame_type << std::endl
      << "   Revision = " << m_revisions << std::endl
      << "   HeaderSize = " << (int)m_header->HeaderSize[1]
      << " Blocks" << std::endl
      << "   ItemSize = " << (int)m_header->ItemSize[1] << std::endl
      << "   n data = " << m_n_data << std::endl
      << "   Event Id = " << m_event_id
      << ", Cobo Id = " << m_cobo_id
      << ", AsAd Id = " << m_asad_id << std::endl
      << "   ReadOffset = " << (int)m_header->ReadOffset[1] << std::endl
      << "   Status = " << (int)m_header->Status << std::endl
      << "   Padded = " << m_padded << std::endl;
  send_normal_message( oss.str() );
}

//_____________________________________________________________________________
void
EventReader::ReadOneBlock( void )
{
  if( eof() )
    return;
  m_istream->read( reinterpret_cast<char*>( m_header ), sizeof(GetHeader) );
  DecodeHeader();
  CheckHeaderFormat();
  if( !m_is_good )
    return;
  uint32_t evnum = m_event_id-m_event_id_offset-1;
  switch( m_frame_type ){
  case GetHeader::kPartialRead: {
    if( m_event_buf[evnum].empty() ){
      m_event_buf[evnum].resize( NumOfAsAd );
    }
    uint32_t total_data_size = ( m_n_data + sizeof(GetHeader)/sizeof(uint32_t)
				 + 2 ); // magic+size
    m_event_buf[evnum][m_asad_id].resize( total_data_size );
    auto itr = m_event_buf[evnum][m_asad_id].begin();
    *(itr++) = kAsAdMagic;
    *(itr++) = total_data_size;
    std::memcpy( &( *itr ), m_header, sizeof(GetHeader) );
    m_istream->read( reinterpret_cast<char*>
		     ( &( *( itr + sizeof(GetHeader)/sizeof(uint32_t) ) ) ),
                     sizeof(uint32_t)*m_n_data );
    m_asad_flag[m_event_id].set( m_asad_id );
    break;
  }
  case GetHeader::kFullRead: {
    send_fatal_message( "not support full read mode!" );
    std::exit( EXIT_FAILURE );
  }
  default:
    std::cerr << FUNC_NAME << " unknown frame type=" << m_frame_type
              << std::endl;
    m_is_good = false;
    return;
  }
  ++m_counter;
  if( m_asad_flag[m_event_id].count() == NumOfAsAd ||
      ( m_cobo_id == 7 && m_asad_flag[m_event_id].count() == NumOfAsAd-1 ) ){
    IncrementEvent();
  }
  m_istream->seekg( m_padded, std::ios::cur );
  m_tellg = m_istream->tellg();
}

//_____________________________________________________________________________
void
EventReader::Run( void )
{
}

//_____________________________________________________________________________
void
EventReader::seekg( std::streamsize p )
{
  if( p > 0 )
    m_istream->seekg( p, std::ios::beg );
  // else
  //   m_istream->seekg( -4, std::ios::end );
}

//_____________________________________________________________________________
void
EventReader::SetStream( const std::string& stream )
{
  // std::cout << FUNC_NAME << " " << stream << std::endl;
  m_stream_path = stream;
  if( m_istream ){
    m_istream->close();
    delete m_istream;
  }
  m_istream = new std::ifstream( stream );
  m_istream->ignore( HeaderOffset, std::char_traits<char>::eof() );
  std::string run_number( ::basename( stream.c_str() ) );
  for( const auto& pattern :
	 std::vector<std::string>{ "run_", "run", ".dat", ".gz" } ){
    run_number = std::regex_replace( run_number, std::regex( pattern ), "" );
  }
  m_run_number = std::atoi( run_number.c_str() );
}

//_____________________________________________________________________________
bool
EventReader::Next( void )
{
  uint32_t counter = m_counter;
  if( eof() )
    return false;
  // Clear();
  while( !eof() && !m_increment_event ){
    ReadOneBlock();
  }
  m_increment_event = false;
  // read();
  if( eof() ){
    // if( m_counter > 0 )
    //   PrintEventNumber();
    m_counter = counter;
    return false;
  }
  Decode();
  return true;
}
