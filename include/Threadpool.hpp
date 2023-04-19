#pragma once

#include "TaskQue.hpp"
#include "Logger.h"
#include "Server.h"

#include <memory>
#include <iostream>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <utility>

using namespace std;

class ThreadPool
{
public:
    explicit ThreadPool(const int threads = 4) : work_threads(vector<thread>(threads)), m_shutdown(false)
    {
    }
    void init() //初始化线程池
    {
        for (int i = 0; i < work_threads.size(); ++i)
        {
            work_threads[i] = thread(Work(this, i)); //分配工作线程
        }
    }
    void shutdown() //关闭线程池
    {
        m_shutdown = true;
        cond.notify_all();
        for (int i = 0; i < work_threads.size(); ++i)
        {
            if (work_threads[i].joinable())
            {
                work_threads[i].join();
            }
        }
    }
    //禁止拷贝构造和移动语义
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    template <typename F, typename... Args>                                 //多参数的函数模板
    auto submit(F &&f, Args &&...args) -> void //多参数的函数模板化
    {
        //内传内置参数
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); //避免左右值的歧义
        auto taskptr = make_shared<packaged_task<decltype(f(args...))()>>(func);                                 //创建了一个智能指针 std::packaged_task的绑定构造 用于期物的创建
        function<void()> task_func = [taskptr]()
        {
            (*taskptr)();
        };                            //任意类型的返回值
        tasks.push(task_func);        //压入任务队列
        cond.notify_one();            //唤醒其中的一个线程
        return ; //返回一个期物 用于在不同线程调用函数返回值
    }

    ~ThreadPool()
    {
        shutdown(); //线程池析构的时候进行关闭
    }

private:
    class Work
    {
    public:
        Work(ThreadPool *pool, int id) : m_pool(pool), m_id(id)
        { //初始化工作线程
        }
        void operator()() //重载（）开始工作
        {
            //基础任务函数
            std::function<void()> task;
            //是否正在进行出队
            bool dequeued;
            while (!m_pool->m_shutdown)
            {
                {
                    std::unique_lock<std::mutex> locker(m_pool->m_mutex);
                    if(m_pool->tasks.empty())
                    {
                        m_pool->cond.wait(locker);
                    }
                    dequeued = m_pool->tasks.pop(task);
                }
                if (dequeued)
                {
                    task(); //执行任务
                }
            }
        }

    private:
        int m_id;           //工作线程的id
        ThreadPool *m_pool; //所属的线程池
    };
    
    TaskQue<std::function<void()>> tasks; // std::function 用于包装函数
    vector<thread> work_threads;
    condition_variable cond;
    mutex m_mutex;
    bool m_shutdown;
};
