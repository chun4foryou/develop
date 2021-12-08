#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <list>

#include "thread_manager.hpp"


int main(int argc, char** argv)
{

    std::list<std::string> a;
    a.push_back("aaa");

    std::cout << "empty : " << a.empty() << std::endl;


    std::mutex lock;

    {
        std::lock_guard<std::mutex> a(lock);
    }
    {
        std::lock_guard<std::mutex> a(lock);
    }

    ThreadManager thread_manager;

    std::cout << thread_manager.GetThreadPool() << std::endl;

    thread_manager.RegisterThread([]()
                {
                    while(1) {
                        std::cout << "ok!!!!" << std::endl;
                        std::this_thread::sleep_for (std::chrono::seconds(10));
                        std::cout << "thread is exit" << std::endl;
                        break;
                    }
                }
            );

    std::cout << thread_manager.GetRunningThreadCount() << std::endl;
    std::cout << thread_manager.GetRemainThreadCount() << std::endl;
    std::this_thread::sleep_for (std::chrono::seconds(3));
    std::cout << thread_manager.GetRunningThreadCount() << std::endl;
    std::cout << thread_manager.GetRemainThreadCount() << std::endl;
    thread_manager.Stop();

    return 0;
}
