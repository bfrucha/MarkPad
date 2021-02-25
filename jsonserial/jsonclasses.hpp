//
//  JsonSerial: C++ Object Serialization in JSON.
//  JsonClasses
//  (C) Eric Lecolinet 2017/2018 - https://www.telecom-paristech.fr/~elc
//

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef jsonclasses_hpp
#define jsonclasses_hpp

namespace ccuty {
  
  class JsonSerial;
  class JsonClasses;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Generic Metaclass.
  class MetaClass {
  public:
    /// @internal Creator function.
    struct Creator {
      virtual void* create() = 0;
      virtual ~Creator() {}
    };
    virtual ~MetaClass() {}
    virtual const std::string& classname() const = 0;
    virtual void* create() const = 0;
    virtual bool readMember(JsonSerial&, void* obj, const std::string& name,
                            const std::string& value) const = 0;
    virtual void writeMembers(JsonSerial&, const void* obj) const = 0;
    virtual void doPostRead(void* obj) const = 0;
    virtual void doPostWrite(const void* obj) const = 0;
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  struct JsonArray {
    virtual ~JsonArray() {}
    virtual void add(JsonSerial&, MetaClass::Creator*, const std::string& s) = 0;
    virtual void end(JsonSerial&) {}
  };
  
  template <class T, class Enable = void> struct JsonArrayImpl : public JsonArray {};
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /** Serves to declare the (serialized) members of a C++ class.
   *  @see jsonserial.hpp (explanations and example).
   *  @see the member() methods of this class.
   */
  template <class C>
  class ObjectClass : public MetaClass {
  public:
    /// Returns the name of the C++ class.
    const std::string& classname() const override {return classname_;}
    
    /** Declares a superclass.
     * Template Argument:
     * - the C++ superclass
     *
     *  Multiple inheritence is supported by calling this method once for each
     *  superclass (in the order they are inherited).
     */
    template <typename Super> ObjectClass& extends();
    
    /** Declares a static member variable or a non-member variable.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _var_: a static or global variable
     *
     * This method must be used only for static or global variables. The variable
     * will appear in all instances of the C++ class in the JSON file.
     */
    template <typename Var>
    ObjectClass& member(const std::string& varname, Var& var);
    
    /** Declares a member variable.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _var_: an instance variable
     */
    template <typename Var, typename T>
    ObjectClass& member(const std::string& varname, Var T::* var);

    /* Declares a member variable and a writing condition.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _var_: an instance variable
     * - _write_if_: a testing function.
     *
     * The member will be written in the JSON file only if write_if(var) returns true.
     *
    template <typename Var, typename T>
    ObjectClass& member(const std::string& varname,
                        Var T::* var,
                        std::function<bool(const C&)> write_if);
    */
    
    /** Declares a member variable using its accessors.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _setter_: the setter method of the variable
     * - _getter_: the getter method of the variable
     *
     * This method involves temporary variables and should be used only
     * if the variable cannot be accessed (e.g. the variable is private and a
     * 'friend' statement cannot be added)
     */
    template <typename SetVal, typename GetVal>
    ObjectClass& member(const std::string& varname,
                        void (C::*setter)(SetVal),
                        GetVal (C::*getter)()const);
    
    /** Declares a member variable that is a pointer and a creator helper.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _var_: an instance variable that is a pointer
     * - _creator_: a helper function that creates an instance of the pointee
     *
     * This method should only be used if the pointee needs to be created using
     * a custom function (e.g. because the class of the pointer does not have a
     * no-argument constructor).
     *
     * _creator_ can be 1) a lambda, a static method, a non-member function or
     * 2) an instance method of C, the class that contains the variable:
     * - It must return a *pointer* to the newly created object,
     * - In case 1) its parameter is of type C& and refers to the object that
     *   contains the variable.
     * - In case 2) there is no parameter because _creator_ is a method ('this'
     *   will then point to the object).
     */
    template <typename Var>
    ObjectClass& member(const std::string& varname,
                        Var C::* var,
                        std::function<typename make_pointer<Var>::type(C&)> creator);
    
    /** Declares a member variable that is a container (or a C array) and a creator helper.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _var_: an instance variable that is a standard container or a 1-dimensional
     *          C array with brackets
     * - _creator_: a helper function that creates an instance of the container elements
     *
     * This method should only be used if the elements of the container needs to be
     * created using a custom function (e.g. because the class of the pointer does
     * not have a no-argument constructor).
     *
     * _creator_ can be 1) a lambda, a static method, a non-member function or
     * 2) an instance method of C, the class that contains the variable:
     * - It must return a *pointer* to the newly created object,
     * - In case 1) its parameter is of type C& and refers to the object that
     *   contains the variable.
     * - In case 2) there is no parameter because _creator_ is a method ('this'
     *   will then point to the object).
     */
    template <typename Var>
    ObjectClass& member(const std::string& varname,
                        Var C::* variable,
                        std::function<typename make_array_pointer<Var>::type(C&)> creator);
    
    /** Declares a member variable that is serialized using custom functions.
     * Arguments:
     * - _varname_: the UTF8 name of variable as it will appear in the JSON file
     *              (not necessarily the name of the C++ variable)
     * - _read_: the function that reads the variable from the JSON file
     * - _write_: the function that writes the variable to the JSON file
     *
     *  This methods make it possible to read/write variables in a specific way.
     *  _read_ and _write_ can be 1) lambdas, static methods, non-member functions,
     *  2) instance methods of C, the class that contains the variable:
     *
     *  - In case 1) the first parameter is of type C& (read) or const C& (write)
     *    and it refers to the object that contains the variable.
     *  - In case 2) there is no first parameter because the function is a method
     *    ('this' will then point to the object).
     *  - The _read_ function has a last parameter with contains the value read
     *    from the JSON file.
     */
    ObjectClass& member(const std::string& varname,
                        std::function<void(C&, JsonSerial&, const std::string& value)> read,
                        std::function<void(const C&, JsonSerial&)> write);
    
    /** Calls a function once all members have been read.
     * Argument:
     * - _fun_: a function that is called after all members have been read
     *
     * _fun_ can be 1) a lambda, a static method, a non-member function or
     * 2) an instance method of C, the class that contains the variable:
     * - In case 1) the first parameter is of type C& and refers to the object
     *   that contains the variable.
     * - In case 2) there is no parameter because _fun_ is a method ('this' will
     *   then point to the object).
     */
    ObjectClass& postread(std::function<void(C&)> fun)
    {postread_ = fun; return *this;}
    
    /** Calls a function once all members have been written.
     * Argument:
     * - _fun_: a function that is called after all members have been written
     *
     * _fun_ can be 1) a lambda, a static method, a non-member function or
     * 2) an instance method of C, the class that contains the variable:
     * - In case 1) the first parameter is of type const C& and refers to the object
     *   that contains the variable.
     * - In case 2) there is no parameter because _fun_ is a method ('this' will
     *   then point to the object).
     */
    ObjectClass& postwrite(std::function<void(const C&)> fun)
    {postwrite_ = fun; return *this;}
    
    class Member {
    public:
      Member(const std::string& name) : name_(name) {}
      virtual ~Member() {}
      const std::string& name() const {return name_;}
      virtual bool isCustom() const {return false;}
      virtual void read(JsonSerial&, C& object, const std::string& value) = 0;
      virtual void write(JsonSerial&, const C& object) = 0;
    protected:
      const std::string name_;
    };
    
  protected:
    friend class ccuty::JsonClasses;
    
    ObjectClass(JsonClasses& classes, const std::string& classname, std::function<C*()> creator)
    : classes_(classes), classname_(classname), creator_(creator) {}
    virtual ~ObjectClass() {for (auto& it : members_) delete it;}
    
    void* create() const override {return creator_ ? (creator_)() : nullptr;}
    void addMember(const std::string& varname, Member*);
    Member* getMember(const std::string& varname) const;
    bool readMember(JsonSerial&, void* obj, const std::string& name, const std::string& val) const override;
    void writeMembers(JsonSerial&, const void* obj) const override;
    void doPostRead(void* obj) const override;
    void doPostWrite(const void* obj) const override;
    
    struct Superclass {
      const MetaClass* super_;
      std::function<void*(void*)> upcast_;
    };
    
    struct Superclasses : public std::list<Superclass> {
      template <class S> void add(const MetaClass* c) {this->push_back(Superclass{c, upcast<S>});}
      template <class S> static void* upcast(void* obj) {return dynamic_cast<S*>(static_cast<C*>(obj));}
    };
    
    JsonClasses& classes_;
    const std::string classname_;
    Superclasses superclasses_;
    std::function<C*()> creator_{nullptr};
    std::list<Member*> members_;
    std::unordered_map<std::string, Member*> membermap_;
    std::function<void(C&)> postread_{nullptr};
    std::function<void(const C&)> postwrite_{nullptr};
  };
  
  
  /// @internal Metaclass for maps.
  template <class C> class MapClass : public MetaClass {
  public:
    const std::string& classname() const override {static std::string s("std::map"); return s;}
    void* create() const override {return new C();}
    bool readMember(JsonSerial&, void* obj, const std::string& name, const std::string& value) const override;
    void writeMembers(JsonSerial&, const void* obj) const override;
    void doPostRead(void*) const override {}
    void doPostWrite(const void*) const override {}
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /** @brief Stores the C++ classes that are serialized.
   * @see jsonserial.hpp for explanations and an example.
   * @see defclass() methods for declaring classes.
   */
  class JsonClasses {
  public:
    
    /** Constructor.
     * Argument:
     * - _handler_: an optional error handler (a lambda or a static function).
     *
     *  Errors are printed on std::cerr if no error handler is specified.
     */
    JsonClasses(JsonError::Handler handler = nullptr) : errhandler_(handler) {}
    
    ~JsonClasses() {
      for (auto& it : classnames_) delete it.second;
      delete jsonerror_;
    }
    
    /** Declares a class that has a public no-argument constructor.
     * Argument:
     * - _classname_: the UTF8 name of the C++ class.
     *
     * The no-argument constructor will be used to instanciate objects of this class.
     * _Class_ (the template argument) is the C++ class and _classname_ its UTF8 name.
     *
     * The other version of _defclass_ must be used if the class does not
     * have a no-argument constructor, or if this constr. is not public, or if
     * the class is abstract.
     */
    template <class Class>
    ObjectClass<Class>& defclass(const std::string& classname) {
      struct Helper {static Class* create() {return new Class();} ;};
      return defclass<Class>(classname, Helper::create);
    }
    
    /** Declares an abstract class or a class that does NOT have a public no-argument constructor.
     * Arguments:
     * - _classname_: the UTF8 name of the C++ class.
     * - _creator_: a helper function that creates an instance of this class
     *
     * This method should only be used in one of the following cases:
     * - 1) The class is abstract, _creator_ must then be nullptr
     * - 2) It does not have a no-argument constructor
     * - 3) This constructor is not public.
     *
     * _creator_ can be lambda, a static method or a non-member function (except
     * in the first case as the class can't be instanciated).
     *
     * Note that nullptr can be given as a _creator_ even if the class is not
     * abstract if it is never instanciated by JsonSerial (i.e. if no pointer
     * of this a class is serialized).
     */
    template <class Class>
    ObjectClass<Class>& defclass(const std::string& classname, std::function<Class*()> creator);
    
    /// produces an error.
    void error(JsonError::Type type, const std::string& arg, const std::string& where) {
      if (!jsonerror_) jsonerror_ = new JsonError();
      jsonerror_->set(type, true, where, arg, "", 0, errhandler_);
    }
    
    /// returns true if no class was defined, false otherwise.
    bool empty() const {return classnames_.empty();}
    
    /// Returns the class having this ASCII name.
    const MetaClass* getClass(const std::string& classname) const {
      auto it = classnames_.find(classname);
      return (it == classnames_.end()) ? nullptr : it->second;
    }
    
    /// Returns the class corresponding to this type_info.
    const MetaClass* getClass(const std::type_info& tinfo) const {
      auto it = classindexes_.find(std::type_index(tinfo));
      return (it == classindexes_.end()) ? nullptr : it->second;
    }
    
  private:
    JsonError::Handler errhandler_{nullptr};
    JsonError* jsonerror_{nullptr};
    std::unordered_map<std::type_index, MetaClass*> classindexes_;
    std::unordered_map<std::string, MetaClass*> classnames_;
  };
  
}
#endif

