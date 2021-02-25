//
//  ccstring: useful functions for C++ strings
//  (C) Eric Lecolinet 2016/17 - https:www.telecom-paristech.fr/~elc
//

/** @file
 * Useful functions for C++ strings (trim, split, trimsplit, icompare, tolower, etc.)
 * @author Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
 */

#ifndef ccuty_ccstring
#define ccuty_ccstring

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <regex>

/// C++ Utilities.
namespace ccuty {

  /** @brief Returns a numeric ID from last characters that can be used in `switch` blocks.
   * The returned ID relies on the last 8 characters of the string (or on the last
   * 8 characters up to the _last_ position if _last_ is provided).
   * Because this function is a `constexpr`, it is computed at compile time and 
   * can be used in `switch` blocks.
   *
   * Example:
   * @code
   *   int getRGB(const std::string& color) {
   *     switch (strid(color)) {
   *       case strid("red"): return 0xf00;
   *       case strid("green"): return 0x0f0;
   *       case strid("blue"): return 0x00f;
   *       default: return 0;
   *     }
   *   }
   * @endcode
   *
   * @see ccuty::strid(const std::string&) for C++ strings.
   * @see ccuty:stringset, which provides similar functionality but takes all 
   * characters into account.
   */
  inline constexpr unsigned long long strid(const char* str,
                                            int last = 32767,
                                            unsigned long long id = 0) {
    return (!*str || last==0) ? id : ccuty::strid(str+1, last-1, (id<<8) + *str);
  }
  
  /** @brief Returns a numeric ID from last characters that can be used in `switch` blocks.
   * @see ccuty::strid(const char*)
   */
  inline unsigned long long strid(const std::string& str, int last = 32767) {
    return ccuty::strid(str.c_str(), last, 0);
  }
  
  /** @brief Delimiters for strsplit(), regsplit(), strtoken() and strtrim().
   */
  struct strdelim {
  
    /// contructor that specifies that all whitespaces are delimiters.
    strdelim() {}
  
    /// contructor that sets the delimiters (see pattern()).
    strdelim(const std::string& delimiters) {pattern(delimiters);}
    
    /// contructor that sets the delimiters (see pattern()).
    strdelim(const char* delimiters) {pattern(delimiters);}
    
    /// resets the strdelim so that it can be used again (useful only for strtoken()).
    void reset() {pos_ = 0; size_ = 0;}

    /** changes delimiters.
     *  _delimiters_ specifies the delimiters. 
     *  All whitespaces match if _delimiters_ is the empty string. Otherwise:
     *  - 1) With strplit() and strtoken(), all characters are taken as delimiters;
     *  - 2) With regplit(), _delimiters_ is a regex;
     *  - 3) With strtrim(), _delimiters_ are removed on the left/right side of the string.
     */
    void pattern(const std::string& delimiters) {
      pattern_ = delimiters;
      if (delimiters.empty()) spaces_ = true;
    }
    
    /// specifies that all whitespaces are delimiters.
    strdelim& spaces(bool val = true) {spaces_ = val; return *this;}
    
    /// specifies that simple and double quotes are taken into account (no effect with regsplit()).
    strdelim& quotes(bool val = true) {quotes_ = val; return *this;}

    /// specifies that tokens must be trimmed (the default).
    strdelim& trim(bool val = true) {trim_ = val; return *this;}
    
    /// changes the starting position in the string (default is 0).
    void pos(size_t val) {pos_ = val;}
    
    /// returns the position in the string after calling strtoken().
    size_t pos() const {return pos_;}
    
    /// returns the delimited that was found after calling strtoken()).
    char matched() const {return matched_;}
  
    // impl.
    size_t pos_{0}, size_{0};
    std::string pattern_;
    char matched_{0};
    bool spaces_{false}, quotes_{false}, trim_{true};
  };
  
  /** @brief Splits a string into a vector of tokens.
   *  Splits _str_ into tokens that are stored in _tokens_.
   *  If greater than 0, _limit_ is the maximum number of tokens and the last
   *  token contains the remaining text.
   *  _delimiters_ specifies the delimiters. All whitespaces match if _delimiters_ is the empty string.
   *  By default, the tokens are trimmed. See ccuty::strdelim for more options.
   *
   *  @code
   *      std::vector<std::string> tokens;
   *      strsplit(tokens, "res = one + two", "=+");                       // [1]
   *      strsplit(tokens, "res one\ttwo", "");                            // [2]
   *      strsplit(tokens, "res one\ttwo", " ");                           // [3]
   *      strsplit(tokens, "res = one + two", strdelim("=+").trim(false)); // [4]
   *      strsplit(tokens, "res = one two", strdelim("=").spaces());       // [5]

   *  @endcode
   *  - [1] '=' and '+' are delimiters. Tokens are trimmed (the default).
   *    _tokens_ = {"res", "one", "two"}
   *  - [2] all whitespaces are delimiters (including \\t).
   *    _tokens_ = {"res", "one", "two"}
   *  - [3] only ' ' is a delimiter.
   *    _tokens_ = {"res", "one\ttwo"}
   *  - [4] same as [1] except that tokens are *not* trimmed.
   *    _tokens_ = {" res ", " one ", " two" }
   *  - [5] '=' and whitespaces are delimiters.
   *    _tokens_ = {"res", "one", "two"}
   *
   *  @return the number of tokens (i.e. _tokens_.size()).
   *  @see ccuty::strdelim for more options.
   */
  inline size_t strsplit(std::vector<std::string>& tokens,
                         const std::string& str, ccuty::strdelim delimiters, int limit = 0);
  
  /** @brief Splits a string into two tokens.
   *  Works as ccuty::strsplit() except that this function splits _str_ into two 
   *  tokens that are stored in _left_ and _right_.
   *
   *  @code
   *      std::string left, right;
   *      strsplit(left, right, " name :  Bob", ":");                        // [1]
   *      strsplit(left, right, " name :  Bob", strdelim(":").trim(false));  // [2]
   *  @endcode
   *  - [1] _left_ contains "name" and _right_ contains "Bob" (tokens are trimmed).
   *  - [2] _left_ contains " name " and _right_ contains "  Bob" (tokens are not trimmed).
   *
   *  @return the number of tokens (e.g. 0, 1 or 2).
   *  @see ccuty::strsplit() for examples and details.
   */
  inline size_t strsplit(std::string& left, std::string& right,
                         const std::string& str, ccuty::strdelim delimiters);
  
  /** @brief Splits a string into a vector of tokens using a `regex`.
   *  Works as ccuty::strsplit() except that the delimiter is a `regex`.
   *
   *  @code
   *      std::vector<std::string> tokens;
   *      regsplit(tokens, " res = one +  two", "=|+");                       // [1]
   *      regsplit(tokens, " res = one +  two", strdelim("=|+").trim(false)); // [2]
   *  @endcode
   *  - [1] _tokens_ = "res" "one, "two" (tokens are trimmed).
   *    Note the | in "=|+" because the delimiter is a regex.
   *  - [2] _tokens_ = " res " " one " "  two" (tokens are not trimmed).
   *
   *  @note This function is slower than strsplit().
   *  The strdelim::quotes() option has no effect.
   *
   *  @see ccuty::strsplit() for examples and details.
   */
  inline size_t regsplit(std::vector<std::string>& tokens,
                         const std::string& str, ccuty::strdelim delimiters, int limit = 0);

  /** @brief Retrieves the next token in a string.
   *  Retrieves the next token _token_ in string _str_.
   *  _delimiters_ specifies the delimiters. All whitespaces match if _delimiters_ is the empty string.
   *
   *  @return true if a token was found.
   *
   *  @note _delimiters_ is modified each time this function is called.
   *  strdelim::reset() must be called before using the same _delimiters_ variable with another string.
   *
   *  @see ccuty::strdelim for more options.
   */
  bool strtoken(std::string& token, const std::string& str, ccuty::strdelim& delimiters);
  
  /** @brief Removes beginning and trailing characters.
   *  Removes beginning and trailing characters that belongs to _toremove_.
   *  Removes all whitespaces if _toremove_ is the empty string.
   *  @see ccuty::strdelim for more options.
   */
  inline void strtrim(std::string& str, ccuty::strdelim toremove);
  
  /// Removes beginning and trailing whitespaces.
  inline void strtrim(std::string& str) {
    ccuty::strtrim(str, ccuty::strdelim(""));
  }
  
  /// Removes beginning/trailing quotes and whitespaces.
  inline void unquote(std::string& str) {
    strtrim(str, ccuty::strdelim("").quotes());
  }
  
  /// Returns a copy without beginning and trailing spaces.
  inline std::string trimmed(const std::string& str) {
    std::string result(str);
    ccuty::strtrim(result, "");
    return result;
  }
  
  /// Returns a copy without beginning/trailing quotes and whitespaces.
  inline std::string unquoted(const std::string& str) {
    std::string result(str);
    ccuty::unquote(result);
    return result;
  }

  /// Compares two ASCII strings ignoring case (relies on tolower(int)).
  inline int icompare(const std::string& a, const std::string& b) {
    size_t len = a.length() < b.length() ? a.length() : b.length();
    for (size_t k=0; k<len; ++k) {
      if (::tolower(a[k]) < ::tolower(b[k])) return -1;
      else if (::tolower(a[k]) > ::tolower(b[k])) return +1;
    }
    return a.length()==b.length() ? 0 : a.length()<b.length() ? -1 : +1;
  }
  
  /// Compares two ASCII strings ignoring case (relies on tolower(int)).
  inline bool iequal(const std::string& str1, const std::string& str2) {
    return str1.length()!=str2.length() ? false : icompare(str1, str2)==0;
  }
  
  /// Lowercases an ASCII string (relies on tolower(int)).
  inline void lowerize(std::string& str) {
    for (auto& c : str) c = char(::tolower(c));
  }
  
  /// Uppercases an ASCII string (relies on tolower(int)).
  inline void upperize(std::string& str) {
    for (auto& c : str) c = char(::toupper(c));
  }
  
  /// Returns an lowercased copy of an ASCII string (relies on toupper(int)).
  inline std::string tolower(const std::string& str) {
    std::string s(str); ccuty::lowerize(s); return s;
  }
  
  /// Returns an uppercased copy of an ASCII string (relies on tolower(int)).
  inline std::string toupper(const std::string& str) {
    std::string s(str); ccuty::upperize(s); return s;
  }
  
  /// Converts string to bool (returns true if the value could be successfully converted).
  inline bool from_string(bool& var, const std::string& str) {
    if (str == "true") {var = true; return true;}
    else if (str == "false") {var = false; return true;}
    else {var = false; return false;}
  }
  
  /// Converts string to long (returns true if the value could be successfully converted).
  inline bool from_string(long& var, const std::string& str) {
    char* p{nullptr};
    errno = 0;
    var = ::strtol(str.c_str(), &p, 0);
    if (p == nullptr || errno != 0) {var = 0; return false;} else return true;
  }
  
  /// Converts string to unsigned long (returns true if the value could be successfully converted).
  inline bool from_string(unsigned long& var, const std::string& str) {
    char* p{nullptr};
    errno = 0;
    var = ::strtoul(str.c_str(), &p, 0);
    if (p == nullptr || errno != 0) {var = 0; return false;} else return true;
  }
  
  /// Converts string to float (returns true if the value could be successfully converted).
  inline bool from_string(float& var, const std::string& str) {
    char* p{nullptr};
    errno = 0;
    var = ::strtof(str.c_str(), &p);
    if (p == nullptr || errno != 0) {var = 0; return false;} else return true;
  }
  
  /// Converts string to double (returns true if the value could be successfully converted).
  inline bool from_string(double& var, const std::string& str) {
    char* p{nullptr};
    errno = 0;
    var = ::strtod(str.c_str(), &p);
    if (p == nullptr || errno != 0) {var = 0; return false;} else return true;
  }
  
  /// Converts string to long double (returns true if the value could be successfully converted).
  inline bool from_string(long double& var, const std::string& str) {
    char* p{nullptr};
    errno = 0;
    var = ::strtold(str.c_str(), &p);
    if (p == nullptr || errno != 0) {var = 0; return false;} else return true;
  }
  
  /// Calls the C `system()` function.
  inline int system(const std::string& command) {
    return ::system(command.c_str());
  }
  
  // - - - implementation - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  inline void strtrim(std::string& str, ccuty::strdelim d) {
    if (str.empty()) return;
    if (d.size_ == 0) d.size_ = str.size();
    if (d.pos_ >= d.size_) return;
    
    const char *p = str.c_str() + d.pos_;
    const char *end = str.c_str() + d.size_;

    while (p < end   // trim left
           && ((d.spaces_ && ::isspace(*p))
               || d.pattern_.find(*p) != std::string::npos)
           ) ++p;
    if (p >= end) {str.clear(); return;}
    
    --end;
    while (end >= p   // trim right
           && ((d.spaces_ && ::isspace(*end))
               || d.pattern_.find(*end) != std::string::npos)
           ) --end;
    if (end < p) {str.clear(); return;}
    
    if (d.quotes_) {
      if (*p == '"' || *p == '\'') ++p;
      if (*end == '"'  || *end == '\'') --end;
      if (end < p) {str.clear(); return;}
    }
    str.assign(p, end-p+1);
  }
  
  inline bool strtoken(std::string& token, const std::string& str, ccuty::strdelim& d) {
    token.clear();
    d.matched_ = 0;
    if (d.size_ == 0) d.size_ = str.size();
    if (d.pos_ >= d.size_) {d.matched_ = 0; return false;}
    
    const char* p = str.c_str() + d.pos_;
    char delim = 0;
    
    if (d.trim_) {
      while (*p != 0 && ::isspace(*p)) ++p;  // skip spaces
      if (*p == 0) {d.pos_ = d.size_; d.matched_ = 0; return false;}
    }
    const char* begtok = p;
    
    if (d.quotes_ && (*p == '"' || *p == '\'')) {  // quoted token
      char quote = *p++;
      while (*p != 0 && *p != quote) ++p;
      if (*p == 0) {    // ending quote not found, taking rest
        token.assign(begtok+1, p-begtok);
        d.pos_ = d.size_; d.matched_ = 0; return true;
      }
      else {
        token.assign(begtok+1, p-begtok-1);
        ++p; // after quote
        delim = ::isspace(*p) ? ' ' : quote;
        goto after_token;
      }
    }
    else {   // unquoted string
      while (*p != 0       // find delim
             && (!d.spaces_ || !::isspace(*p))
             && (!d.quotes_ || (*p != '"' && *p != '\''))
             && d.pattern_.find(*p) == std::string::npos) ++p;
      
      if (d.spaces_ && ::isspace(*p)) {
        token.assign(begtok, p-begtok);
        delim = ' ';
        goto after_token;
      }
      else if (d.quotes_ && (*p == '"' || *p == '\'')) {
        token.assign(begtok, p-begtok);
        delim = *p;
        goto after_token;
      }
      else {
        const char* endtok = p-1;
        if (d.trim_) {
          while (endtok > begtok && ::isspace(*endtok)) --endtok;
        }
        token.assign(begtok, endtok-begtok+1);
        if (*p == 0) {
          d.pos_ = d.size_; d.matched_ = 0; return true;
        }
        else {
          d.pos_ = p - str.c_str() + 1; d.matched_ = *p; return true;
        }
      }
    }
    return false;
    
  after_token:
    if (d.trim_) {
      while (*p != 0 && ::isspace(*p)) ++p;
    }
    if (*p == 0) {
      d.pos_ = d.size_; d.matched_ = 0; return true;
    }
    // nb: returns delim rather then space when both conds apply
    else if (d.pattern_.find(*p) != std::string::npos) {
      d.pos_ = p - str.c_str() + 1;
      d.matched_ = *p;
      return true;
    }
    else {  // delim is quote or space
      d.pos_ = p - str.c_str() + (d.trim_ ? 0 : 1);
      d.matched_ = delim;
      return true;
    }
  }

  inline size_t strsplit(std::string& left, std::string& right, const std::string& str,
                         ccuty::strdelim delim) {
    std::string s(str);     // str may be the same string as left or right
    left.clear(); right.clear();
    size_t n{0};
    if (ccuty::strtoken(left, s, delim)) n++;
    delim.pattern_.clear();
    delim.spaces(false);
    if (n==0) {if (ccuty::strtoken(left, s, delim)) n++;}
    else {if (ccuty::strtoken(right, s, delim)) n++;}
    return n;
  }
  
  inline size_t strsplit(std::vector<std::string>& tokens, const std::string& str,
                         ccuty::strdelim delim, int limit) {
    tokens.clear();
    std::string tok;
    if (limit <= 0) limit = 32767;
    while (int(tokens.size()) < limit-1 && ccuty::strtoken(tok, str, delim)) tokens.push_back(tok);
    delim.pattern_.clear();
    delim.spaces(false);
    if (ccuty::strtoken(tok, str, delim) && !tok.empty()) tokens.push_back(tok);
    return tokens.size();
  }
  
  inline size_t regsplit(std::vector<std::string>& tokens, const std::string& str,
                         ccuty::strdelim delim, int limit) {
    tokens.clear();
    if (str.empty()) return 0;
    std::regex expr(delim.spaces_ ? "[:space:]"+delim.pattern_ : delim.pattern_);
    const char *p = str.c_str(), *prefix = p;
    
    auto trimRight = [&tokens](const char* beg, const char* end, bool atEnd) {
      while (end >= beg && ::isspace(*end)) --end;
      if (end < beg) {if (!atEnd) tokens.push_back("");}
      else tokens.push_back(std::string(beg, end-beg+1));
    };
    
    while (limit <= 0 || int(tokens.size()) < limit-1) {
      if (delim.trim_) { // trim left
        while (*p != 0 && ::isspace(*p)) ++p;
        if (*p == 0) {prefix = p; break;}
      }
      prefix = p;
      std::cmatch m;
      if (!std::regex_search(p, m, expr)) break;
      else {
        if (!delim.trim_) tokens.push_back(std::string(prefix, p-prefix+m.position()));
        else (trimRight)(prefix, p+m.position()-1, false);
        p += m.position() + m.length();
        prefix = p;
      }
    }
    
    if (*prefix) {
      if (!delim.trim_) tokens.push_back(std::string(prefix));
      else {
        while (*prefix != 0 && ::isspace(*prefix)) ++prefix;
        if (*prefix != 0) (trimRight)(prefix, prefix+::strlen(prefix)-1, true);
      }
    }
    return tokens.size();
  }
}

#endif
