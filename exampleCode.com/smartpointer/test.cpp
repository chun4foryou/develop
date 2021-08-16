#include <iostream>

extern "C" int calltest()
{
    std::cout << "calltest" << std::endl;
    return 0;
}
