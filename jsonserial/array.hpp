//
//  JsonSerial: C++ Object Serialization in JSON.
//  (C) Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonserial_array
#define jsonserial_array

#include <array>

namespace ccuty {
  
  template <class T, size_t N>
  struct is_std_array<std::array<T,N>> : std::true_type {};

  // add element to a std array (dont confuse with std::is_array for C arrays)
  template<class T>
  struct JsonArrayImpl<T, typename std::enable_if<is_std_array<T>::value>::type> : public JsonArray {
    T& array_;
    size_t index_;
    
    JsonArrayImpl(T& array) : array_(array), index_(0) {}
    
    void add(JsonSerial& js, MetaClass::Creator* cr, const std::string& s) override {
      ObjectPtr* objptr{nullptr};
      if (index_ >= array_.size()) js.error(JsonError::CantAddToArray);
      else readArrayValue(js, array_[index_++], objptr, cr, s);
    }
  };
  
}
#endif
