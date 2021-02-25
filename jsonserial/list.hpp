//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_list
#define jsonserial_list

#include <list>

namespace ccuty {
  
  template <class T, class Alloc>
  struct is_std_list<std::list<T,Alloc>> : std::true_type {};

  template<class T>
  struct JsonArrayImpl<T, typename std::enable_if<is_std_list<T>::value>::type> : public JsonArray {
    T& cont_;
    
    JsonArrayImpl(T& cont) : cont_(cont) {cont_.clear();}
    
    void add(JsonSerial& js, MetaClass::Creator* cr, const std::string& s) override {
      cont_.resize(cont_.size()+1);
      ObjectPtr* objptr{nullptr};
      readArrayValue(js, cont_.back(), objptr, cr, s);
    }
  };
  
}
#endif
