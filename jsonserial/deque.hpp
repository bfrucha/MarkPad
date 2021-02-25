//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_deque
#define jsonserial_deque

#include <deque>
#include "./vector.hpp"

namespace ccuty {
  
  template <class T, class Alloc>
  struct is_std_vector<std::deque<T,Alloc>> : std::true_type {};
  
}
#endif
