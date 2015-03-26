// Author: Tomonori Takahashi

#ifndef HDDAQ__O_GZ_FILE_STREAM_H
#define HDDAQ__O_GZ_FILE_STREAM_H

#include "GZFileBuf.hh"
#include "output_stream.hh"

namespace hddaq
{
  namespace unpacker
  {
    typedef basic_output_stream<basic_gz_filebuf, char> OGZFileStream;
  }
}
#endif
