#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"




//线程池类，将它定义为模板类是为了代码复用，参数T是任务类
template<typename T>
class threadpool
{
public:
    threadpool( int thread_number = 8, int max_request = 10000);
    ~threadpool();
    bool append(T *request);  //将任务加入到请求队列 

private:
    static void *worker(void *arg);//工作线程运行的函数，它不断从工作队列中取出任务并执行
    void run();

private:
    int m_thread_number;        //线程池中的线程数
    int m_max_requests;         //请求队列中允许的最大请求数
    pthread_t *m_threads;       //描述线程池的数组，其大小为m_thread_number
    std::list<T *> m_workqueue; //请求队列
    locker m_queuelocker;       //保护请求队列的互斥锁
    sem m_queuestat;            //信号量，用来表示是否有任务需要处理
    bool m_stop;                //是否结束线程
};



template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false),m_threads(NULL)
{
    printf("初始化线程池！\n");
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
        throw std::exception();
    //创建thread_number个线程，并将他们设置为脱离线程
    for (int i = 0; i < thread_number; ++i)
    {
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}


template <typename T>
threadpool<T>::~threadpool()
{
    printf("线程池析构！\n");
    delete[] m_threads;
    m_stop = true;
}


template <typename T>
bool threadpool<T>::append(T *request)
{
    printf("执行append()函数！\n");
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}


template <typename T>
void *threadpool<T>::worker(void *arg)
{
    printf("执行work()函数!\n");
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}


template <typename T>
void threadpool<T>::run()
{
    printf("执行run()函数!\n");
    while(!m_stop)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T * request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
        {
            continue;
        }
        request->process();
    }
}

#endif

