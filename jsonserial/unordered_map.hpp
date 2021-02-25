//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_unordered_map
#define jsonserial_unordered_map

#include <unordered_map>
#include "./map.hpp"

namespace ccuty {
  
  template <class Key, class T, class Comp, class Alloc>
  struct is_std_map<std::unordered_map<Key,T,Comp,Alloc>> : std::true_type {};

  template <class Key, class T, class Comp, class Alloc>
  struct is_std_map<std::unordered_multimap<Key,T,Comp,Alloc>> : std::true_type {};
}
#endif
