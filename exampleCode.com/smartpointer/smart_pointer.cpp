/* Copyright (C) 
* 2021 - doitnowman
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
#include <iostream>
#include <memory>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <dlfcn.h>
#include "./test.hpp"


class Smartptr
{
public:
    Smartptr() {
        std::cout << "생성된다" << std::endl;
    }
    ~Smartptr(){
        std::cout << "소멸된다" << std::endl;
    }
    std::string name = "smartptr name";
};

int main(int argc, char** argv)
{
    char buff[PATH_MAX];
    char *res = realpath(argv[0], buff);
    std::cout << buff << std::endl;

    std::cout << chdir(dirname(buff)) << std::endl;
    std::cout << chroot(dirname(buff)) << std::endl;

    void *handle = dlopen("libcall_test.so", RTLD_NOW);

    if (handle) {
        typedef int (*calltest_t)();
        calltest_t calltest = (calltest_t)dlsym(handle, "calltest"); 
        char *error;

        if((error = dlerror()) != NULL) { 
            printf("dlsym error: %s\n", error);
            dlclose(handle); 
            return -1; 
        }

        std::cout << "before error" <<std::endl;
        calltest();
        //std::cout<<  "c!!!!! " << calltest() << std::endl;
        dlclose(handle);
    } else {
        std::cout << "dl open fail " <<std::endl;
    }
    DIR *dir = opendir("./");
    dirent *list;
    while ((list = readdir(dir)) != NULL) {
        std::cout << list -> d_name << "  ";
    }
    std::cout << std::endl;
    closedir(dir);
    std::shared_ptr<Smartptr> sp(new Smartptr);
    std::shared_ptr<Smartptr> sp1(new Smartptr);
    std::weak_ptr<Smartptr> wp1 = sp;
    sp1 = wp1.lock();

    std::cout << "sp name addres  : " << &(sp->name) << std::endl;
    std::cout << "sp count : " << sp.use_count() << std::endl;
    sp.reset();
    std::cout << "sp1 count : " << sp1.use_count() << std::endl;
    sp1.reset();
    std::cout << "wp1 count : " << wp1.use_count() << std::endl;




    return 0;
}
