//
//  JsonSerial: C++ Object Serialization in JSON.
//  Main file
//  (C) Eric Lecolinet 2017/2018 - https://www.telecom-paristech.fr/~elc
//

/** @file 
 *  Object Serialization in JSON.
 *
 * - Jsonserial allows serializing any set of C++ objects (a single object or a collection,
 *   a tree or even a cyclic graph of objects).
 * - Variables can be arithmetic values, enums, C and C++ strings, C++ objects (plain objects
 *   or pointers, including shared_ptr and unique_ptr), C++ containers (vector, list,
 *   map, unordered_map) and 1-dimensional arrays with brackets.
 * - The names in the JSON file can be the names of the C++ variables or whatever desired.
 *   This allows producing human-readable files such as resource or configuration files.
 * - Jsonserial is not intrusive in that sense it does not require writing read/write
 *   functions in the C++ classes. Instead, classes and variables are registered
 *   using Jsonserial functions outside C++ classes.
 * - Jsonserial supports multiple inheritance, polymorphism and shared objects
 *   (which are thus not duplicated). When useful, class names and object IDs are stored
 *   in JSON files as "@class" or "@id" members inside JSON objects.
 * - Jsonserial allows relaxing the JSON syntax in various ways (comments are supported,
 *   quotes and commas can optionally be omitted, etc.)
 * - JsonSerial consists of a single header file and relies on C++ 11 templates.
 *
 * **Main classes:** ccuty::JsonSerial, ccuty::JsonClasses, ccuty::ObjectClass, ccuty::JsonError.
 *
 * **Example:**
 * @code
 *    class PhoneNumber {
 *    public:
 *       const std::string& getType() const {return type;}
 *       const std::string& getNumber() const  {return number;}
 *       void setType(const std::string& s) {type = s;}
 *       void setNumber(const std::string& s)  {number = s;}
 *    private:
 *      std::string type, number;
 *    };
 *
 *    class Contact {
 *      friend class Classes;
 *      std::string firstname, lastname;
 *      std::list<PhoneNumber> numbers;
 *    };
 *
 *    class Classes : public JsonClasses {
 *      Classes() {
 *        defclass<Contact>("Contact")
 *         .member(&Contacts::firstname, "firstname")
 *         .member(&Contacts::lastname, "lastname")
 *         .member(&Contacts::numbers, "numbers");
 *
 *        defclass<PhoneNumber>("PhoneNumber")
 *         .member(&PhoneNumber::setType, &PhoneNumber::getType, "type")
 *         .member(&PhoneNumber::setNumber, &PhoneNumber::getNumber, "number");
 *    }
 *
 *    int main() {
 *      static Classes classes;
 *      JsonSerial js(classes);
 *      Contact c;
 *
 *      // needed if cyclic graph of objects, infinite loop otherwise!
 *      // (objects are not duplicated, IDs are wtiten in the JSON file)
 *      // js.setSharing();
 *
 *      if (!js.read(c, "data.json")) return 1;
 *      if (!js.write(c, "data-sav.json")) return 1;
 *      return 0;
 *    }
 * @endcode
 *
 * @author Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
 */

#ifndef jsonserial_hpp
#define jsonserial_hpp

#include <string.h>
#include <cstdlib>
#include <locale>
#include <memory>
#include <type_traits>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <utility>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <unordered_map>
#include "jsondefs.hpp"
#include "jsonerror.hpp"
#include "jsonclasses.hpp"

namespace ccuty {

  /** Reads/writes C++ objects from/to a JSON file. 
   * See:
   * - jsonserial.hpp for explanations and an example.
   * - read() to read objects from a JSON file
   * - write() to write objects to a JSON file
   * - setSharing() to share objects whithout duplicating them
   * - setSyntax() to relax syntax.
   */
  class JsonSerial {
  public:
    /** Constructor.
     *  _classes_ refers to the classes that have been registered for serialization.
     *  _handler_ is an optional error handler (can be a lambda or a static function).
     *  Errors are printed on std::cerr if no error handler is specified.
     *  @see JsonSerial, JsonClasses, JsonError.
     */
    JsonSerial(JsonClasses& classes, JsonError::Handler handler = nullptr)
    : classes_(classes), errhandler_(handler) {}
    
    ~JsonSerial() {
      delete jsonerror_;
    }
    
    /** Reads an object and its members recursively from a JSON file.
     *  Returns false an prints a message in case of an error (see constructor for details)
     */
    template <class T>
    bool read(T& object, const std::string& filename) {
      try {
        std::ifstream input(filename);
        if (!input) {
          reset(filename, 0, nullptr, nullptr);
          error(JsonError::CantReadFile);
        }
        else if (!read(object, input, filename, 1)) return false;
      }
      catch (JsonError* e) {return false;}
      return !jsonerror_;  // not null if warning
    }
    
    /** Reads an object and its members recursively from an input stream.
     *  Returns false an prints a message in case of an error (see constructor for details)
     */
    template <class T>
    bool read(T& object, std::istream& stream,
              const std::string& streamname = "", size_t firstline = 1) {
      try {
        reset(streamname, firstline, &stream, nullptr);
        std::string keyword, dump;
        bool found1, found2;
        readLine(keyword, dump, found1, found2, true);
        if (found1) readValue(*this, object, keyword); else error(JsonError::NoData);
      }
      catch (JsonError* e) {return false;}
      return !jsonerror_;
    }
    
    /** Writes an object and its members recursively in a JSON file.
     *  Returns false an prints a message in case of an error (see constructor for details)
     */
    template <class T>
    bool write(const T& object, const std::string& filename) {
      try {
        std::ofstream output(filename);
        if (!output) {
          reset(filename, 0, nullptr, nullptr);
          error(JsonError::CantWriteFile);
        }
        else if (!write(object, output, filename, 1)) return false;
      }
      catch (JsonError* e) {return false;}
      return !jsonerror_;
    }
    
    /** Writes an object and its members recursively on an output stream.
     *  Returns false an prints a message in case of an error (see constructor for details)
     */
    template <class T>
    bool write(const T& object, std::ostream& stream,
               const std::string& streamname = "", size_t firstline = 1) {
      try {
        reset(streamname, firstline, nullptr, &stream);
        writeValue(object);
        *out_ << "\n" << std::endl;
      }
      catch (JsonError* e) {return false;}
      return !jsonerror_;
    }
    
    /// Returns the corresponding JsonClasses object.
    JsonClasses& getClasses() {return classes_;}

    /// Returns the last JsonError (if any).
    JsonError* getError() {return jsonerror_;}
    
    /** Allows sharing objects.
     * If _mode_ is true, objects pointed by several pointers are not duplicated
     * in JSON files, instead, they are referenced using a special "@id" field.
     * Objects will then be re-created in the same way when reading the files.
     *
     * This also makes it possible to write cross referenced objects and cycling graphs
     * (without this option the write() method will enter an infinite loop in such a case)
     */
    void setSharing(bool mode = true) {sharing_ = mode;}
    
    /// Return true if object sharing is allowed.
    bool getSharing() const {return sharing_;}
    
    /* JSON syntax.
     * - Strict: strict JSON syntax
     * - Relaxed: all options are allowed
     * - Comments: allows C++ style comments
     * - NoQuotes: allows unquoted strings
     * - NoCommas: name/value pairs can be sparated by a newline
     * - Newlines: strings can contain newlines and any ::isspace() character
     */
    enum Syntax {
      Strict=0, Comments=1, NoQuotes=2, NoCommas=4, Newlines=8,
      Relaxed=(Comments|NoQuotes|NoCommas|Newlines)
    };
    
    /** Sets syntax.
     *  Argument is an ORred mask of Syntax values.
     *  By default, comments are allowed (= setSyntax(JsonSerial::Comments))
     *  See Syntax enum.
     */
    void setSyntax(unsigned int mode) {allow_ = mode;}
    
    /// Returns current syntax options (ORred mask of Syntax values).
    unsigned int getSyntax() const {return allow_;}
    
    /** Changes indentation.
     *  _tabchar_: tabulation character, _tabcount_: how many times it is repeated.
     */
    void setIndent(char tabchar, unsigned int tabcount) {tabchar_ = tabchar; indent_ = tabcount;}

    /// Returns current indentation.
    void getIndent(char& tabchar, unsigned int& tabcount) const {tabchar = tabchar_; tabcount = indent_;}

    template <class T>
    void readMember(T& variable, const std::string& str) {
      readValue(*this, variable, str);
    }
    
    template <class T>
    void writeMember(const T& variable) {
      writeTabs(); *out_ << '"' << token1_ << "\": ";
      writeValue(variable);
    }
    
    // - - - Read - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // throws if class not found.
    const MetaClass* getCheckedClass(const std::type_info& tinfo) {
      const MetaClass* cl = classes_.getClass(tinfo);
      if (!cl) error(JsonError::UnknownClass, tinfo.name());
      return cl;
    }
    
    // - - - Write - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // writes a char
    void writeValue(char c) {*out_ << '\"' << c << '\"'; needcomma_ = true;}
    
    // writes a bool.
    void writeValue(bool b) {*out_ << (b ? "true" : "false"); needcomma_ = true;}
    
    // writes a C++ string.
    void writeValue(const std::string& s) {writeString(s.c_str(), false);}
    
    // writes a C string.
    void writeValue(char* s) {writeString(s, true);}
    void writeValue(const char* s) {writeString(s, true);}
    
    // writes anything except a string or a bool.
    template <class T> void writeValue(const T& val) {writeValue2<T>(val); needcomma_ = true;}
    
    // writes a raw pointer (note: is_pointer differentiates from is_array).
    template <class T>
    void writeValue2(const typename std::enable_if<std::is_pointer<T>::value,T>::type & ptr) {
      if (!ptr) *out_ << "null"; else writeValue(*ptr);
    }
    
    // writes a smart pointer.
    template <class T>
    void writeValue2(const typename std::enable_if<is_smart_ptr<T>::value,T>::type & ptr) {
      if (!ptr) *out_ << "null"; else writeValue(*ptr);
    }
    
    // writes a number.
    template <class T>
    void writeValue2(const typename std::enable_if<std::is_arithmetic<T>::value,T>::type & number) {
      *out_ << number;
    }
    
    // writes an enum.
    template <class T>
    void writeValue2(const typename std::enable_if<std::is_enum<T>::value,T>::type & e) {
      *out_ << int(e);
    }
    
    // writes a map.
    template <class T>
    void writeValue2(const typename std::enable_if<is_std_map<T>::value,T>::type & obj) {
      MapClass<T> cl; writeObject(cl, false, &obj);
    }
    
    // writes a defobject.
    template <class T>
    void writeValue2(const typename std::enable_if<is_defobject<T>::value,T>::type & obj) {
      const MetaClass* cl = classes_.getClass(typeid(obj));
      if (!cl) error(JsonError::UnknownClass, typeid(obj).name());
      writeObject(*cl, (typeid(obj) != typeid(T)), &obj);
    }
    
    // writes an array_style C++ container
    template <class T>
    void writeValue2(const typename std::enable_if<has_array_format<T>::value,T>::type & cont) {
      if (cont.empty()) *out_ << "[]"; else writeArray(cont);
    }
    
    // writes a C-array.
    template <class T>
    void writeValue2(const typename std::enable_if<std::is_array<T>::value,T>::type & carray) {
      if (std::extent<T>::value == 0) *out_ << "[]"; else writeArray(carray);
    }
    
    // writes a defobject.
    void writeObject(const MetaClass& cl, bool is_derived_class, const void* obj) {
      if (sharing_) {
        auto it = object_to_id_.find(obj);
        if (it != object_to_id_.end()) {*out_ << "\"@"<< it->second <<'"'; return;}
        else object_to_id_[obj] = ++current_object_id_;
      }
      needcomma_ = false;
      *out_ << "{\n";
      addTab();
      if (is_derived_class) {   // polymorphism
        writeTabs(); *out_ << "\"@class\": \"" << cl.classname() << "\",\n";
      }
      if (sharing_) {
        writeTabs(); *out_ << "\"@id\": \"" << current_object_id_ << "\",\n";
      }
      cl.writeMembers(*this, obj);
      removeTab();
      *out_ << "\n"; writeTabs(); *out_ << "}";
      needcomma_ = true;
      cl.doPostWrite(obj);  // end of the object
    }
    
    // writes a C++ container or a C-array.
    template <class T> void writeArray(const T & array) {
      needcomma_ = false;
      *out_ << "[\n";
      addTab();
      for (auto& it : array) {
        if (needcomma_) *out_ << ",\n";
        writeTabs();
        needcomma_ = false;
        writeValue(it);
      }
      removeTab();
      *out_ << "\n"; writeTabs(); *out_ << "]";
    }
    
    // writes a string.
    void writeString(const char* s, bool is_cstring) {
      if (!s) {*out_ << (is_cstring ? "null" : "\"\"");}
      else {
        out_->put('"');
        for (; *s != 0; ++s) {
          switch (*s) {
            case '"': out_->put('\\'); out_->put('"'); break;
            case '\\': out_->put('\\'); out_->put('\\'); break;
            case '\b': out_->put('\\'); out_->put('b'); break;
            case '\f': out_->put('\\'); out_->put('f'); break;
            case '\n': out_->put('\\'); out_->put('n'); break;
            case '\r': out_->put('\\'); out_->put('r'); break;
            case '\t': out_->put('\\'); out_->put('t'); break;
            default: out_->put(*s);
          }
        }
        out_->put('"');
      }
      needcomma_ = true;
    }
    
    /// produces an error; throws except if _warning_ is true.
    void error(JsonError::Type type, const std::string& arg = "", bool fatal = true) {
      std::string where = (in_!=nullptr || type==JsonError::CantReadFile) ? "read" : "write";
      if (!jsonerror_) jsonerror_ = new JsonError();
      jsonerror_->set(type, fatal, where, arg, streamname_, lineno_, errhandler_);
      if (fatal) throw jsonerror_;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
  //private:
    template <class T> friend class ObjectClass;
    template <class T> friend class MapClass;
    
    void readLine(std::string& token1, std::string& token2, bool& found1, bool& found2, bool inObj) {
      token1.clear();
      token2.clear();
      token1_.clear();
      token2_.clear();
      found1 = found2 = false;
      enum {
        Begin, InQuotedToken1, InUnquotedToken1, AfterToken1, AfterComa,
        InQuotedToken2, InUnquotedToken2, AfterToken2, Comment, LineComment
      }
      part{Begin}, lastPart{Begin};
      char c = 0;
      
      while (true) {
        if (!in_->get(c)) {
          if (!token1_.empty()) {token1 = token1_; checkValue(token1,inObj);}
          return;
        }
        
        if (c == '\n')
          lineno_++;
        else if (::iscntrl(c) && !::isspace(c))
          goto INVALID_CHAR;
        else if ((allow_&Comments) && part!=InQuotedToken1 && part!=InQuotedToken2) {
          if (part != Comment && c == '/' && in_->peek() == '/') {
            if (part != LineComment) {lastPart = part; part = LineComment;}
          }
          else if (part != LineComment && c == '/' && in_->peek() == '*') {
            if (part != Comment) {in_->get(c); lastPart = part; part = Comment;}
          }
        }
        switch (part) {
          case Begin:
            if (c == '"') {found1 = true; part = InQuotedToken1;}
            else if (c == '{' || c == '[') {found1 = true; token1 = c; return;}
            else if (!::isspace(c)) {found1 = true; token1_ += c; part = InUnquotedToken1;}
            break;
          case InQuotedToken1:
            if (c == '"') {token1 = token1_; part = AfterToken1;}
            else if (c == '\\') readEscape(token1_);
            else if (::iscntrl(c) && (!(allow_&Newlines) || !::isspace(c))) goto INVALID_CHAR;
            else token1_ += c;
            break;
          case InUnquotedToken1:
            if (c == ',' || ((allow_&NoCommas) && c == '\n')) {token1 = token1_; checkValue(token1,inObj); return;}
            else if (c == '}' || c == ']')
             {in_->putback(c); token1 = token1_; checkValue(token1,inObj); return;}
            else if (c == ':' && inObj) {token1 = token1_; checkValue(token1,inObj); part = AfterComa;}
            else if (c == '\\') readEscape(token1_);
            else token1_ += c;
            break;
          case AfterToken1:
            if (c == ',' || ((allow_&NoCommas) && c == '\n')) return;
            else if (c == '}' || c == ']') {in_->putback(c); return;}
            else if (c == ':' && inObj) part = AfterComa;
            else if (!::isspace(c)) {error(JsonError::ExpectingComma); return;}
            break;
          case AfterComa:
            if (c == '"') {
              found2 = true;
              if (in_->peek() != '"') part = InQuotedToken2;
              else {
                in_->get(c);
                if (in_->peek() != '"') {token2 = ""; part = AfterToken2;}
                else {in_->get(c); part = InQuotedToken2; in_multiquotes_ = true;}
              }
            }
            else if (c == '{' || c == '[') {found2 = true; token2 = c; return;}
            else if (!::isspace(c)) {found2 = true; token2_ += c; part = InUnquotedToken2;}
            break;
          case InQuotedToken2:
            if (c == '"') {
              if (!in_multiquotes_) {token2 = token2_; part = AfterToken2;}
              else if (in_->peek() != '"') token2_ += '"';
              else {
                in_->get(c);
                if (in_->peek() != '"') token2_ += "\"\"";
                else {
                  in_->get(c); token2 = token2_; part = AfterToken2; in_multiquotes_ = false;
                }
              }
            }
            else if (in_multiquotes_ && ::isspace(c)) token2_ += c;
            else if (c == '\\') readEscape(token2_);
            else if (::iscntrl(c) && (!(allow_&Newlines) || !::isspace(c))) goto INVALID_CHAR;
            else token2_ += c;
            break;
          case InUnquotedToken2:
            if (c == ',' || ((allow_&NoCommas) && c == '\n')) {token2 = token2_; checkValue(token2,false); return;}
            else if (c == '}' || c == ']') {in_->putback(c); token2 = token2_; checkValue(token2,false); return;}
            else if (c == '\\') readEscape(token2_);
            else token2_ += c;
            break;
          case AfterToken2:
            if (c == ',' || ((allow_&NoCommas) && c == '\n')) return;
            else if (c == '}' || c == ']') {in_->putback(c); return;}
            else if (!::isspace(c)) {error(JsonError::ExpectingDelimiter); return;}
            break;
          case LineComment:
            if ((allow_&Comments) && c == '\n') part = lastPart;
            break;
          case Comment:
            if ((allow_&Comments) && (c == '*' && in_->peek() == '/')) {in_->get(c); part = lastPart;}
            break;
        }
      }
      return;
      INVALID_CHAR:
      std::string msg;
      if (c=='\n') msg="newline "; else if (c=='\r') msg="CR "; else if (c=='\t') msg="tab ";
      error(JsonError::InvalidCharacter, msg + "(code: "+std::to_string(int(c))+")");
    }
    
    void readEscape(std::string& token) {
      int c = in_->get();
      switch (c) {
        case '"': token += '"'; break;
        case '\\': token += '\\'; break;
        case '/': token += '/'; break;
        case 'b': token += '\b'; break;
        case 'f': token += '\f'; break;
        case 'n': token += '\n'; break;
        case 'r': token += '\r'; break;
        case 't': token += '\t'; break;
        //case 'u': utf16_to_utf8(token); break; TODO
        default: token += c; break;
      }
    }
    
    bool isNumber(const std::string& token) {
      if (token.empty()) return false;
      bool dotfound{false}, expfound{false};
      const char *p = token.c_str();
      if (*p == '-') ++p;
      for (; *p != 0; ++p)
        if (!::isdigit(*p)) {
          if (*p=='.') {if (dotfound) return false; else dotfound = true;}
          else if (*p=='e' || *p=='E') {
            if (expfound) return false; else expfound = true;
            if (*(p+1)=='+' || *(p+1)=='-') ++p;
          }
          else return false;
        }
      return true;
    }
    
    void checkValue(std::string& token, bool objName) {
      if (!token.empty()) {   // trimRight
        const char *s = token.c_str(), *end = s + token.length()-1;
        while (end >= s && ::isspace(*end)) --end;
        if (end >= s) token.assign(s, end-s+1);
      }
      if (objName) {
        if ((allow_&NoQuotes) || token[0]=='}' || token[0]==']') return;
        else error(JsonError::ExpectingString, token);
      }
      else if ((allow_&NoQuotes) || token.empty()
          || token[0]=='}' || token[0]==']' || token=="true" || token=="false" || token=="null"
          || isNumber(token))
        return;
      else error(JsonError::InvalidValue, token+" (should be quoted?)");
    }
    
    void reset(const std::string& streamname, size_t lineno, std::istream *in, std::ostream *out) {
      in_ = in;
      out_ = out;
      if (in_) in_->imbue(locale_);
      if (out_) out_->imbue(locale_);
      streamname_ = streamname;
      lineno_ = lineno;
      needcomma_ = false;
      level_ = 0;
      token1_.reserve(50);
      token2_.reserve(50);
      in_multiquotes_ = false;
      tabs_.assign(40, tabchar_);
      object_to_id_.clear();
      id_to_object_.clear();
      current_object_id_ = 0;
      delete jsonerror_; jsonerror_ = nullptr;
    }
    
    void addTab() {if (++level_*indent_ >= tabs_.size()) tabs_.resize(tabs_.size() + 20, tabchar_);}
    void removeTab() {if (--level_ < 0) level_ = 0;}
    void writeTabs() {out_->write(tabs_.data(), level_*indent_);}
    
    JsonClasses& classes_;
    std::locale locale_{std::locale::classic()};
    std::istream *in_{nullptr};
    std::ostream *out_{nullptr};
    unsigned char allow_{Comments};
    bool needcomma_{false}, in_multiquotes_{false}, sharing_{false};
    size_t lineno_{0};
    unsigned int indent_{2};
    int level_{0};
    char tabchar_{' '};
    std::string streamname_, tabs_, token1_, token2_;
    unsigned long current_object_id_{0};
    std::unordered_map<const void*, unsigned long> object_to_id_;
    std::unordered_map<unsigned long, ObjectPtr> id_to_object_;
    JsonError::Handler errhandler_{nullptr};
    JsonError* jsonerror_{nullptr};
  };
}

#endif

#include "jsonimpl.hpp"


