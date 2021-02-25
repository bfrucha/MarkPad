//
//  JsonSerial: C++ Object Serialization in JSON.
//  Definitions
//  (C) Eric Lecolinet 2017/2018 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsondefs_hpp
#define jsondefs_hpp

namespace ccuty {

  /// is this objet a C++ array (not to be confused with C-style bracketed arrays)?.
  template <class T> struct is_std_array : std::false_type {};
  
  /// is this objet a list?.
  template <class T> struct is_std_list : std::false_type {};
  
  /// is this objet a forward_list?.
  template <class T> struct is_std_forward_list : std::false_type {};
  
  /// is this object a map or an unordered_map?.
  template <class T> struct is_std_map : std::false_type {};
  
  /// is this objet a set or an unordered_set?.
  template <class T> struct is_std_set : std::false_type {};
  
  /// is this object a vector or a deque?.
  template <class T> struct is_std_vector : std::false_type {};
  
  // is this object formatted as a JSON array?.
  template <class T> struct has_array_format {
    static constexpr bool value = is_std_array<T>::value || is_std_list<T>::value
    || is_std_forward_list<T>::value || is_std_vector<T>::value || is_std_set<T>::value;
  };
  
  /// is this object a smart pointer (std::shared_ptr and std::unique_ptr)?.
  template <class T> struct is_smart_ptr : std::false_type {};
  template <class T> struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};
  template <class T> struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};
  template <class T> struct is_smart_ptr<std::weak_ptr<T>> : std::true_type {};
  
  /* is this object a "defobject"?.
   * a defobject is a C++ object that must be defined using JsonClasses::defclass()
   * if it is serialized (runtime error otherwise)
   */
  template <class T> struct is_defobject {
    static constexpr bool value = std::is_class<T>::value
    && !std::is_base_of<std::string, T>::value
    && !is_smart_ptr<T>::value && !has_array_format<T>::value && !is_std_map<T>::value;
  };
 
  /// Obtains the pointer type corresponding to T,
  template<class T, class Enable = void> struct make_pointer {};
  
  template<class T> struct make_pointer<T, typename std::enable_if<is_smart_ptr<T>::value>::type> {
    typedef typename T::element_type* type;
  };
  
  template<class T> struct make_pointer<T, typename std::enable_if<!is_smart_ptr<T>::value>::type> {
    typedef typename std::conditional<std::is_pointer<T>::value,T, T*>::type type;
  };
  
  /// Obtains the pointer type corresponding to an array or a JsonArray,
  template<class X, class Enable = void> struct make_array_pointer {}; // primary template
  
  template<class X> struct make_array_pointer<X, typename std::enable_if<std::is_array<X>::value>::type> {
    typedef typename make_pointer<typename std::remove_extent<X>::type>::type type;
  };
  
  template<class X> struct make_array_pointer<X, typename std::enable_if<has_array_format<X>::value>::type> {
    typedef typename make_pointer<typename X::value_type>::type type;
  };
 
  struct ObjectPtr {void *raw_{nullptr}, *shared_{nullptr}; bool init_{false};};

}

#endif
