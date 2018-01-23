#include "threads.h"

Threads::Threads(int n) : stop_(false), max_threads_(n)
{
    pthread_mutex_init(&mtx_, NULL);
    pthread_cond_init(&cond_, NULL);
    // mtx_ = PTHREAD_MUTEX_INITIALIZER;
    // cond_ = PTHREAD_COND_INITIALIZER;
    tid_ = new pthread_t[n];
    for (int i = 0; i < n; ++i)
    {
        pthread_create(&tid_[i], NULL, thread_func, (void*)this);
    } 
}

Threads::~Threads()
{
    pthread_mutex_lock(&mtx_);
    stop_ = true;
    pthread_mutex_unlock(&mtx_);
    pthread_cond_broadcast(&cond_);
    while (Job* job = tasks_.front())
        free(job);
    for (int i = 0; i < max_threads_; ++i)
        pthread_join(tid_[i], NULL);
    delete tid_;
}

// 增加任务
int Threads::add_task(void*(*callback_func)(void* args), void* args)
{
    pthread_mutex_lock(&mtx_);
    struct Job* job = (struct Job*) malloc(sizeof(struct Job));
    if (job == NULL)
    {
        pthread_mutex_unlock(&mtx_);
        return -1;
    }
    job->callback_func = callback_func;
    job->args = args;
    tasks_.push(job);
    pthread_mutex_unlock(&mtx_);
    // 通知线程池中的线程
    pthread_cond_broadcast(&cond_);

    return 0;
}

// 线程函数
void* Threads::thread_func(void* args)
{
    Threads* thread = (Threads*) args;
    pthread_t tid = pthread_self();
    while(1)
    {
        pthread_mutex_lock(&thread->mtx_);
        while (thread->tasks_.size() == 0 && !thread->stop_)
            pthread_cond_wait(&thread->cond_, &thread->mtx_);
        
        if (thread->stop_ && thread->tasks_.empty())
        {
            pthread_mutex_unlock(&thread->mtx_);
            pthread_exit(NULL);
        }

        // 取出一个任务并执行
        Job* job = thread->tasks_.front();
        thread->tasks_.pop();
        pthread_mutex_unlock(&thread->mtx_);
        // 回调函数的调用
        (*(job->callback_func))(job->args);
        free(job); 
        job = NULL;
    }

}