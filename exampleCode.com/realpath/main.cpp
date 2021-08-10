#include <iostream>
#include <limits.h> /* PATH_MAX */
#include <libgen.h> /* dirname  */

int main(int argc, char** argv)
{
    char buff[PATH_MAX];

    char *res = realpath(argv[0], buff);
    if (res) {
        std::cout << buff << std::endl;
    }

    std::cout << chdir(dirname(buff)) << std::endl;
    std::cout << chroot(dirname(buff)) << std::endl;
    std::cout << dirname(buff) << std::endl;
}



