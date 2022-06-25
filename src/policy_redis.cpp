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


using std::string;


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
      validpathstr = validpathstr + ":" + path
    }

    std::cout << "round_branches incr:" << incr << " vbranches_ " << branches_->to_string() << " validpathstr " << validpathstr << " index " << index << " basepath " << basepath  << std::endl;

    return 0;
  }

  static
  int 
  search_path_none_redis(const Branches::CPtr &branches_,
                        const char           *fusepath_,
                        StrVec               *paths_,
                        Config::Read         &cfg)
  {
    int rv;

    rv = Policies::Search::epff(branches_,fusepath_,paths_);
    if(rv == 0)
    {
      return 0;
    }

    StrVec combinedirs = fs::string2vec(cfg->combinedirs);
    fs::combinedir(branches_, fusepath_, combinedirs, paths_);

    if (!paths_->empty())
    {
      return 0;
    }

    string fusedirpath = fs::path::dirname(fusepath_);
    return Policies::Search::eprand(branches_,fusedirpath.c_str(),paths_);
  }

  static
  int 
  search_path_from_redis(const Branches::CPtr &branches_,
                        const char           *fusepath_,
                        StrVec               *paths_)
  {
    // std::cout << "policy_redis::create, fusepath_" << fusepath_ << std::endl;
    Config::Read cfg;
    int redis_err = 0;
    auto basepath = Redis::hget(Redis::redis_key, fusepath_, &redis_err);
    if (redis_err != 0) {
      return search_path_none_redis(branches_, fusepath_, paths_, cfg);
    }

    if (basepath) {
      // std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
      paths_->push_back(*basepath);
      return 0;
    }

    if (cfg->combinedirs.to_string().empty())
    {
      std::cerr << "cfg->combinedirs == null" << std::endl;
      return -1;
    }

    string fusedirpath = fs::path::dirname(fusepath_);
    string basename = fs::path::basename(fusepath_);
    StrVec combindir = fs::string2vec(cfg->combinedirs);

    if (fs::is_in_combinedir(fusedirpath, combindir)) {
      for(const auto &dir : combindir)
      {
        string field = fs::path::make(dir.c_str(), basename.c_str());
        basepath = Redis::hget(Redis::redis_key, field, &redis_err);
        if (redis_err != 0) {
          return search_path_none_redis(branches_, fusepath_, paths_, cfg);
        }

        if (basepath) {
          // std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
          paths_->push_back(*basepath);
          return 0;
        }
      }

      // 均匀落盘
      ::redis::round_branches(branches_, paths_);
      return 0;
    }

    basepath = Redis::hget(Redis::redis_key, fusedirpath, &redis_err);
    if (redis_err != 0) {
      return search_path_none_redis(branches_, fusepath_, paths_, cfg);
    }

    if (basepath) {
      // std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
      paths_->push_back(*basepath);
      return 0;
    }

    // 均匀落盘
    ::redis::round_branches(branches_, paths_);
    return 0;
  }

  static
  int
  create(const Branches::CPtr &branches_,
         const char           *fusepath_,
         StrVec               *paths_)
  {
    return search_path_from_redis(branches_, fusepath_, paths_);
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
    return search_path_from_redis(branches_, fusepath_, paths_);
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
