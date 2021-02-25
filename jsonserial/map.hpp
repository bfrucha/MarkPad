//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_map
#define jsonserial_map

#include <map>

namespace ccuty {
  
  template <class Key, class T, class Comp, class Alloc>
  struct is_std_map<std::map<Key,T,Comp,Alloc>> : std::true_type {};

  template <class Key, class T, class Comp, class Alloc>
  struct is_std_map<std::multimap<Key,T,Comp,Alloc>> : std::true_type {};
  
}
#endif
