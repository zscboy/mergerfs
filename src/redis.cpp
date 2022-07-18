// #include <sw/redis++/redis++.h>
#include "redis.hpp"
#include <iostream>
#include "strvec.hpp"
#include <initializer_list>

sw::redis::Redis *Redis::redis;
const string Redis::redis_file2disk_hash_key = "file2disk";
const string Redis::redis_disk2file_set_key = "disk2file:";
const string Redis::redis_errpath_set_key = "errpath:";
const string Redis::redis_incr_key = "incr";

using std::string;

  Redis::Redis() {}
  Redis::~Redis(){
      delete redis;
  }  

  int Redis::init(string address)
    {
        if (address.empty())
        {
            std::cerr << "redis server address can not empty" << std::endl;
            return -1;
        }

        // string url = "tcp://" + address + "?socket_timeout=50ms&connect_timeout=1s"
        try {
            sw::redis::ConnectionOptions connection_options(address);
            sw::redis::ConnectionPoolOptions pool_options;
            pool_options.size = 10; 

            redis = new sw::redis::Redis(connection_options, pool_options);
            // redis = &instance;
            return 0;
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " instance redis error " << e.what() << std::endl;
            return -1;
        }
    }

    bool Redis::isInit() 
    {
        return redis != NULL;
    }

    long long Redis::exists(string key)
    {
        if (redis == NULL) {
                std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->exists(key);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis set error " << e.what() << std::endl;
            return 0;
        }
    }

    bool Redis::set(string key, string value)
    {
        if (redis == NULL) {
                std::cerr << "redis instance == NULL" << std::endl;
            return false;
        }

        try {
            return redis->set(key, value);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis set error " << e.what() << std::endl;
            return false;
        }
    }


    sw::redis::OptionalString Redis::get(string key)
    {
        if (redis == NULL) {
                std::cerr << "redis instance == NULL" << std::endl;
            return sw::redis::OptionalString();
        }

        try {
            return redis->get(key);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis get error " << e.what() << std::endl;
            return sw::redis::OptionalString();;
        }
    }

    bool Redis::hset(string hash, string field, string value)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return false;
        }

        try {
            return redis->hset(hash, field, value);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis hset error " << e.what() << std::endl;
            return false;
        }
    }

    
    long long Redis::hset(string hash,  std::unordered_map<std::string, std::string> m)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->hset(hash,  m.begin(), m.end());
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis hset error " << e.what() << std::endl;
            for (it = m.begin(); it != m.end(); it++)
            {
                 std::cerr << it->first  << ":" << it->second  << std::endl;
            }
            return 0;
        }
    }

    sw::redis::OptionalString Redis::hget(string hash, string field, int *err)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return sw::redis::OptionalString();
        }

        try {
            return redis->hget(hash, field);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis hget error " << e.what() << std::endl;
            if (err)
            {
              *err = -1;
            }
            return sw::redis::OptionalString();;
        }
    }

    long long Redis::hdel(string key, string field)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->hdel(key, field);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis hdel error " << e.what() << std::endl;
            return 0;
        }
    }

    long long Redis::hdel(string key, StrVec::iterator first,  StrVec::iterator last)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->hdel(key, first, last);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis hdel error " << e.what() << std::endl;
            return 0;
        }
    }

    long long Redis::incr(string key)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->incr(key);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis incr error " << e.what() << std::endl;
            return 0;
        }
    }

    void Redis::delete_data(StrVec paths)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return;
        }

        try {
            redis->del(redis_file2disk_hash_key);
            for (const auto &path : paths) 
            {
                string key = redis_disk2file_set_key + path;
                redis->del(key);
            }
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis del error " << e.what() << std::endl;
            return;
        }
    }

    long long Redis::sadd(string key, string member)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->sadd(key,member);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis sadd error " << e.what() << std::endl;
            return 0;
        }
    }

    long long Redis::sadd(string key, StrVec::iterator first,  StrVec::iterator last)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->sadd(key, first, last);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis sadd error " << e.what() << std::endl;
            return 0;
        }
    }

    bool Redis::sismember(string key, string member)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return false;
        }

        try {
            return redis->sismember(key, member);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis sismember error " << e.what() << std::endl;
            return false;
        }
    }

    long long Redis::srem(string key, string member)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->srem(key,member);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis srem error " << e.what() << std::endl;
            return 0;
        }
    }

    long long Redis::del(string key)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return 0;
        }

        try {
            return redis->del(key);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis del error " << e.what() << std::endl;
            return 0;
        }
    }

    void Redis::smembers(string key,  std::back_insert_iterator<StrVec> output)
    {
        if (redis == NULL) {
            std::cerr << "redis instance == NULL" << std::endl;
            return;
        }

        try {
            return redis->smembers(key, output);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis smembers error " << e.what() << std::endl;
            return;
        }
    }
    
    void Redis::set_path(string fusepath, string basepath)
    {
        hset(redis_file2disk_hash_key, fusepath, basepath);

        string key = redis_disk2file_set_key + basepath;
        sadd(key, fusepath);
    }

    void Redis::remove_path(string fusepath, string basepath)
    {
        hdel(redis_file2disk_hash_key, fusepath);

        string key = redis_disk2file_set_key + basepath;
        srem(key, fusepath);
    }


