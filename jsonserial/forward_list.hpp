//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_forward_list
#define jsonserial_forward_list

#include <forward_list>

namespace ccuty {
  
  template <class T, class Alloc>
  struct is_std_forward_list<std::forward_list<T,Alloc>>: std::true_type {};

  template<class T>
  struct JsonArrayImpl<T, typename std::enable_if<is_std_forward_list<T>::value>::type> : public JsonArray {
    T& cont_;
    typename T::iterator pos_;
    
    JsonArrayImpl(T& cont) : cont_(cont) {cont_.clear(); pos_ = cont_.before_begin();}
    
    void add(JsonSerial& js, MetaClass::Creator* cr, const std::string& s) override {
      typename T::value_type val;
      ObjectPtr* objptr{nullptr};
      readArrayValue(js, val, objptr, cr, s);
      pos_= cont_.insert_after(pos_, val);
    }
  };
  
}
#endif
