#include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>
#include "redis.hpp"
// using namespace sw::redis;
// #include <iostream>

using std::string;
  Redis::Redis() {}
  Redis::~Redis(){}
          
  static int Redis::init(string address)
    {
        if (address.empty())
        {
            std::cerr << "redis server address can not empty" << std::endl;
            return -1;
        }

        // string url = "tcp://" + address + "?socket_timeout=50ms&connect_timeout=1s"
        try {
                auto instance = sw::redis::Redis(address);
                redis = &instance;
                return 0
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " instance redis error " << e.what() << std::endl;
            return -1;
        }
    }

    static bool Redis::set(string key, string value)
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


    static sw::redis::OptionalString Redis::get(string key)
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

    static bool Redis::hset(string hash, string field, string value)
    {
        if (redis == NULL) {
                std::cerr << "redis instance == NULL" << std::endl;
            return false;
        }

        try {
            redis->hset(hash, field, field);
        }
        catch (const sw::redis::Error &e) {
            std::cerr << " redis hset error " << e.what() << std::endl;
            return false;
        }
    }

    static sw::redis::OptionalString Redis::hget(string hash, string field)
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
            return sw::redis::OptionalString();;
        }
    }
