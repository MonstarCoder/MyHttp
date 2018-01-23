#ifndef MYHTPP_THREADS_H_
#define MYHTPP_THREADS_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include<queue>
#include<vector>

struct Job
{
    // 线程回调函数指针
    void* (*callback_func)(void* args);
    // 回调函数参数
    void* args;
};

// // 线程函数
// void* thread_func(void* args);

class Threads
{
private:
    // 线程函数
    static void* thread_func(void* args);
public:
    explicit Threads(int n = 10);
    ~Threads();
    // 向线程池添加任务，成功返回0，失败-1
    int add_task(void*(*callback_func)(void* args), void* args);
private:
    bool stop_; // 是否停止
    int max_threads_; // 线程池最大个数限制 
    // int curr_threads_; // 当前线程池中总的线程数 
    // int idle_threads_; // 当前线程池中空闲的线程数 
    pthread_mutex_t mtx_;
    pthread_cond_t cond_;
    pthread_t* tid_;
    std::queue<Job*> tasks_; // 任务队列
};

#endif