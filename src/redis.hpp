// #include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>

using std::string;
class Redis
{
    public:
        Redis();
        ~Redis();
        static int init(string address);
        static bool set(string key, string value);
        static sw::redis::OptionalString get(string key);
        static bool hset(string hash, string field, string value);
        static sw::redis::OptionalString hget(string hash, string field);
        static long long Redis::incr(string key);

        const string redis_key = "file2disk";
        const string redis_incr_key = "incr";

    private:
        static sw::redis::Redis *redis; 

};
