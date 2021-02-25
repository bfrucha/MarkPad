//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_unordered_set
#define jsonserial_unordered_set

#include <unordered_set>
#include "./set.hpp"

namespace ccuty {
  
  template <class T, class Comp, class Alloc>
  struct is_std_set<std::unordered_set<T,Comp,Alloc>> : std::true_type {};

  template <class T, class Comp, class Alloc>
  struct is_std_set<std::unordered_multiset<T,Comp,Alloc>> : std::true_type {};
}

#endif
