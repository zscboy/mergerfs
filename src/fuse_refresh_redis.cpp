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
#include "fs_open.hpp"
#include "fs_devid.hpp"
#include "fs_getdents64.hpp"
#include "mempools.hpp"
#include "fs_close.hpp"
#include "linux_dirent64.h"
#include "fs_combine_dir.hpp"
#include "redis.hpp"

using std::string;

namespace l
{

  static
  void
  refresh_dir_to_redis(const string basepath_, const string dirpath_)
  {
    char *buf;
    int dirfd;
    int64_t nread;
    string fullpath;
    struct linux_dirent64 *d;

    buf = (char*)g_DENTS_BUF_POOL.alloc();
    if(buf == NULL)
    {
      std::cerr << " alloc memory failed, not enough memory !" << std::endl;
      return;
    }

    fullpath = fs::path::make(basepath_.c_str(),dirpath_.c_str());
    dirfd = fs::open_dir_ro(fullpath);
    if(dirfd == -1)
    {
      return;
    }

    for(;;)
    {
      nread = fs::getdents_64(dirfd,buf,g_DENTS_BUF_POOL.size());
      if(nread == -1)
        break;
      if(nread == 0)
        break;

      for(int64_t pos = 0; pos < nread; pos += d->reclen)
        {
          d = (struct linux_dirent64*)(buf + pos);
          if (strcmp(d->name, ".") == 0 || strcmp(d->name, "..") == 0) 
          {
            continue;;
          }

          const string filepath = fs::path::make(dirpath_.c_str(),d->name);
          fullpath = fs::path::make(basepath_.c_str(),filepath.c_str());

          struct stat st;
          int rv = fs::lstat(fullpath,&st);
          if(rv == -1)
            continue;
          else if(S_ISDIR(st.st_mode))
            refresh_dir_to_redis(basepath_, filepath);

          Redis::hset(Redis::redis_key, filepath, basepath_);
          std::cout << "refresh_dir_to_redis dir " << dirpath_ << " basepath_ " << basepath_ << " => " << filepath << std::endl;
        }
    }

    fs::close(dirfd);
    g_DENTS_BUF_POOL.free(buf);

  }


}

namespace FUSE
{
  void 
  refresh_redis(const Branches::CPtr &branches_, const string combinedirs)
  {
    const string root = "/";
    for(const auto &branch : *branches_)
    {
      l::refresh_dir_to_redis(branch.path, root);
    }
  }
}