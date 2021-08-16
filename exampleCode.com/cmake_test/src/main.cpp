#include <iostream>
#include <main.hpp>
#include <hello.hpp>

std::string Version()
{
    return PROG_VERSION;
}

int main(int argc, char** argv)
{
    std::cout << Version() << std::endl;
    std::cout << HelloLibVersion() << std::endl;
    return 0;
}
