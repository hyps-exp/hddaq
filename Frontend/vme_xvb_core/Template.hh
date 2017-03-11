#ifndef TEMPLATE_HH
#define TEMPLATE_HH

#include <iterator>
#include <vector>

namespace vme
{

template <typename List>
void
DeleteModule( List& c )
{
  typename List::const_iterator itr, end;
  for( itr=c.begin(), end=c.end(); itr!=end; ++itr ){
    delete (*itr);
  }
}

template <typename List>
void
OpenModule( List& c )
{
  typename List::const_iterator itr, end;
  for( itr=c.begin(), end=c.end(); itr!=end; ++itr ){
    (*itr)->Open();
  }
}

}

#endif
