#include<iostream>
#include<mutex>
#include<queue>
#include<condition_variable>

template<typename T>

class TaskQue{
    
public:

TaskQue():ifstop(false){}

template<typename U>//相当于模板函数独立出来了
void push(U&&item)//万能引用
{
    {
        // static_assert(std::is_same<T,U>::==true);//判断模板类型是否一样
        std::lock_guard<std::mutex>locker(m_mutex);
        m_queue.push(std::forward<U>(item));//完美转发 保证之传进来的参数类型的属性（左值右值等等）
    }
    m_cond.notify_all();
}

bool pop(T&item)
{
    std::unique_lock<std::mutex>locker(m_mutex);
    while(m_queue.empty()||ifstop)
    {
        m_cond.wait(locker);
    }
    if(m_queue.empty())
    return false;
    item=(std::move(m_queue.front()));//右值引用本身就是左值
    m_queue.pop();
    return true;
}

size_t size()//const
{
    std::lock_guard<std::mutex>locker(m_mutex);
    return m_queue.size();
}

bool empty()
{
    std::lock_guard<std::mutex>locker(m_mutex);
    return m_queue.empty();
}

void stop()
{
    {
        std::lock_guard<std::mutex>locker(m_mutex);
        ifstop=true;
    }
    m_cond.notify_all();
}

private:
std::mutex m_mutex;
std::queue<T> m_queue;
std::condition_variable m_cond;
bool ifstop;
};
