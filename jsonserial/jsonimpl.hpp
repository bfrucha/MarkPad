//
//  JsonSerial: C++ Object Serialization in JSON.
//  Implementation
//  (C) Eric Lecolinet 2017/2018 - https://www.telecom-paristech.fr/~elc
//

#ifndef jsonimpl_hpp
#define jsonimpl_hpp

namespace ccuty {
  
  inline void* readObject(JsonSerial& js,
                          const MetaClass* objclass, const MetaClass* pointerclass,
                          ObjectPtr*& jsp, MetaClass::Creator* cr, void* obj,
                          const std::string& s);
  
  // reads a non-object pointee pointed by a unique_ptr
  template <class E>
  inline void readPointee2(JsonSerial& js,
                           typename std::enable_if<!is_defobject<E>::value,std::unique_ptr<E>>::type & ptr,
                           ObjectPtr *&,
                           MetaClass::Creator*,
                           const std::string& s) {
    ptr.reset(new E{});
    readValue(js, *ptr, s);
  }
  
  // reads an object pointee pointed by a unique_ptr
  template <class E>
  inline void readPointee2(JsonSerial& js,
                           typename std::enable_if<is_defobject<E>::value,std::unique_ptr<E>>::type & ptr,
                           ObjectPtr *& objptr,
                           MetaClass::Creator* cr,
                           const std::string& s) {
    ptr.reset(static_cast<E*>(readObject(js, nullptr, js.getCheckedClass(typeid(E)),
                                         objptr, cr, nullptr, s)));
  }
  
  // read non-object pointee pointed by shared_ptr
  template <class E>
  inline void readPointee2(JsonSerial& js,
                           typename std::enable_if<!is_defobject<E>::value,std::shared_ptr<E>>::type & ptr,
                           ObjectPtr *&,
                           MetaClass::Creator*,
                           const std::string& s) {
    ptr.reset(new E{});
    readValue(js,*ptr, s);
  }
  
  // read object pointee pointed by shared_ptr
  template <class E>
  inline void readPointee2(JsonSerial& js,
                           typename std::enable_if<is_defobject<E>::value,std::shared_ptr<E>>::type & ptr,
                           ObjectPtr *& objptr,
                           MetaClass::Creator* cr,
                           const std::string& s) {
    E* p = static_cast<E*>(readObject(js, nullptr, js.getCheckedClass(typeid(E)),
                                      objptr, cr, nullptr, s));
    if (!objptr) ptr.reset(p);
    else if (objptr->shared_) { // smart_ptr already init
      objptr->init_ = false;
      ptr = *static_cast<std::shared_ptr<E>*>(objptr->shared_);
    }
    else {
      ptr.reset(p);
      objptr->shared_ = &ptr;  // init smart_ptr
      objptr->init_ = true;
    }
  }
  // - - -
  
  // reads a non-object pointed by a raw ptr
  template <class T>
  inline void readPointee(JsonSerial& js,
                          typename std::enable_if<!is_defobject<typename std::remove_reference<T>::type>::value,T>::type *& ptr,
                          ObjectPtr *&,
                          MetaClass::Creator*,
                          const std::string& s) {
    ptr = new T{};
    readValue(js, *ptr, s);
  }
  
  // reads an object pointed by a raw ptr
  template <class T>
  inline void readPointee(JsonSerial& js,
                          typename std::enable_if<is_defobject<typename std::remove_reference<T>::type>::value,T>::type *& ptr,
                          ObjectPtr *& objptr,
                          MetaClass::Creator * cr,
                          const std::string& s) {
    ptr = static_cast<T*>(readObject(js, nullptr, js.getCheckedClass(typeid(T)),
                                     objptr, cr, nullptr, s));
  }
  

  // reads any pointee pointed by a smart pointer
  template <class T>
  inline void readPointee(JsonSerial& js,
                          typename std::enable_if<is_smart_ptr<T>::value,T>::type & ptr,
                          ObjectPtr *& objptr,
                          MetaClass::Creator* cr,
                          const std::string& s) {
    readPointee2<typename T::element_type>(js, ptr, objptr, cr, s);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  // reads a smart pointer.
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<is_smart_ptr<T>::value,T>::type & ptr,
                         const std::string& s) {
    ptr = nullptr;
    ObjectPtr* objptr{nullptr};
    if (s != "null") readPointee<T>(js, ptr, objptr, nullptr, s);
  }
  
  // reads a number of another type than int, long, etc.
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<std::is_arithmetic<T>::value,T>::type & val,
                         const std::string& s) {
    std::istringstream ss(s);
    ss.imbue(js.locale_);
    ss >> val;
  }
  
  // reads an enum.
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<std::is_enum<T>::value,T>::type & e,
                         const std::string& s) {
    e = T(std::stoi(s));
  }
  
  // reads a defobject.
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<is_defobject<T>::value,T>::type & obj,
                         const std::string& s) {
    const MetaClass* wanted_class = js.getCheckedClass(typeid(obj));
    ObjectPtr* objptr{nullptr};
    readObject(js, wanted_class, wanted_class, objptr, nullptr, &obj, s);
  }
  
  // reads a map.
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<is_std_map<T>::value,T>::type & obj,
                         const std::string& s) {
    MapClass<T> wanted_class;
    ObjectPtr* objptr{nullptr};
    readObject(js, &wanted_class, &wanted_class, objptr, nullptr, &obj, s);
  }
  
  // reads a C-array.
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<std::is_array<T>::value,T>::type & array,
                         const std::string& s) {
    JsonArrayImpl<T> a(array);
    readArray(js, a, nullptr, s);
  }
  
  // reads an array_style container
  template <class T>
  inline void readValue2(JsonSerial& js,
                         typename std::enable_if<has_array_format<T>::value,T>::type & array,
                         const std::string& s) {
    JsonArrayImpl<T> a(array);
    readArray(js, a, nullptr, s);
  }
  
  // - - -
  
  // reads a string
  inline void readValue(JsonSerial&, std::string& var, const std::string& s) {var = s;}
  
  inline void readValue(JsonSerial&, char*& var, const std::string& s) {
    var = (s == "null" || !s.c_str()) ? nullptr : ::strdup(s.c_str());
  }
  
  inline void readValue(JsonSerial&, const char*& var, const std::string& s) {
    var = (s == "null" || !s.c_str()) ? nullptr : ::strdup(s.c_str());
  }
  
  // reads a char
  inline void readValue(JsonSerial&, char& var, const std::string& s) {
    var = s.empty() ? 0 : s[0];
  }
  
  // reads a bool
  inline void readValue(JsonSerial& js, bool& var, const std::string& s) {
    if (s == "true") var = true;
    else if (s == "false") var = false;
    else js.error(JsonError::InvalidValue, s+" should be a boolean");
  }
  
  // reads an integral numebr
  inline void readValue(JsonSerial&, int& var, const std::string& s) {var = std::stoi(s);}
  inline void readValue(JsonSerial&, long& var, const std::string& s) {var = std::stol(s);}
  inline void readValue(JsonSerial&, long long& var, const std::string& s) {var = std::stoll(s);}
  inline void readValue(JsonSerial&, unsigned long& var, const std::string& s) {var = std::stoul(s);}
  inline void readValue(JsonSerial&, unsigned long long& var, const std::string& s) {var = std::stoull(s);}
  
  // reads a floating number
  inline void readValue(JsonSerial&, float& var, const std::string& s) {var = std::stof(s);}
  inline void readValue(JsonSerial&, double& var, const std::string& s) {var = std::stod(s);}
  inline void readValue(JsonSerial&, long double& var, const std::string& s) {var = std::stold(s);}
  
  // reads a raw pointer.
  template <class T>
  inline void readValue(JsonSerial& js, T *& ptr, const std::string& s) {
    ptr = nullptr;
    ObjectPtr* objptr{nullptr};
    if (s != "null") readPointee<T>(js, ptr, objptr, nullptr, s);
  }
  
  // reads a value of another type.
  template <class T>
  inline void readValue(JsonSerial& js, T& value, const std::string& s) {
    readValue2<T>(js, value, s);
  }
  
  // - - -
  
  /* reads a defobject.
   * - objclass : class of the object, may be null (see below)
   * - pointerclass : class of the pointer, used as a default if no @type field
   * - obj : points to the object, if null the object is auto-created
   *   taking into account 1) @type if defined, 2) pointerclass
   * - returns a pointer to the object (whether created or not)
   */
  inline void* readObject(JsonSerial& js,
                          const MetaClass* objclass, const MetaClass* pointerclass,
                          ObjectPtr*& jsp, MetaClass::Creator* cr, void* obj,
                          const std::string& s) {
    if (s.empty()) js.error(JsonError::ExpectingBrace);
    else if (s[0] == '@') {  // shared object
      auto it = js.id_to_object_.find(std::strtoul(s.c_str()+1, nullptr, 0));
      if (it == js.id_to_object_.end()) js.error(JsonError::InvalidID);
      jsp = &it->second;
      return obj = it->second.raw_;
    }
    else if (s != "{") js.error(JsonError::ExpectingBrace);
    
    while (js.in_->good()) {
      std::string name, value;
      bool found1, found2;
      js.readLine(name, value, found1, found2, true);
      if (!found1) js.error(JsonError::ExpectingPairOrBrace);
      else if (!found2 && name != "}") js.error(JsonError::ExpectingPairOrBrace);
      
      if (name[0]=='@' && name != "@class" && name != "@id")
        js.error(JsonError::WrongKeyword, value);
      
      if (!objclass) {  // search class
        if (name != "@class") objclass = pointerclass;
        else { // polymorphism
          objclass = js.classes_.getClass(value);
          if (!objclass) js.error(JsonError::UnknownClass, value);
        }
        if (!obj) { // create object if it does not exist
          if (cr) obj = cr->create();
          else obj = objclass->create();
        }
        if (!obj) js.error(JsonError::AbstractClass, objclass->classname());
        if (name == "@class") continue;
      }
      
      if (name == "}") {objclass->doPostRead(obj); return obj;}  // end of object
      else if (name == "@id") {  // id of object
        jsp = &js.id_to_object_[std::stoul(value)];
        jsp->raw_ = obj;
        continue;
      }
      else try {
        if (!objclass->readMember(js, obj, name, value))
          js.error(JsonError::UnknownMember,
                   "'" +name + "' in class '" + objclass->classname()+"'",
                   false/*not fatal*/);
      }
      catch (std::invalid_argument) {
        js.error(JsonError::InvalidValue, value+" for member '"+name+"'");
      }
    }
    js.error(JsonError::PrematureEOF);
    return nullptr;
  }
  
  // - - -

  // reads a C++ container or a C-array.
  inline void readArray(JsonSerial& js,
                        JsonArray& a, MetaClass::Creator* cr,
                        const std::string& s) {
    if (s != "[") js.error(JsonError::ExpectingBracket);
    while (js.in_->good()) {
      std::string tok, dump;
      bool found1, found2;
      js.readLine(tok, dump, found1, found2, false);
      if (!found1) js.error(JsonError::ExpectingValueOrBracket);
      else if (tok == "]") {a.end(js); return;} // end of array
      //else if (tok == "null");  // null element ignored
      else a.add(js, cr, tok);
    }
  }
  
  // reads a smart pointer in an array/container.
  template <class T>
  inline void readArrayValue2(JsonSerial& js,
                              typename std::enable_if<is_smart_ptr<T>::value,T>::type & e,
                              ObjectPtr*& objptr, MetaClass::Creator* cr,
                              const std::string& s) {
    e.reset();
    if (s != "null") readPointee<T>(js, e, objptr, cr, s);
  }
  
  // reads anything else in an array/container.
  template <class T>
  inline void readArrayValue2(JsonSerial& js,
                              typename std::enable_if<!is_smart_ptr<T>::value,T>::type & e,
                              ObjectPtr*&, MetaClass::Creator*,
                              const std::string& s) {
    readValue(js, e, s);
  }
  
  // reads a char* in an array/container.
  inline void readArrayValue(JsonSerial& js,
                             char* & e, ObjectPtr*&, MetaClass::Creator*,
                             const std::string& s) {
    e = nullptr;
    readValue(js, e, s);
  }
  
  // reads a const char* in an array/container.
  inline void readArrayValue(JsonSerial& js,
                             const char* & e, ObjectPtr*&, MetaClass::Creator*,
                             const std::string& s) {
    e = nullptr;
    readValue(js, e, s);
  }
  
  // reads a raw pointer in an array/container.
  template <class T>
  inline void readArrayValue(JsonSerial& js,
                             T *& e, ObjectPtr*& objptr, MetaClass::Creator* cr,
                             const std::string& s) {
    e = nullptr;
    if (s != "null") readPointee<T>(js, e, objptr, cr, s);
  }
  
  // reads anything in an array/container except a char* or a raw pointer.
  template <class T>
  inline void readArrayValue(JsonSerial& js,
                             T & e, ObjectPtr*& objptr, MetaClass::Creator* cr,
                             const std::string& s) {
    readArrayValue2<T>(js, e, objptr, cr, s);
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  template <typename C, typename R>
  struct ObjectCreatorImpl : public MetaClass::Creator {
    ObjectCreatorImpl(C& obj, std::function<R(C&)> creator)
    : obj_(obj), creator_(creator) {}
    
    C& obj_;
    std::function<R(C&)> creator_;
    void* create() override {return (creator_)(obj_);}
  };
  
  template <typename T, typename Var>
  struct StaticMember : public ObjectClass<T>::Member {
    StaticMember(const std::string& name, Var& var)
    : ObjectClass<T>::Member(name), variable_(var) {}
    
    void read(JsonSerial& js, T&, const std::string& val) override
    {readValue(js, variable_, val);}
    
    void write(JsonSerial& js, const T&) override
    {js.writeValue(variable_);}
  
  protected:
    Var& variable_;
  };
  
  template <typename T, typename Var>
  struct InstanceMember : public ObjectClass<T>::Member {
    InstanceMember(const std::string& name, Var T::* var)
    : ObjectClass<T>::Member(name), variable_(var) {}
    
    void read(JsonSerial& js, T& obj, const std::string& val) override
    {readValue(js, obj.*variable_, val);}
    
    void write(JsonSerial& js, const T& obj) override
    {js.writeValue(obj.*variable_);}
  
  protected:
    Var T::* variable_;
  };
  
  template <typename T, typename Var>
  struct InstanceMemberWithCond : public ObjectClass<T>::Member {
    InstanceMemberWithCond(const std::string& name, Var T::* var, std::function<bool(const T&)> write_if)
    : ObjectClass<T>::Member(name), variable_(var), write_if_(write_if) {}
    
    void read(JsonSerial& js, T& obj, const std::string& val) override
    {readValue(js, obj.*variable_, val);}
    
    void write(JsonSerial& js, const T& obj) override
    {if ((write_if_)(obj)) js.writeValue(obj.*variable_);}
    
  protected:
    Var T::* variable_;
    std::function<bool(const T&)> write_if_;
  };
  
  template <typename T, typename Var, typename R>
  struct InstanceMemberWithCreator : public ObjectClass<T>::Member {
    InstanceMemberWithCreator(const std::string& name,
                              Var T::* var, std::function<R(T&)> creator)
    : ObjectClass<T>::Member(name), variable_(var), creator_(creator) {}
    
    void read(JsonSerial& js, T& obj, const std::string& s) override {
      ObjectCreatorImpl<T,R> c(obj, creator_);
      obj.*variable_ = nullptr;
      ObjectPtr* jsp{nullptr};
      using TObj = typename std::remove_pointer<Var>::type;
      if (s != "null") readPointee<TObj>(js, (obj.*variable_), jsp, &c, s);
    }
    void write(JsonSerial& js, const T& obj) override {js.writeValue(obj.*variable_);}
  
  protected:
    Var T::* variable_;
    std::function<R(T&)> creator_;
  };
  
  template <typename T, typename Var, typename R>
  struct ArrayMemberWithCreator : public ObjectClass<T>::Member {
    ArrayMemberWithCreator(const std::string& name,
                           Var T::* var, std::function<R(T&)> creator)
    : ObjectClass<T>::Member(name), variable_(var), creator_(creator) {}
    
    void read(JsonSerial& js, T& obj, const std::string& s) override {
      ObjectCreatorImpl<T,R> c(obj, creator_);
      JsonArrayImpl<Var> a(obj.*variable_);
      readArray(js, a, &c, s);
    }
    void write(JsonSerial& js, const T& obj) override {js.writeValue(obj.*variable_);}
    
  protected:
    Var T::* variable_;
    std::function<R(T&)> creator_;
  };
  
  template <typename T, typename SetVal, typename GetVal>
  struct InstanceMemberWithAccessor : public ObjectClass<T>::Member  {
    InstanceMemberWithAccessor(const std::string& name,
                               void(T::*setter)(SetVal), GetVal(T::*getter)()const)
    : ObjectClass<T>::Member(name), setter_(setter), getter_(getter) {}
    
    void read(JsonSerial& js, T& obj, const std::string& val) override {
      typename std::remove_const<typename std::remove_reference<SetVal>::type>::type var;
      readValue(js, var, val);
      (obj.*setter_)(std::move(var)); // allow move when possible
    }
    
    void write(JsonSerial& js, const T& obj) override {js.writeValue((obj.*getter_)());}
    
  protected:
    void (T::*setter_)(SetVal);
    GetVal (T::*getter_)() const;
  };
  
  template <typename T>
  struct InstanceCustomMember : public ObjectClass<T>::Member {
    InstanceCustomMember(const std::string& name,
                         std::function<void(T&, JsonSerial&, const std::string&)> readfun,
                         std::function<void(const T&, JsonSerial&)> writefun)
    : ObjectClass<T>::Member(name), readfun_(readfun), writefun_(writefun) {}
    
    bool isCustom() const override {return true;}
    void read(JsonSerial& js, T& obj, const std::string& val) override {(readfun_)(obj,js,val);}
    void write(JsonSerial& js, const T& obj) override {(writefun_)(obj,js);}
    
  protected:
    std::function<void(T&, JsonSerial&, const std::string&)> readfun_;
    std::function<void(const T&, JsonSerial&)> writefun_;
  };
  
  // - - - - - - - -
  
  template <class T>
  template <typename Var>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name, Var& var) {
    addMember(name, new StaticMember<T,Var>(name, var));
    return *this;
  }
  
  template <class T>
  template <typename Var, typename C>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name, Var C::* var) {
    addMember(name, new InstanceMember<T,Var>(name, var));
    return *this;
  }
  
  /*
  template <class T>
  template <typename Var, typename C>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name,
                                         Var C::* var,
                                         std::function<bool(const T&)> write_if) {
    addMember(name, new InstanceMemberWithCond<T,Var>(name, var, write_if));
    return *this;
  }*/
  
  template <class T>
  template <typename SetVal, typename GetVal>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name,
                                         void (T::*setter)(SetVal),
                                         GetVal (T::*getter)() const) {
    addMember(name, new InstanceMemberWithAccessor<T,SetVal,GetVal>(name, setter, getter));
    return *this;
  }
  
  template <class T>
  template <typename Var>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name, Var T::* var,
                                         std::function<typename make_pointer<Var>::type(T&)> cr) {
    addMember(name, new InstanceMemberWithCreator<T,Var, typename make_pointer<Var>::type>(name, var, cr));
    return *this;
  }
  
  template <class T>
  template <typename Var>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name, Var T::* var,
                                         std::function<typename make_array_pointer<Var>::type(T&)> cr) {
    addMember(name, new ArrayMemberWithCreator<T,Var, typename make_array_pointer<Var>::type>(name, var, cr));
    return *this;
  }
  
  template <class T>
  ObjectClass<T>& ObjectClass<T>::member(const std::string& name,
                                         std::function<void(T&, JsonSerial&, const std::string&)> read,
                                         std::function<void(const T&, JsonSerial&)> write) {
    addMember(name, new InstanceCustomMember<T>(name, read, write));
    return *this;
  }
  
  template <class T>
  template <typename Super>
  ObjectClass<T>& ObjectClass<T>::extends() {
    static_assert(std::is_base_of<Super, T>::value, "In call to superclass<S>(): S is not a superclass");
    const MetaClass* c = classes_.getClass(typeid(Super));
    if (!c) classes_.error(JsonError::UnknownSuperclass,
                           std::string(": superclass ")+typeid(Super).name()+" of class "+classname_, "extends()");
    else {
      bool added{false};
      for (auto& it : superclasses_) {if (it.super_ == c) added = true;}
      if (!added) superclasses_.template add<Super>(c);
      else classes_.error(JsonError::RedefinedSuperclass,
                          ": superclass "+c->classname()+" of class "+classname_, "extends()");
    }
    return *this;
  }
  
  template <class T>
  void ObjectClass<T>::addMember(const std::string& name, Member* m) {
    if (getMember(name))
      classes_.error(JsonError::RedefinedMember,": member "+name+" of class "+classname_, "member()");
    else {
      members_.push_back(m); membermap_[name] = m;
    }
  }
  
  template <class T>
  typename ObjectClass<T>::Member* ObjectClass<T>::getMember(const std::string& name) const {
    auto it = membermap_.find(name);
    if (it == membermap_.end()) return nullptr; else return it->second;
  }
  
  template <class T>
  bool ObjectClass<T>::readMember(JsonSerial& js, void* obj, const std::string& name, const std::string& val) const {
    if (auto mb = getMember(name)) {    // search in subclass first
      mb->read(js, *static_cast<T*>(obj), val);
      return true;
    }
    for (auto& it : superclasses_) {    // if not found, search in superclasses
      if (it.super_->readMember(js, (it.upcast_)(obj), name, val)) return true;
    }
    return false;
  }
  
  template <class T>
  void ObjectClass<T>::writeMembers(JsonSerial& js, const void* obj) const {
    for (auto& it : superclasses_) {    // print members in superclasses first
      it.super_->writeMembers(js, (it.upcast_)((void*)obj));
    }
    for (auto& it : members_) {  // then print members (can't be shadowed!)
      if (js.needcomma_) *(js.out_) << ",\n"; js.needcomma_ = false;
      if (it->isCustom()) js.token1_ = it->name();
      else {js.writeTabs(); *(js.out_) << '"' << it->name() << "\": ";}
      it->write(js, *static_cast<const T*>(obj));
    }
  }
  
  template <class T>
  void ObjectClass<T>::doPostRead(void* obj) const {
    if (obj && postread_) postread_(*static_cast<T*>(obj));
  }
  
  template <class T>
  void ObjectClass<T>::doPostWrite(const void* obj) const {
    if (obj && postwrite_) postwrite_(*static_cast<const T*>(obj));
  }
  
  // - - - - - - - -
  
  template <class T>
  bool MapClass<T>::readMember(JsonSerial& js, void* map, const std::string& key, const std::string& val) const {
    using E = typename T::mapped_type;
    readValue(js, (*static_cast<T*>(map))[key] = E{}, val);
    return true;
  }
  
  template <class T>
  void MapClass<T>::writeMembers(JsonSerial& js, const void* map) const {
    for (auto& it : *static_cast<const T*>(map)) {
      if (js.needcomma_) *(js.out_) << ",\n";
      js.needcomma_ = false;
      js.writeTabs(); *(js.out_) << '"' << it.first << "\": ";
      js.writeValue(it.second);
    }
  }
  
  template <class T>
  ObjectClass<T> & JsonClasses::defclass(const std::string& classname, std::function<T*()> creator) {
    if (getClass(classname)) error(JsonError::RedefinedClass, classname, "defclass()");
    ObjectClass<T>* cl = new ObjectClass<T>(*this, classname, creator);
    classindexes_[std::type_index(typeid(T))] = classnames_[classname] = cl;
    return *cl;
  }
  
  // add element to a C-array with brackets.
  template<class T>
  struct JsonArrayImpl<T, typename std::enable_if<std::is_array<T>::value>::type> : public JsonArray {
    T& array_;
    size_t index_;
    
    JsonArrayImpl(T& array) : array_(array), index_(0) {}
    
    void add(JsonSerial& js, MetaClass::Creator* cr, const std::string& s) override {
      ObjectPtr* jsp{nullptr};
      if (index_ >= std::extent<T>::value) js.error(JsonError::CantAddToArray);
      else readArrayValue(js, array_[index_++], jsp, cr, s);
    }
  };
  
  /*
   // add element to a dynamic C array.
   template <class A>
   static A* addToDynamicArray(A*& array, JsonSerial&, size_t index) {
   array = (A*)::realloc(array, index * sizeof(A*));
   return array + index;
   }
   */
}

#endif

