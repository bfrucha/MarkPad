//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_set
#define jsonserial_set

#include <set>

namespace ccuty {
  
  template <class T, class Comp, class Alloc>
  struct is_std_set<std::set<T,Comp,Alloc>> : std::true_type {};

  template <class T, class Comp, class Alloc>
  struct is_std_set<std::multiset<T,Comp,Alloc>> : std::true_type {};
  
  // add element to a set.
  template<class T>
  struct JsonArrayImpl<T, typename std::enable_if<is_std_set<T>::value>::type>
  : public JsonArray {
    T& set_;
    
    JsonArrayImpl(T& set) : set_(set) {set_.clear();}
    
    void add(JsonSerial& js, MetaClass::Creator* cr, const std::string& s) override {
      typename T::value_type val;
      ObjectPtr* objptr{nullptr};
      readArrayValue(js, val, objptr, cr, s);
      set_.insert(val);
    }
  };

}
#endif
