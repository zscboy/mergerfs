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

#include <iostream>


using std::string;


namespace redis
{
  static
  int
  search_uniform_path(const Branches::CPtr &branches_, StrVec  *paths_)
  {
    StrVec validpaths
    fs::info_t info;

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

    auto incr = Redis::incr(Redis::redis_incr_key)
    int index = incr % validpaths.size();
    paths_->push_back(validpaths[index]);

    return 0;
  }

  static
  int
  create(const Branches::CPtr &branches_,
         const char           *fusepath_,
         StrVec               *paths_)
  {
    Config::Read cfg;
    auto basepath = Redis::hget(Redis::redis_key, fusepath_);
    if (basepath) {
      std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
      paths_->push_back(*basepath);
      return 0;
    }

    if (cfg->combinedirs.empty())
    {
      std::cerr << "cfg->combinedirs == null" << std::endl;
      return -1;
    }

    string fusedirpath = fs::path::dirname(fusepath_);
    string basename = fs::path::basename(fusepath_);
    StrVec combindir = fs::string2Vec(cfg->combinedirs);

    if (fs::isInCombinedir(fusedirpath, combindir)) {
      for(const auto &dir : combindir)
      {
        string field = fs::make(dir, basename)
        basepath = Redis::hget(Redis::redis_key, field);
        if (basepath) {
          std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
          paths_->push_back(*basepath);
          return 0;
        }
      }

      // 均匀落盘
      ::redis::search_uniform_path(branches_, paths_);
      return 0;
    }

    basepath = Redis::hget(Redis::redis_key, fusedirpath);
    if (basepath) {
      std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
      paths_->push_back(*basepath);
      return 0;
    }

    // 均匀落盘
    ::redis::search_uniform_path(branches_, paths_);
    return 0;
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
    Config::Read cfg;
    auto basepath = Redis::hget(Redis::redis_key, fusepath_);
    if (basepath) {
      std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
      paths_->push_back(*basepath);
      return 0;
    }

    if (cfg->combinedirs.empty())
    {
      std::cerr << "cfg->combinedirs == null" << std::endl;
      return -1;
    }

    string fusedirpath = fs::path::dirname(fusepath_);
    string basename = fs::path::basename(fusepath_);
    StrVec combindir = fs::string2Vec(cfg->combinedirs);

    if (fs::isInCombinedir(fusedirpath, combindir)) {
      for(const auto &dir : combindir)
      {
        string field = fs::make(dir, basename);
        basepath = Redis::hget(Redis::redis_key, field);
        if (basepath) {
          std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
          paths_->push_back(*basepath);
          return 0;
        }
      }

      // 均匀落盘
      ::redis::search_uniform_path(branches_, paths_);
      return 0;
    }

    basepath = Redis::hget(Redis::redis_key, fusedirpath);
    if (basepath) {
      std::cout << "Redis find basepath " << fusepath_ << " => " << *basepath << std::endl;
      paths_->push_back(*basepath);
      return 0;
    }

    // 均匀落盘
    ::redis::search_uniform_path(branches_, paths_);
    return 0;
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
