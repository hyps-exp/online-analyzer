/*
  ThreeVector.hh

  2012/1/24
*/

#ifndef THREE_VECTOR_HH
#define THREE_VECTOR_HH

#include <ostream>

#include <TVector3.h>

typedef TVector3 ThreeVector;

//______________________________________________________________________________
inline
std::ostream&
operator <<( std::ostream& ost,
	     const ThreeVector& v )
{
  ost << "(" << v.x() << ", " << v.y()
      << ", " << v.z() << ")";
  return ost;
}

#endif
