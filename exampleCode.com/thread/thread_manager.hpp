#ifndef __THREAD_MANGER_CPP__
#define __THREAD_MANGER_CPP__

#include <iostream>
#include <thread>
#include <list>
#include <functional>
#include <mutex>
#include <algorithm>
#include <future>

class ThreadManager
{
public:
    ThreadManager(uint32_t _thread_pool_count = 10) {
        m_uThread_pool_count = _thread_pool_count;
        m_tRun_thread_pool = std::thread(&ThreadManager::RunThreadPool, this);
    }

    ~ThreadManager()
    {
        if (m_tRun_thread_pool.joinable()) {
            std::cout << "wait join" << std::endl;
            m_tRun_thread_pool.join();
            std::cout << "end join" << std::endl;
        }
    }

    void Stop() 
    {
        m_bStop_flag = true;
    }

    void RegisterThread(std::function<void()> func) 
    {
        std::lock_guard<std::mutex> guard(m_count_mutex);
        ThreadInfo t_info;
        t_info.status = 0;
        t_info.func = std::move(func);
        m_vThread_pool.push_back(std::move(t_info));

    }


    uint32_t GetThreadPool() 
    {
        return m_uThread_pool_count;
    }

    uint32_t GetRunningThreadCount()
    {
        std::lock_guard<std::mutex> guard(m_count_mutex);
        return m_vThread_pool.size();
    }

    uint32_t GetRemainThreadCount()
    {
        std::lock_guard<std::mutex> guard(m_count_mutex);
        return m_uThread_pool_count - m_vThread_pool.size();
    }

private:
    typedef struct ThreadInfo {
        uint32_t status;
        std::function<void()> func;
        std::future<void> f_thread;
    } ThreadInfo;

    std::list<ThreadInfo> m_vThread_pool;
    std::list<std::thread> m_vWait_Thread_list;
    std::thread m_tRun_thread_pool;

    std::mutex m_count_mutex;
    uint32_t m_uThread_pool_count;
    bool m_bStop_flag = false;


    void RunThreadPool() 
    {
        std::list<ThreadInfo>::iterator info;
        while (1) {
            if ((true == m_bStop_flag) && (GetRunningThreadCount() == 0)) {
                std::cout << "exit RunThreadPool" << std::endl;
                break;
            }
            if (m_vThread_pool.size() > 0) {
                for(info = m_vThread_pool.begin(); info != m_vThread_pool.end(); info++) {
                    if (info->status == 0) {
                        info->f_thread = std::async(std::launch::async, info->func);
                        info->status = 1;
                    } else if (info->status == 1) {
                        auto status = info->f_thread.wait_for(std::chrono::seconds(0));
                        if (status == std::future_status::ready) {
                            std::cout << "Thread finished" << std::endl;
                            info = m_vThread_pool.erase(info);
                        } else {
                            std::cout << "Thread still running" << std::endl;
                        }
                    }
                }
            }
            std::this_thread::sleep_for (std::chrono::seconds(1));
        }
    }
};

#endif //__THREAD_MANGER_CPP__
