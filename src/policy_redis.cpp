/*
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

#include "errno.hpp"
#include "fs_exists.hpp"
#include "fs_info.hpp"
#include "fs_path.hpp"
#include "fs_statvfs_cache.hpp"
#include "policy.hpp"
#include "policy_epmfs.hpp"
#include "policy_error.hpp"

#include <limits>
#include <string>
#include "redis.hpp"
#include "config.hpp"
#include "fs_combine_dir.hpp"
#include "policy_eprand.hpp"
#include "policy_epff.hpp"

#include <iostream>
#include <chrono>
#include <sys/time.h>
#include <ctime>

using std::chrono::duration_cast;


using std::string;

const long long maxIncr = 9223372036854775807;


namespace redis
{
  static
  int
  round_branches(const Branches::CPtr &branches_, StrVec  *paths_)
  {
    StrVec validpaths;
    fs::info_t info;
    int error;
    int rv;

    for(const auto &branch : *branches_)
    {
        if(branch.ro_or_nc())
          error_and_continue(error,EROFS);
        rv = fs::info(branch.path,&info);
        if(rv == -1)
          error_and_continue(error,ENOENT);
        if(info.readonly)
          error_and_continue(error,EROFS);
        if(info.spaceavail < branch.minfreespace())
          error_and_continue(error,ENOSPC);

        validpaths.push_back(branch.path);
    }

    auto incr = Redis::incr(Redis::redis_incr_key);
    const int index = incr % validpaths.size();
    const string basepath = validpaths[index];
    paths_->push_back(basepath);

    string validpathstr;
    for (const auto &path : validpaths) {
      validpathstr = validpathstr + ":" + path;
    }

    if (incr >= maxIncr) {
      Redis::set(Redis::redis_incr_key, "0");
    }
    // std::cout << "round_branches incr:" << incr << " vbranches_ " << branches_->to_string() << " validpathstr " << validpathstr << " index " << index << " basepath " << basepath  << std::endl;

    return 0;
  }


  static
  int
  create(const Branches::CPtr &branches_,
         const char           *fusepath_,
         StrVec               *paths_)
  {
    const string sealedpath = "/sealed";
    const string cachepath = "/cache";
    Config::Read cfg;

    string searchpath;
    string fusedirpath = fs::path::dirname(fusepath_);
    string basename = fs::path::basename(fusepath_);

    // 特殊处理
    if (cachepath == fusedirpath)
    {
      searchpath = fs::path::make(sealedpath.c_str(), basename.c_str());
    } 
    else if(sealedpath == fusedirpath)
    {
      // 特殊处理rsync类工具同步文件
      searchpath = fs::path::make(cachepath.c_str(), basename.c_str());
    }

    if (!searchpath.empty())
    {
      int rv = Policies::Search::epff(branches_, searchpath.c_str(), paths_);
      if(rv == -1)
      {
        return ::redis::round_branches(branches_, paths_);
      }
      return 0;
    }

    return Policies::Search::epff(branches_, fusedirpath.c_str(), paths_);
  }

  static
  int
  action(const Branches::CPtr &branches_,
         const char           *fusepath_,
         StrVec               *paths_)
  {
    int rv;
    int error;
    uint64_t epmfs;
    fs::info_t info;
    const string *basepath;

    error = ENOENT;
    epmfs = std::numeric_limits<uint64_t>::min();
    basepath = NULL;
    for(const auto &branch : *branches_)
      {
        if(branch.ro())
          error_and_continue(error,EROFS);
        if(!fs::exists(branch.path,fusepath_))
          error_and_continue(error,ENOENT);
        rv = fs::info(branch.path,&info);
        if(rv == -1)
          error_and_continue(error,ENOENT);
        if(info.readonly)
          error_and_continue(error,EROFS);
        if(info.spaceavail < epmfs)
          continue;

        epmfs = info.spaceavail;
        basepath = &branch.path;
      }

    if(basepath == NULL)
      return (errno=error,-1);

    paths_->push_back(*basepath);

    return 0;
  }

  static
  int
  search(const Branches::CPtr &branches_,
          const char           *fusepath_,
          StrVec               *paths_)
  {
    for(auto &branch : *branches_)
      {
        if(!fs::exists(branch.path,fusepath_))
          continue;

        paths_->push_back(branch.path);

        return 0;
      }

    return (errno=ENOENT,-1);
  }
}

int
Policy::REDIS::Action::operator()(const Branches::CPtr &branches_,
                                  const char           *fusepath_,
                                  StrVec               *paths_) const
{
  return ::redis::action(branches_,fusepath_,paths_);
}

int
Policy::REDIS::Create::operator()(const Branches::CPtr &branches_,
                                  const char           *fusepath_,
                                  StrVec               *paths_) const
{
  return ::redis::create(branches_,fusepath_,paths_);
}

int
Policy::REDIS::Search::operator()(const Branches::CPtr &branches_,
                                  const char           *fusepath_,
                                  StrVec               *paths_) const
{
  return ::redis::search(branches_,fusepath_,paths_);
}
