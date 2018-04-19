// Author: Tomonori Takahashi

#ifndef HDDAQ__GZ_FILE_BUF_H
#define HDDAQ__GZ_FILE_BUF_H

#include <streambuf>
#include <bits/char_traits.h>
#include <string>
#include <vector>
#include <zlib.h>

#include <iostream>

namespace hddaq
{
  namespace unpacker
  {

    template<typename CharT, typename Traits = std::char_traits<CharT> >
    class basic_gz_filebuf
      : public std::basic_streambuf<CharT, Traits>
    {

      //   static const unsigned int k_Buffer_Size = 8192;
      static const unsigned int k_Buffer_Size = 1024*1024*16;

    public:
      typedef CharT                     	        char_type;
      typedef Traits                    	        traits_type;
      typedef typename traits_type::int_type      int_type;
      typedef typename traits_type::pos_type 	pos_type;
      typedef typename traits_type::off_type 	off_type;

      typedef std::basic_streambuf<char_type, traits_type>  streambuf_type;
      typedef basic_gz_filebuf<char_type, traits_type>      filebuf_type;

      friend class std::ios_base; // For sync_with_stdio.

    protected:
      gzFile                  m_file;
      std::ios_base::openmode m_mode;
      int                     m_level;
      int                     m_strategy;
      char_type*              m_buf;
      std::size_t             m_buf_size;
      bool                    m_buf_allocated;
      bool                    m_reading;
      bool                    m_writing;
      char_type	            m_pback;
      char_type*              m_pback_cur_save;
      char_type*              m_pback_end_save;
      bool		    m_pback_init;
      //     char_type*           m_ext_buf;
      //     std::streamsize      m_ext_buf_size;
      //     const char*	    m_ext_next;
      //     char*		    m_ext_end;

    public:
      basic_gz_filebuf();
      virtual ~basic_gz_filebuf();

      filebuf_type* close() throw();
      bool          is_open() const throw();
      filebuf_type* open(const char* s,
			 std::ios_base::openmode mode);
      void          set_compression(int level,
				    int strategy = Z_DEFAULT_STRATEGY);

    protected:
      void                    allocate_internal_buffer();
      bool                    convert_to_external(char_type* s,
						  std::streamsize n);
      void                    create_pback();
      void                    destroy_internal_buffer() throw();
      void                    destroy_pback() throw();
      virtual int_type        overflow(int_type c = Traits::eof());
      virtual int_type        pbackfail(int_type c = Traits::eof());
      pos_type                seek(off_type off,
				   std::ios_base::seekdir way);
      virtual pos_type        seekoff(off_type off,
				      std::ios_base::seekdir way,
				      std::ios_base::openmode mode
				      = std::ios_base::in | std::ios_base::out);
      virtual pos_type        seekpos(pos_type pos,
				      std::ios_base::openmode mode
				      = std::ios_base::in | std::ios_base::out);
      virtual streambuf_type* setbuf(char_type* s,
				     std::streamsize n);
      void                    set_buffer(std::streamsize off);
      virtual std::streamsize showmanyc();
      virtual int             sync();
      virtual int_type        underflow();
      bool                    terminate_output();
      //     virtual std::streamsize  xsgetn(char_type* s, std::streamsize n);
      //     virtual std::streamsize  xsputn(const char_type* s, std::streamsize n);

    };

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    basic_gz_filebuf<CharT, Traits>::basic_gz_filebuf()
      : streambuf_type(),
	m_file(0),
	m_mode(std::ios_base::openmode(0)),
	// m_level(Z_DEFAULT_COMPRESSION),
	m_level(Z_BEST_SPEED),
	m_strategy(Z_DEFAULT_STRATEGY),
	m_buf(0),
	m_buf_size(k_Buffer_Size),
	m_buf_allocated(false),
	m_reading(false),
	m_writing(false),
	m_pback(),
	m_pback_cur_save(0),
	m_pback_end_save(0),
	m_pback_init(false)
	//     m_ext_buf(0),
	//     m_ext_buf_size(0),
	//     m_ext_next(0),
	//     m_ext_end(0)
    {
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    basic_gz_filebuf<CharT, Traits>::~basic_gz_filebuf()
    {
      this->close();
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    void
    basic_gz_filebuf<CharT, Traits>::allocate_internal_buffer()
    {
      if (!m_buf_allocated && !m_buf)
	{
	  m_buf = new char_type[m_buf_size];
	  m_buf_allocated = true;
	}
      return;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::filebuf_type*
    basic_gz_filebuf<CharT, Traits>::close() throw()
    {
      filebuf_type* ret=0;
      if (this->is_open())
	{
	  if (0==this->sync())
	    {
	      if (::gzclose(m_file) >= 0)
		ret = this;
	      m_file = 0;
	    }
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    bool
    basic_gz_filebuf<CharT, Traits>::convert_to_external(char_type* s,
							 std::streamsize n)
    {
      std::streamsize elen;
      std::streamsize plen;
      elen = ::gzwrite(m_file,
		       reinterpret_cast<char*>(s),
		       n);
      plen = n;
      return (elen == plen);
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    void
    basic_gz_filebuf<CharT, Traits>::create_pback()
    {
      if (!m_pback_init)
	{
	  m_pback_cur_save = this->gptr();
	  m_pback_end_save = this->egptr();
	  this->setg(&m_pback, &m_pback, &m_pback + 1);
	  m_pback_init = true;
	}
      return;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    void
    basic_gz_filebuf<CharT, Traits>::destroy_internal_buffer() throw()
    {
      if (m_buf_allocated)
	{
	  if (m_buf)
	    {
	      delete [] m_buf;
	      m_buf = 0;
	    }
	  m_buf_allocated = false;
	}

      //   delete [] m_ext_buf;
      //   m_ext_buf = 0;
      //   m_ext_buf_size = 0;
      //   m_ext_next     = 0;
      //   m_ext_end      = 0;
      return;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    void
    basic_gz_filebuf<CharT, Traits>::destroy_pback() throw()
    {
      if (m_pback_init)
	{
	  m_pback_cur_save += (this->gptr() != this->eback());
	  this->setg(m_buf, m_pback_cur_save, m_pback_end_save);
	  m_pback_init = false;
	}

      return;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    bool
    basic_gz_filebuf<CharT, Traits>::is_open() const throw()
    {
      return (0 != m_file);
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::filebuf_type*
    basic_gz_filebuf<CharT, Traits>::open(const char* s,
					  std::ios_base::openmode mode)
    {
      filebuf_type* ret = 0;
      if (!this->is_open())
	{
	  std::string open_mode;
	  if (std::ios_base::in & mode)
	    open_mode += "r";
	  else if (std::ios_base::app & mode)
	    open_mode += "a";
	  else if (std::ios_base::out & mode)
	    open_mode += "w";

	  if (std::ios_base::binary & mode)
	    open_mode += "b";

	  m_file = ::gzopen(s, open_mode.c_str());
	}

      if (this->is_open())
	{
	  allocate_internal_buffer();
	  m_mode    = mode;
	  m_reading = false;
	  m_writing = false;
	  set_buffer(-1);

	  ret = this;
	}
      return ret;

    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::int_type
    basic_gz_filebuf<CharT, Traits>::overflow(int_type c)
    {
      //   std::cout << "#D buf size = " << m_buf_size << std::endl;
      int_type ret = traits_type::eof();
      const bool test_eof = traits_type::eq_int_type(c, ret);
      const bool test_out = m_mode & std::ios_base::out;
      if (test_out && !m_reading)
	{
	  if (this->pbase() < this->pptr())
	    {
	      if (!test_eof)
		{
		  *this->pptr() = traits_type::to_char_type(c);
		  this->pbump(1);
		}
	      if (convert_to_external(this->pbase(),
				      this->pptr() - this->pbase()))
		{
		  set_buffer(0);
		  ret = traits_type::not_eof(c);
		}
	    }
	  else if (m_buf_size > 1)
	    {
	      set_buffer(0);
	      m_writing = true;
	      if (!test_eof)
		{
		  *this->pptr() = traits_type::to_char_type(c);
		  this->pbump(1);
		}
	      ret = traits_type::not_eof(c);
	    }
	  else
	    {
	      char_type conv = traits_type::to_char_type(c);
	      if (test_eof || convert_to_external(&conv, 1))
		{
		  m_writing = true;
		  ret = traits_type::not_eof(c);
		}
	    }
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::int_type
    basic_gz_filebuf<CharT, Traits>::pbackfail(int_type c)
    {
      int_type ret = traits_type::eof();
      const bool test_in = m_mode & std::ios_base::in;
      if (test_in && !m_writing)
	{
	  const bool test_pb  = m_pback_init;
	  const bool test_eof = traits_type::eq_int_type(c, ret);
	  int_type tmp;
	  if (this->eback() < this->gptr())
	    {
	      this->gbump(-1);
	      tmp = traits_type::to_int_type(*this->gptr());
	    }
	  else if (this->seekoff(-1, std::ios_base::cur) != pos_type(off_type(-1)))
	    {
	      tmp = this->underflow();
	      if (traits_type::eq_int_type(tmp, ret))
		return ret;
	    }
	  else
	    {
	      return ret;
	    }

	  if (!test_eof && traits_type::eq_int_type(c, tmp))
	    ret = c;
	  else if (test_eof)
	    ret = traits_type::not_eof(c);
	  else if (!test_pb)
	    {
	      create_pback();
	      m_reading = true;
	      *this->gptr() = traits_type::to_char_type(c);
	      ret = c;
	    }
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::pos_type
    basic_gz_filebuf<CharT, Traits>::seek(off_type off,
					  std::ios_base::seekdir way)
    {
      pos_type ret = pos_type(off_type(-1));
      if (terminate_output())
	{
	  int whence = 0;
	  if (std::ios_base::beg == way)
	    whence = SEEK_SET;
	  else if (std::ios_base::cur == way)
	    whence = SEEK_CUR;
	  else if (std::ios_base::end == way)
	    whence = SEEK_END;
	  ret = pos_type(::gzseek(m_file, off, whence));
	  if (ret != pos_type(off_type(-1)))
	    {
	      ret = pos_type(::gzseek(m_file, off, whence));
	      m_reading = false;
	      m_writing = false;
	      //	  m_ext_next = m_ext_end = m_ext_buf;
	      set_buffer(-1);

	    }
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::pos_type
    basic_gz_filebuf<CharT, Traits>::seekoff(off_type off,
					     std::ios_base::seekdir way,
					     std::ios_base::openmode mode)
    {
      int width = 0;
      pos_type ret = pos_type(off_type(-1));
      const bool test_fail = (0 != off && width <=0);
      if (this->is_open() && !test_fail)
	{
	  destroy_pback();
	  off_type computed_off = off * width;
	  if (m_reading && way == std::ios_base::cur)
	    {
	      computed_off += this->gptr() - this->egptr();
	    }
	  ret = seek(computed_off, way);
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::pos_type
    basic_gz_filebuf<CharT, Traits>::seekpos(pos_type pos,
					     std::ios_base::openmode mode)
    {
      pos_type ret = pos_type(off_type(-1));
      if (this->is_open())
	{
	  destroy_pback();
	  ret = seek(off_type(pos), std::ios_base::beg);
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::streambuf_type*
    basic_gz_filebuf<CharT, Traits>::setbuf(char_type* s,
					    std::streamsize n)
    {
      if (!this->is_open()) {
	if (0==s && 0==n)
	  {
	    m_buf_size = 1;
	  }
	else if (s && n > 0)
	  {
	    m_buf      = s;
	    m_buf_size = n;
	  }
      }
      return this;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    void
    basic_gz_filebuf<CharT, Traits>::set_buffer(std::streamsize off)
    {
      const bool test_in  = m_mode & std::ios_base::in;
      const bool test_out = m_mode & std::ios_base::out;

      if (test_in && off > 0)
	this->setg(m_buf, m_buf, m_buf + off);
      else
	this->setg(m_buf, m_buf, m_buf);

      if (test_out && off == 0 && m_buf_size > 1 )
	this->setp(m_buf, m_buf + m_buf_size - 1);
      else
	this->setp(0, 0);
      return;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    void
    basic_gz_filebuf<CharT, Traits>::set_compression(int level, int strategy)
    {
      m_level    = level;
      m_strategy = strategy;
      return;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    std::streamsize
    basic_gz_filebuf<CharT, Traits>::showmanyc()
    {
      std::streamsize ret = -1;
      const bool test_in = m_mode & std::ios_base::in;
      if (test_in && this->is_open())
	ret = this->egptr() - this->gptr();
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    int
    basic_gz_filebuf<CharT, Traits>::sync()
    {
      int ret = 0;
      if (this->pbase() < this->pptr())
	{
	  const int_type tmp = this->overflow();
	  if (traits_type::eq_int_type(tmp, traits_type::eof()))
	    ret = -1;
	}
      return ret;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    bool
    basic_gz_filebuf<CharT, Traits>::terminate_output()
    {
      bool test_valid = true;

      if (this->pbase() < this->pptr())
	{
	  const int_type tmp = this->overflow();
	  if (traits_type::eq_int_type(tmp, traits_type::eof()))
	    test_valid = false;
	}

      if (m_writing && test_valid)
	{
	  const int_type tmp = this->overflow();
	  if (traits_type::eq_int_type(tmp, traits_type::eof()))
	    test_valid = false;
	}
      return test_valid;
    }

    //______________________________________________________________________________
    template <typename CharT, typename Traits>
    typename basic_gz_filebuf<CharT, Traits>::int_type
    basic_gz_filebuf<CharT, Traits>::underflow()
    {
      //   std::cout << "#D buf size = " << m_buf_size << std::endl;
      int_type ret = traits_type::eof();
      const bool test_in = m_mode & std::ios_base::in;
      if (test_in && !m_writing)
	{
	  destroy_pback();
	  if (this->gptr() < this->egptr())
	    return traits_type::to_int_type(*this->gptr());

	  const std::size_t buf_len = (m_buf_size > 1) ? (m_buf_size - 1) : 1;
	  bool got_eof = false;
	  std::streamsize nbytes_read = 0;
	  nbytes_read = ::gzread(m_file,
				 reinterpret_cast<char*>(this->eback()),
				 buf_len);
	  if (0==nbytes_read)
	    got_eof = true;

	  if (nbytes_read>0)
	    {
	      set_buffer(nbytes_read);
	      m_reading = true;
	      ret = traits_type::to_int_type(*this->gptr());
	    }
	  else if (got_eof)
	    {
	      set_buffer(-1);
	      m_reading = false;
	    }
	}
      return ret;
    }

    //______________________________________________________________________________
    typedef basic_gz_filebuf<char> GZFileBuf;

  }
}
#endif
