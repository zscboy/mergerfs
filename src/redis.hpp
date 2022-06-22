#include "config.hpp"
#include <string>
#include <sw/redis++/redis++.h>

using namespace sw::redis;
// #include <iostream>

using std::string;
class Redis
{
    public:
        static void init(string address)
        {

        }
 
    private:
        static Redis instance; 
        Redis() {}

};
