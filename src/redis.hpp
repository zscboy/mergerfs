// #include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>

// using namespace sw::redis;
// #include <iostream>

using std::string;
class Redis
{
    public:    
        Redis(S const&)               = delete;
        void operator=(Redis const&)  = delete;

        static Redis getInstance();
        int init(string address);
        bool set(string key, string value);
        sw::redis::OptionalString get(string key);
        bool hset(string hash, string field, string value);
        sw::redis::OptionalString hget(string hash, string field);

    private:
        Redis() {} 
        Redis(Redis const&); 
        void operator=(Redis const&);

        sw::redis::Redis redis; 

};
