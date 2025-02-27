/*
  ISC License

  Copyright (c) 2020, Antonio SJ Musumeci <trapexit@spawn.link>

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

#include "fuse_readdir_posix.hpp"
#include "fuse_readdir_linux.hpp"

#include "config.hpp"
#include "dirinfo.hpp"
#include "rwlock.hpp"
#include "ugid.hpp"

#include "fuse.h"
#include <iostream>


namespace FUSE
{
  int
  readdir(const fuse_file_info_t *ffi_,
          fuse_dirents_t         *buf_)
  {
    std::cout << "fuse_readdir::readdir " << std::endl;
    Config::Read cfg;
    DirInfo            *di = reinterpret_cast<DirInfo*>(ffi_->fh);
    const fuse_context *fc = fuse_get_context();
    const ugid::Set     ugid(fc->uid,fc->gid);
    std::cout << "fuse_readdir::readdir fusepath:" << di->fusepath << std::endl;
    switch(cfg->readdir)
      {
      case ReadDir::ENUM::LINUX:
        return FUSE::readdir_linux(cfg->branches,di->fusepath.c_str(),buf_);
      default:
      case ReadDir::ENUM::POSIX:
        return FUSE::readdir_posix(cfg->branches,di->fusepath.c_str(),buf_);
      }
  }
}
