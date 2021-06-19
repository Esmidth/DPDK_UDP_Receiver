#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <thread>
#include <vector>

#define THREAD_NUMS 3

void handle_thread()
{
    int i = 0;
    while (true)
    {
        i++;
    }
}

int main(int argc, char *argv[])
{
    cpu_set_t cpuset;

    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_NUMS; i++)
    {
        threads.emplace_back(std::thread(handle_thread));
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        pthread_setaffinity_np(threads[i].native_handle(),sizeof(cpuset),&cpuset);
    }

    threads[0].join();

    // thread = pthread_self();

    /* Set affinity mask to include CPUs 0 to 7. */

    exit(EXIT_SUCCESS);
}