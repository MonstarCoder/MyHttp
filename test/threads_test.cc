#include "../threads.h"

#include <iostream>

void* thread_test(void* args)
{
    std::cout << "hello world!" << std::endl;
}

int main()
{
    Threads thread(10);
    Job job;
    job.callback_func = thread_test;
    job.args = NULL;
    thread.add_task(thread_test, NULL);

    return 0;
}