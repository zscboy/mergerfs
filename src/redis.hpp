#include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>

// using namespace sw::redis;
// #include <iostream>

using std::string;
class Redis
{
    public:
        static int init(string address)
        {
            if (address == NULL || address == "")
            {
                std::cerr << "redis server address can not empty" << std::endl;
                return -1;
            }

            // string url = "tcp://" + address + "?socket_timeout=50ms&connect_timeout=1s"
            try {
                 redis = sw::redis::Redis(address);
            }
            catch (const sw::redis::Error &e) {
                std::cerr << " instance redis error " << e.what() << std::endl;
            }
        }

        static bool set(string key, string value)
        {
            if (redis == NULL) {
                 std::cerr << "redis instance == null" << std::endl;
                return -1;
            }

            try {
                return redis.set(key, value);
            }
            catch (const sw::redis::Error &e) {
                 std::cerr << " redis set error " << e.what() << std::endl;
            }
        }

        
        static sw::redis::OptionalString get(string key)
        {
            if (redis == NULL) {
                 std::cerr << "redis instance == null" << std::endl;
                return -1;
            }

            try {
                return redis.get(key);
            }
            catch (const sw::redis::Error &e) {
                 std::cerr << " redis get error " << e.what() << std::endl;
            }
        }

        static bool hset(string hash, string field, string value)
        {
            if (redis == NULL) {
                 std::cerr << "redis instance == null" << std::endl;
                return -1;
            }

            try {
                redis.hset(hash, field, field);
            }
            catch (const sw::redis::Error &e) {
                 std::cerr << " redis hset error " << e.what() << std::endl;
            }
        }

        static sw::redis::OptionalString hget(string hash, string field)
        {
            if (redis == null) {
                 std::cerr << "redis instance == null" << std::endl;
                return -1;
            }

            try {
                return redis.hget(hash, field);
            }
            catch (const sw::redis::Error &e) {
                 std::cerr << " redis hget error " << e.what() << std::endl;
            }
        }
 
    private:
        static sw::redis::Redis redis; 
        Redis() {}

};
