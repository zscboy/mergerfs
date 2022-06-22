// #include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>

// using namespace sw::redis;
// #include <iostream>

using std::string;
class Redis
{
    public:    
        // Redis(Redis const&)               = delete;
        // void operator=(Redis const&)  = delete;

        // static Redis& getInstance();
        static int init(string address);
        static bool set(string key, string value);
        static sw::redis::OptionalString get(string key);
        static bool hset(string hash, string field, string value);
        static sw::redis::OptionalString hget(string hash, string field);

    private:
        // Redis() {} 
        // Redis(Redis const&); 
        // void operator=(Redis const&);

        static sw::redis::Redis redis; 

};
