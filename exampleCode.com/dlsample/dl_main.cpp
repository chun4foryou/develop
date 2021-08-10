#include <iostream>
#include <dlfcn.h>
#include "./test_func.hpp"

int main(int argc, char** argv)
{
    void *handle = dlopen("libtest_func.so", RTLD_NOW);
    char *error = nullptr;

    if (handle) {
        typedef int (*test_dl_func_t)();
        test_dl_func_t test_dl_func = (test_dl_func_t)dlsym(handle, "test_dl_func"); 

        if((error = dlerror()) != NULL) { 
            printf("dlsym error: %s\n", error);
            dlclose(handle); 
            return -1; 
        }

        test_dl_func();
        dlclose(handle);
    } else {
        if((error = dlerror()) != NULL) { 
            printf("dlopen error: %s\n", error);
            return -1; 
        }
    }
}
