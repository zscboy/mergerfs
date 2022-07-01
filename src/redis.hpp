// #include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>
#include "strvec.hpp"

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
        static sw::redis::OptionalString hget(string hash, string field, int *err);
        static long long incr(string key);
        static long long hdel(string key, string field);
        static void delete_data(StrVec paths);
        static long long sadd(string key, string member);
        static long long srem(string key, string member);
        static void set_path(string fusepath, string basepath);
        static void remove_path(string fusepath, string basepath);

        static long long del(string key);

        static long long hdel(string key, std::vector::iterator first, std::vector::iterator last);
        static void smembers(string key,  std::back_insert_iterator<StrVec> output);
        

        static const string redis_file2disk_hash_key;
        static const string redis_disk2file_set_key;
        static const string redis_incr_key;

    private:
        static sw::redis::Redis *redis; 

};
