#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <list>
#include <future>
#include <atomic>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


#include "thread_manager_future.hpp"

std::atomic<int> at;

void test()
{
    throw "a";
}

void test1()
{
    try {
        test();
    } catch(...) {
        std::cout << "error" << std::endl;
    }
}

int get_value() { 
    std::cout << "끝!!!" << std::endl;
    return 10; 
}

int child_pid = 0;

void timer_handler(int pid) {
      printf("(%ld) time is over, child will be killed\n", time(0));
        kill(child_pid, SIGKILL);
}

int main(int argc, char** argv)
{
    signal(SIGALRM, timer_handler);
    signal(SIGCHLD, timer_handler);

#if 0
    at++;
    at--;
    std::cout << at.load() << std::endl;
    std::future<int> a;
    a = std::async (get_value);

    if(a.valid()) {
        std::cout << "vaild" << std::endl;
    } else {
        std::cout << "in vaild" << std::endl;
    }
    std::this_thread::sleep_for (std::chrono::seconds(3));

    if(a.valid()) {
        std::cout << "vaild" << std::endl;
    } else {
        std::cout << "in vaild" << std::endl;
    }


    auto status = a.wait_for(std::chrono::seconds(0));
    if (status == std::future_status::ready) {
        std::cout << "thread finished" << std::endl;
        a.get();
    } else {
        std::cout << "thread still running" << std::endl;
    }

    std::this_thread::sleep_for (std::chrono::seconds(6));

    if(a.valid()) {
        std::cout << "vaild" << std::endl;
    } else {
        std::cout << "in vaild" << std::endl;
    }
#endif

    ThreadManager thread_manager;

    std::cout << thread_manager.GetThreadPool() << std::endl;

    thread_manager.RegisterThread(
#if 1
    []() {
        sigset_t mask;
        sigset_t orig_mask;
        struct timespec timeout;
        timeout.tv_sec = 2;
        timeout.tv_nsec = 0;

        sigemptyset (&mask);
        sigaddset (&mask, SIGCHLD);
        if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
          //  perror ("sigprocmask");
        }


        //    pid_t parent = getpid();
        pid_t pid = fork();
        child_pid = pid;
        if (pid == -1)
        {
            // error, failed to fork()
        } 
        else if (pid > 0)
        {
         //   alarm(10);
         
            do {
                int ret = sigtimedwait(&mask, NULL, &timeout);
                if (ret < 0) {
                    if (errno == EINTR) {
                        printf ("Interrupted by a signal other than SIGCHLD\n");
                        // Interrupted by a signal other than SIGCHLD.
                        continue;
                    } else if (errno == EAGAIN) {
                        printf ("(%ld) !!! Timeout, killing child\n", time(0));
                        kill (pid, SIGKILL);
                    } else {
                        printf ("(%ld) !! sigtimedwait\n", time(0));
                        break;
                    }
                } else {
                    printf ("Recive SIGCHLD");
                }
                break;
            } while (1);

            int status;
            waitpid(pid, &status, 0);
            std::cout << "wait pid " << child_pid << " status  : " << WEXITSTATUS(status) << std::endl;
        }
        else 
        {
        // we are the child
        execl("./suarez", "suarez",0);

        //       execve(...);

        _exit(EXIT_FAILURE);   // exec never returns
        }
    }

#else 
            []()
                {
                    while(1) {
                        std::cout << "ok!!!!" << std::endl;
                        std::this_thread::sleep_for (std::chrono::seconds(10));
                        std::cout << "thread is exit" << std::endl;
                        break;
                    }
                }
#endif
            );

    std::cout << thread_manager.GetRunningThreadCount() << std::endl;
    std::cout << thread_manager.GetRemainThreadCount() << std::endl;
    std::this_thread::sleep_for (std::chrono::seconds(3));
    std::cout << thread_manager.GetRunningThreadCount() << std::endl;
    std::cout << thread_manager.GetRemainThreadCount() << std::endl;
    thread_manager.Stop();

    return 0;
}
