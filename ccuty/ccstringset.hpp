//
//  stringset: string set and indexed string set
//  (C) Eric Lecolinet 2016/17 - https:www.telecom-paristech.fr/~elc
//

/** @file
 * String set and indexed string set (strings have a numeric ID that can be used
 * in switch statements)
 *
 * **Macros:** STRINGIFY and stringswitch
 * <br>
 * **Classes:** ccuty::stringset and ccuty::istringset
 *
 * @author Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
 */

#ifndef __ccuty_stringset__
#define __ccuty_stringset__

#include <string>
#include <vector>
#include <initializer_list>
#include <set>
#include "ccstring.hpp"

/// C++ Utilities.
namespace ccuty {

#if !defined(NO_STRINGIFY) && !defined(STRINGIFY)
  /// Stringify a symbol.
#  define STRINGIFY(s) _STRINGIFY(s)
#  define _STRINGIFY(s...) #s
#endif


#if !defined(NO_STRING_SWITCH) && !defined(STRING_SWITCH)
  /** @brief Allows comparing strings in switch statements.
   * Example:
   * @code
   *   int getRGB(const std::string& color) {
   *     STRING_SWITCH(color, red, blue, green) {
   *       case red: return 0xf00;
   *       case blue: return 0x0f0;
   *       case green: return 0x00f;
   *       default: return 0;
   *     }
   *   }
   * @endcode
   * This macro relies on ccuty::stringset.
   */
#define STRING_SWITCH(str, ...) \
enum {__VA_ARGS__}; \
static const ccuty::stringset _stringset_ ## __LINE__(#__VA_ARGS__); \
switch(_stringset_ ## __LINE__[str])
#endif


  /** @brief Set of strings, allows converting strings to numbers.
   * Manages a set of strings (here named tokens). Tokens are associated with a
   * unique numeric ID, which is >=0 and can be used in switch statements for 
   * comparing strings. Provides functions to insert tokens, to test if a token 
   * belongs to the set and to retrieve the ID of a token.
   *
   * Example:
   * @code
   *   int getRGB(const std::string& color) {
   *     enum {red, blue, green};
   *     static const stringset colors("red, blue, green");
   *     switch (colors[color]) {
   *       case red: return 0xf00;
   *       case blue: return 0x0f0;
   *       case green: return 0x00f;
   *       default: return 0;
   *     }
   *   }
   * @endcode
   * Tokens must be in the same order in the enum and in the string thar is given
   *  as an argument to the stringset constr. If the labels have specific values,
   * proceed as follows:
   * @code
   *     enum {red=1, blue=10, green=100};
   *     static const stringset colors({"red",red}, {"blue",blue}, {"green",green});
   *     switch (colors[color]) {
   *       // etc.
   *     }
   * @endcode
   * @see Macro STRING_SWITCH.
   * @see Class istringset if tokens need to be retreived from their numeric ID.
   * @see cutty::strid() function which provides another way to compare strings
   * in switch statements.
   */
  class stringset {
  public:
    struct elem {
      elem(const std::string& str, unsigned int id) : str(str), id(id) {}
      const std::string str;
      const unsigned int id;
    };
    
    struct string_comp {
      bool operator()(const elem& left, const elem& right) const {return left.str<right.str;}
    };
    
    using string_set = std::set<elem, string_comp>;

    // - - - -
    
    /// @brief No argument constructor.
    stringset() {}
    
    /** @brief Constructor: _str_ is a C string containing tokens separated by commas.
     * Each token is associated with an ID which corresponds to its position in
     * the list (starting from 0). Duplicate tokens are ignored.
     */
    explicit stringset(const char* str) {
      std::vector<std::string> v;
      ccuty::strsplit(v, str, ",");
      append(v);
    }
    
    /** @brief Constructor: _str_ is a C++ string containing tokens separated by commas.
     * Each token is associated with an ID which corresponds to its position in
     * the list (starting from 0). Duplicate tokens are ignored.
     */
    explicit stringset(const std::string& str) {
      std::vector<std::string> v;
      ccuty::strsplit(v, str, ",");
      append(v);
    }
    
    /** @brief Constructor: _tokens_ is a sequence of tokens (each token is C string).
     * Each token is associated with an ID which corresponds to its position in
     * the list (starting from 0). Duplicate tokens are ignored.
     */
    stringset(std::initializer_list<const char*> tokens) {
      append(tokens);
    }
    
    /** @brief Constructor: _tokens_ is a sequence of {token, ID} couples.
     * IDs must be >= 0. Multiple couples can use the same ID but not the same
     * token (duplicate tokens are ignored).
     */
    stringset(std::initializer_list<elem> tokens) {
      append(tokens);
    }
    
    /** @brief Constructor: _tokens_ is a container (vector, list, map) of tokens or {token, ID} couples.
     * Duplicate tokens are ignored. IDs must be >= 0.
     */
    template <typename T> explicit stringset(const T& tokens) {
      append(tokens);
    }
    
    /// @brief Tests whether the stringset is empty.
    bool empty() const {
      return elements_.empty();
    }
    
    /// @brief Returns the size of the stringset.
    size_t size() const {
      return elements_.size();
    }
    
    /// @brief Returns the internal set of elements (each containing a token and its IDs).
    const string_set& elements() const {return elements_;}
    
    /// Returns the ID of _token_ or -1 if not found.
    int operator[](const std::string& token) const {
      auto it = elements_.find(elem{token,0});
      return it==elements_.end() ? -1 : it->id;
    }

    /** @brief Returns the ID of the token which starts with _str_.
     *  Returns -1 if not found and -2 if several tokens match.
     */
    int starts_with(const std::string& str) const {
      int count = 0;
      int value = -1;
      for (auto& it : elements_) {
        if (it.str.compare(0, str.length(), str) == 0) {
          if (it.str.length() == str.length()) return it.id;  // exact match
          else value = it.id;
          count++;
        }
      }
      if (count == 0) return -1;          // no match
      else if (count == 1) return value;  // one match
      else return -2;                     // multiple matches
    }
    
    /// @brief Clears the stringset.
    void clear() {
      elements_.clear();
    }
    
    /** @brief Inserts _token_ in the stringset.
     * Duplicate tokens are ignored. Return the ID of _token_.
     */
    int insert(const std::string& token) {
      auto p = elements_.insert(elem{token, lastid_});
      if (p.second) lastid_++;   // not a duplicate
      return p.first->id;
    }
    
    /** @brief Inserts _token_ in the stringset.
     * Duplicate tokens are ignored. Return the ID of _token_.
     */
    int insert(const char* token) {
      auto p = elements_.insert(elem{token, lastid_});
      if (p.second) lastid_++;
      return p.first->id;
    }
    
    /** @brief Inserts a {token, ID} couple in the stringset.
     * Duplicate tokens are ignored. Return the ID of _token_.
     */
    int insert(const elem& e) {
      auto p = elements_.insert(e);
      if (p.second) {
        if (e.id >= lastid_) lastid_ = e.id + 1;
        //else if (e.id < 0) p.first->id = lastid_++;
      }
      return p.first->id;
    }
    
    /** @brief Inserts the elements of a container (vector, list, map) of tokens or {token, ID} couples.
     * Duplicate tokens are ignored. IDs must be >= 0.
     */
    template <typename T> void append(const T& cont) {
      for (auto&& it : cont) insert(it);
    }
    
  protected:
    unsigned int lastid_{0};
    string_set elements_;
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /** @brief Indexed set of strings, allows converting strings to numbers and vice-versa.
   * This class provides the same functionnalities as ccutty::stringset but also
   * allows converting numbers to strings.
   */
  class istringset : public stringset {
  public:
    struct comp_id {
      bool operator()(const elem& left, const elem& right) const {return left.id < right.id;}
    };
    using id_set = std::set<elem, comp_id>;

    /// returns an empty string.
    static const std::string& none() {
      static std::string empty;
      return empty;
    }
    
    // - - - -

    /// @brief No argument constructor.
    istringset() {}

    /** @brief Constructor: _str_ is a C string containing tokens separated by commas.
     * Each token is associated with an ID which corresponds to its position in
     * the list (starting from 0). Duplicate tokens are ignored.
     */
    explicit istringset(const char* str) {
      std::vector<std::string> v;
      ccuty::strsplit(v, str, ",");
      append(v);
    }
    
    /** @brief Constructor: _str_ is a C++ string containing tokens separated by commas.
     * Each token is associated with an ID which corresponds to its position in
     * the list (starting from 0). Duplicate tokens are ignored.
     */
    explicit istringset(const std::string& str) {
      std::vector<std::string> v;
      ccuty::strsplit(v, str, ",");
      append(v);
    }
    
    /** @brief Constructor: _tokens_ is a sequence of tokens (each token is C string).
     * Each token is associated with an ID which corresponds to its position in
     * the list (starting from 0). Duplicate tokens are ignored.
     */
    explicit istringset(std::initializer_list<const char*> tokens) {
      append(tokens);
    }
    
    /** @brief Constructor: _tokens_ is a sequence of {token, ID} couples.
     * IDs must be >= 0. Multiple couples can use the same ID but not the same
     * token (duplicate tokens are ignored).
     * @note if several couples use the same ID, operator[](unsigned int id)
     * will return the first token with this ID.
     */
    istringset(std::initializer_list<elem> tokens) {
      append(tokens);
    }
    
    /** @brief Constructor: _tokens_ is a container (vector, list, map) of tokens or {token, ID} couples.
     * Duplicate tokens are ignored. IDs must be >= 0.
     */
    template <typename T> explicit istringset(const T& tokens) {
      append(tokens);
    }

    /// Returns the internal set of IDs.
    const id_set& ids() const {return ids_;}
    
    /// Returns the ID of _token_ or -1 if not found.
    int operator[](const std::string& token) const {
      return stringset::operator[](token);
    }
    
    /// Returns the token corresponding to this ID or istringset::none() if not found.
    const std::string& operator[](unsigned int id) const {
      auto it = ids_.find(elem{"", id});
      if (it==ids_.end()) return none(); else return it->str;
    }
    
    /// @brief Clears the istringset.
    void clear() {
      elements_.clear();
      ids_.clear();
    }
    
    /** @brief Inserts _token_ in the istringset.
     * Duplicate tokens are ignored. Return the ID of _token_.
     */
    int insert(const std::string& token) {
      auto p = elements_.insert(elem{token, lastid_});
      if (p.second) {  // not a duplicate
        lastid_++;
        ids_.insert(*p.first);
      }
      return p.first->id;
    }
    
    /** @brief Inserts _token_ in the istringset.
     * Duplicate tokens are ignored. Return the ID of _token_.
     */
    int insert(const char* token) {
      auto p = elements_.insert(elem{token, lastid_});
      if (p.second) {
        lastid_++;
        ids_.insert(*p.first);
      }
      return p.first->id;
    }
    
    /** @brief Inserts a {token, ID} couple in the istringset.
     * Duplicate tokens are ignored. Return the ID of _token_.
     */
    int insert(const elem& e) {
      auto p = elements_.insert(e);
      if (p.second) {
        if (e.id >= lastid_) lastid_ = e.id + 1;
        //else if (e.id < 0) p.first->id = lastid_++;
        ids_.insert(*p.first);
      }
      return p.first->id;
    }
    
    /** @brief Inserts the elements of a container (vector, list, map) of tokens or {token, ID} couples.
     * Duplicate tokens are ignored. IDs must be >= 0.
     */
    template <typename T> void append(const T& cont) {
      for (auto& it : cont) insert(it);
    }
    
  protected:
    id_set ids_;
  };

}
#endif
