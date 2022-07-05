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

#include <string>
#include "redis.hpp"
#include <iostream>
#include "strvec.hpp"

using std::string;

namespace redis
{
  static
  inline
  void 
  handle_err_path(const char *fusepath_)
  {
    int err = 0;
    auto basepath = Redis::hget(Redis::redis_file2disk_hash_key, fusepath_, &err)
    if (err != 0)
    {
      std::cerr << "handle_err_path failed, err: " << err << std::endl;
      return;
    }

    if (!basepath)
    {
      std::cerr << "path " << fusepath_ < " not exist, not need to handle"<< std::endl;
      return;
    }

    const string path = *basepath;
    const string key = Redis::redis_disk2file_set_key + path;

    bool isExist = Redis::sismember(key, fusepath_);
    if (isExist)
    {
      std::cout << "path " << fusepath_ << " aready in errpath"<< std::endl;
      return;
    }

    StrVec paths;
    Redis::smembers(key, std::back_inserter(paths));
    Redis::sadd(key, paths.begin(), paths.end());
  }
}