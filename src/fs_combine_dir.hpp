/*
  ISC License

  Copyright (c) 2016, Antonio SJ Musumeci <trapexit@spawn.link>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "fs_path.hpp"
#include "fs_exists.hpp"
#include "policy.hpp"

#include <string>
#include <sstream>
#include <iostream>

using std::string;
using std::stringstream;

namespace fs
{
  static
  inline
  void 
  combinedir(const Branches::CPtr &branches_,
             const char           *fusepath_,
             const StrVec         &combinedirs_,
             StrVec               *paths_)
  {
    string fusepath(fusepath_);
    bool exist = false;
    for(const auto &dir : combinedirs_)
    {
      if (fusepath.rfind(dir, 0) == 0) 
      {
        exist = true;
        break;
      }
    }

    if (!exist) 
    {
      return;
    }

    const string *basepath  = NULL;
    string basename = fs::path::basename(fusepath);
    for(const auto &branch : *branches_)
    {
      for(const auto &dir : combinedirs_)
      {
        string path = fs::path::make(dir, basename);
        if(fs::exists(branch.path, path))
        {
          basepath = &branch.path;
          break;
        }
      }
    }

    if (basepath != NULL)
    {
      paths_->push_back(*basepath);
    }

  }

  static
  inline
  StrVec 
  string2vec(const string combinedirs)
  {
    StrVec dirs;
 
    stringstream ss(combinedirs);
 
    while (ss.good()) {
        string substr;
        getline(ss, substr, ':');
        dirs.push_back(substr);
    }
 
    return dirs;
  }

  static
  inline
  bool 
  is_in_combinedir(const string &dirpath,  const StrVec &combinedirs_)
  {
      for(const auto &dir : combinedirs_)
      {
        string path = dir;
        char back = *path.rbegin();
        if (back == '/') {
          path = path.substr(0, path.size()-1);
          // std::cout << " isInCombinedir combinedir path " << path << " dirpath " << dirpath << std::endl;
        }

        if (path == dirpath) {
          return true;
        }
      }

      return false;
  }

  static
  inline
  std::string
  make(const string &base_, const string &suffix_)
  {
    char back;
    std::string path(base_);

    back = *path.rbegin();
    if((back != '/') && (suffix_[0] != '/'))
      path.push_back('/');
    path += suffix_;

    return path;
  }

  static
  inline
  std::string
  make(const string &base_, const char *suffix_)
  {
    char back;
    std::string path(base_);

    back = *path.rbegin();
    if((back != '/') && (suffix_[0] != '/'))
      path.push_back('/');
    path += suffix_;

    return path;
  }


}