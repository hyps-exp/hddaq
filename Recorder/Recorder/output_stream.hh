// Author: Tomonori Takahashi

#ifndef HDDAQ__OUTPUT_STREAM_H
#define HDDAQ__OUTPUT_STREAM_H

#include <ostream>
#include <bits/char_traits.h>

namespace hddaq
{
  namespace unpacker
  {

    template
    <
      template <typename, typename> class StreamBuf,
      typename CharT,
      typename Traits = std::char_traits<CharT>
      >
    class basic_output_stream
      : public std::basic_ostream<CharT, Traits>
    {

    public:
      typedef CharT                          char_type;
      typedef Traits                         traits_type;
      typedef typename traits_type::int_type int_type;
      typedef typename traits_type::pos_type pos_type;
      typedef typename traits_type::off_type off_type;

      typedef StreamBuf<char_type, traits_type>          buffer_type;
      typedef std::basic_ostream<char_type, traits_type> ostream_type;

    private:
      buffer_type m_buffer;

    public:
      basic_output_stream();
      explicit  basic_output_stream(const char* s,
				    std::ios_base::openmode mode
				    = std::ios_base::out
				    | std::ios_base::trunc);
      ~basic_output_stream();

      void          close();
      bool          is_open();
      bool          is_open() const;
      void          open(const char* s,
			 std::ios_base::openmode mode
			 = std::ios_base::out
			 | std::ios_base::trunc);
      buffer_type* rdbuf() const;

    };

  //______________________________________________________________________________
  template
  <
    template <typename, typename> class StreamBuf,
    typename CharT,
    typename Traits
    >
  basic_output_stream<StreamBuf, CharT, Traits>::basic_output_stream()
    : ostream_type(0),
      m_buffer()
    {
      this->init(&m_buffer);
    }

//______________________________________________________________________________
template
<
  template <typename, typename> class StreamBuf,
  typename CharT,
  typename Traits
  >
basic_output_stream<StreamBuf, CharT, Traits>::
basic_output_stream(const char* s,
		    std::ios_base::openmode mode)
  : ostream_type(0),
    m_buffer()
{
  this->init(&m_buffer);
  this->open(s, mode);
}

    //______________________________________________________________________________
    template
    <
      template <typename, typename> class StreamBuf,
      typename CharT,
      typename Traits
      >
    basic_output_stream<StreamBuf, CharT, Traits>::~basic_output_stream()
      {
      }

  //______________________________________________________________________________
  template
  <
    template <typename, typename> class StreamBuf,
    typename CharT,
    typename Traits
    >
  void
  basic_output_stream<StreamBuf, CharT, Traits>::close()
    {
      if (!m_buffer.close())
	this->setstate(std::ios_base::failbit);
      return;
    }

//______________________________________________________________________________
template
<
  template <typename, typename> class StreamBuf,
  typename CharT,
  typename Traits
  >
bool
basic_output_stream<StreamBuf, CharT, Traits>::is_open()
{
  return m_buffer.is_open();
}

    //______________________________________________________________________________
    template
    <
      template <typename, typename> class StreamBuf,
      typename CharT,
      typename Traits
      >
    bool
    basic_output_stream<StreamBuf, CharT, Traits>::is_open() const
      {
	return m_buffer.is_open();
      }

  //______________________________________________________________________________
  template
  <
    template <typename, typename> class StreamBuf,
    typename CharT,
    typename Traits
    >
  void
  basic_output_stream<StreamBuf, CharT, Traits>::open(const char* s,
						      std::ios_base::openmode mode)
    {
      if (!m_buffer.open(s, mode | std::ios_base::out))
	this->setstate(std::ios_base::failbit);
      else
	this->clear();

      return;
    }

//______________________________________________________________________________
template
<
  template <typename, typename> class StreamBuf,
  typename CharT,
  typename Traits
  >
typename basic_output_stream<StreamBuf, CharT, Traits>::buffer_type*
basic_output_stream<StreamBuf, CharT, Traits>::rdbuf() const
{
  return const_cast<buffer_type*>(&m_buffer);
}

}
}
#endif


