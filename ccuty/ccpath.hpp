//
//  ccpath: C++ functions for managing Unix pathnames.
//  (C) Eric Lecolinet 2017/2018 - www.telecom-paristech.fr/~elc
//

/** @file
 * Functions for managing Unix pathnames.
 * @author Eric Lecolinet 2017/2018 - https://www.telecom-paristech.fr/~elc
 */

#ifndef __ccuty_path__
#define __ccuty_path__

#include <string>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/// C++ Utilities.
namespace ccuty {
  
  /// Returns the filename component of the pathname (with the extension if _with_extension_ is true).
  inline std::string basename(const std::string& path, bool with_extension = true) {
    if (path.empty()) return ".";
    if (path.size()==1) return path;
    if (path[path.size()-1]=='/') return ccuty::basename(path.substr(0, path.size()-1));
    size_t pos = path.find_last_of('/');
    if (with_extension) return (pos==std::string::npos) ? path : path.substr(pos+1);
    else {
      ssize_t pos2 = path.size()-1, pos_min = (pos==std::string::npos) ? 0 : pos+1;
      while (pos2 >= pos_min && path[pos2]!='.') --pos2;
      if (pos2 < pos_min) return (pos==std::string::npos) ? path : path.substr(pos+1);
      else return (pos==std::string::npos) ? path.substr(0, pos2) : path.substr(pos+1, pos2-pos-1);
    }
  }
  
  /// Returns the directory component of the pathname.
  inline std::string dirname(const std::string& path) {
    if (path.empty()) return ".";
    if (path.size()==1) return path[0]=='/' ? "/" : ".";
    if (path[path.size()-1]=='/') return ccuty::dirname(path.substr(0, path.size()-1));
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return ".";  // no slash
    return pos== 0 ? "/" : path.substr(0, pos);
  }
  
  /// Returns the extension of the pathname (with the dot if _with_dot_ is true).
  inline std::string extname(const std::string& path, bool with_dot = true) {
    ssize_t pos = path.size()-1;
    while (pos >= 0 && path[pos]!='.' && path[pos]!='/') --pos;
    if (pos < 0 || path[pos]=='/') return "";  // no extension
    return with_dot ? path.substr(pos) : path.substr(pos+1);
  }
  
  /// Changes the extension of the pathname (_ext_ must start with a dot or be empty).
  inline std::string change_extname(const std::string& path, const std::string& ext) {
    ssize_t pos = path.size()-1;
    while (pos >= 0 && path[pos]!='.' && path[pos]!='/') --pos;
    if (pos < 0 || path[pos]=='/') return path + ext;  // no extension
    return path.substr(0, pos) + ext;
  }
  
  /// returns true if the file exists.
  inline bool file_exists(const std::string& path) {
    struct stat fstat;
    return (::stat(path.c_str(), &fstat) == 0);
  }
  
  /// Adds a final / to a path if not already the case.
  inline std::string add_slash(const std::string& path) {
    return (!path.empty() && path.back() != '/') ? path+'/' : path;
  }
  
  /// Creates a new path from 2 paths.
  inline std::string join_paths(const std::string& left, const std::string& right) {
    if (left.empty()) return right;
    else if (right.empty()) return left;
    else if (right[0] == '/') return left + right;
    else if (left.back() == '/') return left + right;
    else return left + '/' + right;
  }
}

#endif
