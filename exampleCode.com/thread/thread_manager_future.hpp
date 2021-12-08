#ifndef __thread_manger_cpp__
#define __thread_manger_cpp__

#include <iostream>
#include <thread>
#include <list>
#include <functional>
#include <mutex>
#include <algorithm>
#include <future>

class threadmanager
{
public:
    threadmanager(uint32_t _thread_pool_count = 10) {
        m_uthread_pool_count = _thread_pool_count;
//        run_thread_pool = std::thread(&threadmnagaer::runthreadpool, this);
        m_frun_thread_pool = std::async(std::launch::async, &threadmanager::runthreadpool, this);
    }

    ~threadmanager()
    {
        if (m_frun_thread_pool.wait_for(std::chrono::seconds(5))
                == std::future_status::timeout) {
            std::cout << "force stop" << std::endl;
        } else {
            if (0 == m_frun_thread_pool.get()) {
                std::cout << "end main thread" << std::endl;
            } else {
                std::cout << "force stop" << std::endl;
            }
        }
    }

    void stop() 
    {
        m_bstop_flag = true;
    }

    void registerthread(std::function<void()> func) 
    {
        std::lock_guard<std::mutex> guard(m_count_mutex);
        threadinfo t_info;
        t_info.status = 0;
        t_info.func = std::move(func);
        m_vthread_pool.push_back(std::move(t_info));

    }


    uint32_t getthreadpool() 
    {
        return m_uthread_pool_count;
    }

    uint32_t getrunningthreadcount()
    {
        std::lock_guard<std::mutex> guard(m_count_mutex);
        return m_vthread_pool.size();
    }

    uint32_t getremainthreadcount()
    {
        std::lock_guard<std::mutex> guard(m_count_mutex);
        return m_uthread_pool_count - m_vthread_pool.size();
    }

private:
    typedef struct threadinfo {
        uint32_t status;
        std::function<void()> func;
        std::future<void> f_thread;
    } threadinfo;

    std::list<threadinfo> m_vthread_pool;
    std::list<std::thread> m_vwait_thread_list;
//    std::thread run_thread_pool;
    std::future<int> m_frun_thread_pool;

    std::mutex m_count_mutex;
    uint32_t m_uthread_pool_count;
    bool m_bstop_flag = false;


    int runthreadpool() 
    {
        std::list<threadinfo>::iterator info;
        while (1) {
            if ((true == m_bstop_flag) && (getrunningthreadcount() == 0)) {
                std::cout << "종료!!"<< std::endl;
                return 0;
            }

            if (m_vthread_pool.size() > 0) {
                for(info = m_vthread_pool.begin(); info != m_vthread_pool.end(); info++) {
                    if (info->status == 0) {
                        info->f_thread = std::async(std::launch::async, info->func);
                        info->status = 1;
                    } else if (info->status == 1) {
                        auto status = info->f_thread.wait_for(std::chrono::seconds(0));
                        if (status == std::future_status::ready) {
                            std::cout << "thread finished" << std::endl;
                            info = m_vthread_pool.erase(info);
                        } else {
                            std::cout << "thread still running" << std::endl;
                        }
                    }
                }
            }
            std::this_thread::sleep_for (std::chrono::seconds(1));
        }
    }
};

#
