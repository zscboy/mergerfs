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

#include "fs_path.hpp"
#include "mergerfs.hpp"
#include "option_parser.hpp"
#include "resources.hpp"
#include "strvec.hpp"

#include "fuse_access.hpp"
#include "fuse_bmap.hpp"
#include "fuse_chmod.hpp"
#include "fuse_chown.hpp"
#include "fuse_copy_file_range.hpp"
#include "fuse_create.hpp"
#include "fuse_destroy.hpp"
#include "fuse_fallocate.hpp"
#include "fuse_fchmod.hpp"
#include "fuse_fchown.hpp"
#include "fuse_fgetattr.hpp"
#include "fuse_flock.hpp"
#include "fuse_flush.hpp"
#include "fuse_free_hide.hpp"
#include "fuse_fsync.hpp"
#include "fuse_fsyncdir.hpp"
#include "fuse_ftruncate.hpp"
#include "fuse_futimens.hpp"
#include "fuse_getattr.hpp"
#include "fuse_getxattr.hpp"
#include "fuse_init.hpp"
#include "fuse_ioctl.hpp"
#include "fuse_link.hpp"
#include "fuse_listxattr.hpp"
#include "fuse_lock.hpp"
#include "fuse_mkdir.hpp"
#include "fuse_mknod.hpp"
#include "fuse_open.hpp"
#include "fuse_opendir.hpp"
#include "fuse_poll.hpp"
#include "fuse_prepare_hide.hpp"
#include "fuse_read_buf.hpp"
#include "fuse_readdir.hpp"
#include "fuse_readdir_plus.hpp"
#include "fuse_readlink.hpp"
#include "fuse_release.hpp"
#include "fuse_releasedir.hpp"
#include "fuse_removexattr.hpp"
#include "fuse_rename.hpp"
#include "fuse_rmdir.hpp"
#include "fuse_setxattr.hpp"
#include "fuse_statfs.hpp"
#include "fuse_symlink.hpp"
#include "fuse_truncate.hpp"
#include "fuse_unlink.hpp"
#include "fuse_utimens.hpp"
#include "fuse_write_buf.hpp"

#include "fuse.h"

#include <cstdlib>
#include <iostream>

#include <string.h>

#include "redis.hpp"
#include "fuse_refresh_redis.hpp"

const string policy_redis = "redis";
namespace l
{
  static
  void
  get_fuse_operations(struct fuse_operations &ops_,
                      const bool              nullrw_)
  {
    ops_.access          = FUSE::access;
    ops_.bmap            = FUSE::bmap;
    ops_.chmod           = FUSE::chmod;
    ops_.chown           = FUSE::chown;
    ops_.copy_file_range = FUSE::copy_file_range;
    ops_.create          = FUSE::create;
    ops_.destroy         = FUSE::destroy;
    ops_.fallocate       = FUSE::fallocate;
    ops_.fchmod          = FUSE::fchmod;
    ops_.fchown          = FUSE::fchown;
    ops_.fgetattr        = FUSE::fgetattr;
    ops_.flock           = FUSE::flock;
    ops_.flush           = FUSE::flush;
    ops_.free_hide       = FUSE::free_hide;
    ops_.fsync           = FUSE::fsync;
    ops_.fsyncdir        = FUSE::fsyncdir;
    ops_.ftruncate       = FUSE::ftruncate;
    ops_.futimens        = FUSE::futimens;
    ops_.getattr         = FUSE::getattr;
    ops_.getxattr        = FUSE::getxattr;
    ops_.init            = FUSE::init;
    ops_.ioctl           = FUSE::ioctl;
    ops_.link            = FUSE::link;
    ops_.listxattr       = FUSE::listxattr;
    ops_.lock            = FUSE::lock;
    ops_.mkdir           = FUSE::mkdir;
    ops_.mknod           = FUSE::mknod;
    ops_.open            = FUSE::open;
    ops_.opendir         = FUSE::opendir;
    ops_.poll            = FUSE::poll;;
    ops_.prepare_hide    = FUSE::prepare_hide;
    ops_.read_buf        = (nullrw_ ? FUSE::read_buf_null : FUSE::read_buf);
    ops_.readdir         = FUSE::readdir;
    ops_.readdir_plus    = FUSE::readdir_plus;
    ops_.readlink        = FUSE::readlink;
    ops_.release         = FUSE::release;
    ops_.releasedir      = FUSE::releasedir;
    ops_.removexattr     = FUSE::removexattr;
    ops_.rename          = FUSE::rename;
    ops_.rmdir           = FUSE::rmdir;
    ops_.setxattr        = FUSE::setxattr;
    ops_.statfs          = FUSE::statfs;
    ops_.symlink         = FUSE::symlink;
    ops_.truncate        = FUSE::truncate;
    ops_.unlink          = FUSE::unlink;
    ops_.utimens         = FUSE::utimens;
    ops_.write_buf       = (nullrw_ ? FUSE::write_buf_null : FUSE::write_buf);

    return;
  }

  static
  void
  setup_resources(void)
  {
    const int prio = -10;

    std::srand(time(NULL));
    resources::reset_umask();
    resources::maxout_rlimit_nofile();
    resources::maxout_rlimit_fsize();
    resources::setpriority(prio);
  }
  void println_policy_name(Config::Read &cfg)
  {
    std::cout << "access " << cfg->func.access.policy.name() << std::endl;
    std::cout << "chmod " << cfg->func.chmod.policy.name()  << std::endl;
    std::cout << "chown " << cfg->func.chown.policy.name()  << std::endl;
    std::cout << "create " << cfg->func.create.policy.name()  << std::endl;
    std::cout << "getattr " << cfg->func.getattr.policy.name()  << std::endl;
    std::cout << "getxattr " << cfg->func.getxattr.policy.name()  << std::endl;
    std::cout << "link " << cfg->func.link.policy.name()  << std::endl;

    std::cout << "listxattr " << cfg->func.listxattr.policy.name()  << std::endl;
    std::cout << "mkdir " << cfg->func.mkdir.policy.name()  << std::endl;
    std::cout << "mknod " << cfg->func.mknod.policy.name()  << std::endl;
    std::cout << "open " << cfg->func.open.policy.name()  << std::endl;
    std::cout << "readlink " << cfg->func.readlink.policy.name()  << std::endl;
    std::cout << "removexattr " << cfg->func.removexattr.policy.name()  << std::endl;

    std::cout << "rename " << cfg->func.rename.policy.name()  << std::endl;
    std::cout << "rmdir " << cfg->func.rmdir.policy.name()  << std::endl;
    std::cout << "setxattr " << cfg->func.setxattr.policy.name()  << std::endl;
    std::cout << "symlink " << cfg->func.symlink.policy.name()  << std::endl;
    std::cout << "truncate " << cfg->func.truncate.policy.name()  << std::endl;
    std::cout << "unlink " << cfg->func.unlink.policy.name()  << std::endl;
    std::cout << "utimens " << cfg->func.utimens.policy.name()  << std::endl;
  }

  bool checkCfg(Config::Read &cfg)
  {
    if (cfg->func.create.policy.name() == policy_redis)
    {
      if (cfg->func.getattr.policy.name() != policy_redis) 
      {
        std::cerr << "not set category.search=redis !" << std::endl;
        return false;
      }
    }

    if (cfg->func.getattr.policy.name() == policy_redis)
    {
      if (cfg->func.create.policy.name() != policy_redis)
      {
        std::cerr << "not set category.create=redis !" << std::endl;
        return false;
      }

      if (cfg->combinedirs.to_string().empty())
      {
        std::cerr << "not set combinedirs!" << std::endl;
        return false;
      }

      if (cfg->redis.to_string().empty())
      {
        std::cerr << "not set redis!" << std::endl;
        return false;
      }
    }

    return true;
  }

  int
  main(const int   argc_,
       char      **argv_)
  {
    Config::Read    cfg;
    Config::ErrVec  errs;
    fuse_args       args;
    fuse_operations ops;

    memset(&ops,0,sizeof(fuse_operations));

    args.argc      = argc_;
    args.argv      = argv_;
    args.allocated = 0;

    options::parse(&args,&errs);
    if(errs.size())
      std::cerr << errs << std::endl;

    l::setup_resources();
    l::get_fuse_operations(ops,cfg->nullrw);

    // println_policy_name(cfg);
    // if (!checkCfg(cfg))
    // {
    //   return 0;
    // }

    if (cfg->func.create.policy.name() == "redis")
    {
      int r = Redis::init((std::string)cfg->redis);
      if (r < 0) {
        std::cerr << "init redis failed, please check !" << std::endl;
        return 0;
      }

      // test redis
      int redis_err = 0;
      Redis::hget(Redis::redis_file2disk_hash_key, "test", &redis_err);
      if (redis_err !=  0)
      {
        std::cerr << "init redis failed, please check !" << std::endl;
        return 0;
      }

      FUSE::refresh_redis(cfg->branches);

      std::cout << "refresh_redis complete" << std::endl;
    }

    return fuse_main(args.argc,
                     args.argv,
                     &ops);
  }
}

int
main(int    argc_,
     char **argv_)
{
  return l::main(argc_,argv_);
}
