#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/queue.h>

#include <thread>

#include <rte_config.h>
#include <rte_common.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_ring.h>
#include <rte_log.h>
#include <rte_mempool.h>
#include <atomic>

// #include <cmdline_rdline.h>
// #include <cmdline_parse.h>
// #include <cmdline_parse_string.h>
// #include <cmdline_socket.h>
// #include <cmdline.h>

struct rte_ring *send_ring, *recv_ring;
struct rte_mempool *send_pool, *recv_pool;
char *PRI_2_SEC = "PRI_2_SEC";
static const char *_MSG_POOL = "MSG_POOL";
#define STR_TOKEN_SIZE 128
const unsigned flags = 0;
const unsigned ring_size = 1048576;
const unsigned pool_size = 1048576;
const unsigned pool_cache = 320;
const unsigned priv_data_sz = 0;
std::atomic<bool> start(false);
std::atomic<bool> run(true);
std::atomic<bool> ring_create(false);

#define BULK_SIZE 8

void secondary_thread()
{

    send_ring = rte_ring_create(PRI_2_SEC, ring_size, rte_socket_id(), 0);
    // send_pool = rte_mempool_create(_MSG_POOL, pool_size, STR_TOKEN_SIZE, pool_cache, priv_data_sz, NULL, NULL, NULL, NULL,rte_socket_id(),flags);
    send_pool = rte_mempool_create(_MSG_POOL, pool_size,
                                   STR_TOKEN_SIZE, pool_cache, priv_data_sz,
                                   NULL, NULL, NULL, NULL,
                                   rte_socket_id(), flags);
    // start = true;
    std::string send_template = "hello";
    unsigned long i = 0;
    // rte_ring_enqueue(send_ring, &i);

    while (!start)
    {
    }

    while (run)
    {
        void *msg = NULL;
        if (rte_mempool_get(send_pool, &msg) < 0)
        {
            rte_panic("Fail to get message buffer\n");
        }

        std::string tmp = send_template + std::to_string(i);
        strlcpy((char *)msg, (tmp.c_str()), STR_TOKEN_SIZE);
        // strlcpy((char*)msg,i,STR_TOKEN_SIZE);
        // std::string tmp = send_template + std::to_string(i);
        // char *tmpp = send_template;
        if (rte_ring_enqueue(send_ring, msg) < 0)
        {
            rte_panic("Fail to send message, message discard\n");
            // rte_mempool_put(send_pool, msg);
        }
        else
        {
            // printf("send:%d\n", i);
        }
        i++;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    printf("run: %d\n", i);
}
void secondary_thread_bulk()
{

    send_ring = rte_ring_create(PRI_2_SEC, ring_size, rte_socket_id(), 0);
    // send_pool = rte_mempool_create(_MSG_POOL, pool_size, STR_TOKEN_SIZE, pool_cache, priv_data_sz, NULL, NULL, NULL, NULL,rte_socket_id(),flags);
    send_pool = rte_mempool_create(_MSG_POOL, pool_size,
                                   STR_TOKEN_SIZE, pool_cache, priv_data_sz,
                                   NULL, NULL, NULL, NULL,
                                   rte_socket_id(), flags);
    // start = true;
    std::string send_template = "hello";
    unsigned long i = 0;
    // rte_ring_enqueue(send_ring, &i);

    while (!start)
    {
    }
    std::string tmp;

    while (run)
    {
        void *msg[BULK_SIZE];

        // printf("1\n");
        if (rte_mempool_get_bulk(send_pool, (void **)msg, BULK_SIZE) < 0)
        {
            rte_panic("Fail to get message buffer\n");
        }
        // printf("1.5\n");
        for (int i = 0; i < BULK_SIZE; i++)
        {
            tmp = send_template + std::to_string(i);
            strlcpy((char *)msg + i, (tmp.c_str()), STR_TOKEN_SIZE);
        }
        // printf("2\n");

        // strlcpy((char*)msg,i,STR_TOKEN_SIZE);
        // std::string tmp = send_template + std::to_string(i);
        // char *tmpp = send_template;
        if (rte_ring_enqueue_bulk(send_ring, msg, BULK_SIZE, NULL) < 0)
        {
            rte_panic("Fail to send message, message discard\n");
        }
        // printf("3\n");

        i += BULK_SIZE;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    printf("run: %d\n", i);
}

void third_thread_bulk()
{
    // std::this_thread::yield();
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    while (!start)
    {
    }
    recv_ring = rte_ring_lookup(PRI_2_SEC);
    recv_pool = rte_mempool_lookup(_MSG_POOL);
    if (recv_ring == nullptr)
    {
        printf("NULL RING\n");
        return;
    }
    if (recv_pool == nullptr)
    {
        printf("NULL RECV_POOL\n");
        return;
    }

    printf("get ring\n");
    void *tmp[BULK_SIZE];
    while (true)
    {
        if (rte_ring_dequeue_bulk(recv_ring, tmp, BULK_SIZE, NULL) < 0)
        {
            // printf("DEQUEUE FAILED\n");
            continue;
        }

        // int recv = *(int*)tmp;
        // printf("recv:%s\n-----\n", (char *)tmp);
        rte_mempool_put_bulk(send_pool,tmp,BULK_SIZE);

        // rte_mempool_put(send_pool, tmp);

        // else
        // {
        //     printf("dequeue success\n");
        // }
    }
}

void third_thread()
{
    // std::this_thread::yield();
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    while (!start)
    {
    }
    recv_ring = rte_ring_lookup(PRI_2_SEC);
    recv_pool = rte_mempool_lookup(_MSG_POOL);
    if (recv_ring == nullptr)
    {
        printf("NULL RING\n");
        return;
    }
    if (recv_pool == nullptr)
    {
        printf("NULL RECV_POOL\n");
        return;
    }

    printf("get ring\n");
    while (true)
    {
        void *tmp;

        if (rte_ring_dequeue(recv_ring, &tmp) < 0)
        {
            // printf("DEQUEUE FAILED\n");
            continue;
        }
        // int recv = *(int*)tmp;
        // printf("recv:%s\n-----\n", (char *)tmp);

        rte_mempool_put(send_pool, tmp);

        // else
        // {
        //     printf("dequeue success\n");
        // }
    }
}

void send_thread()
{
    send_ring = rte_ring_create(PRI_2_SEC, ring_size, rte_socket_id(), 0);
    while (!start)
    {
        std::this_thread::yield();
    }
    int x = 0;
    while (true)
    {
        if (rte_ring_enqueue(send_ring, &x) < 0)
        {
            rte_panic("SEND FAILURE");
        }
        printf("SEND:%d\n", x);
        x++;
        std::this_thread::yield();
    }
}

void recv_thread()
{
    while (!ring_create)
    {
        std::this_thread::yield();
    }
    recv_ring = rte_ring_lookup(PRI_2_SEC);

    void *y;

    while (true)
    {
        if (rte_ring_dequeue(recv_ring, &y) < 0)
        {
            continue;
        }
        printf("recv:%d\n-----\n", *(int *)y);
    }
}

int main(int argc, char **argv)
{
    int ret;
    ret = rte_eal_init(argc, argv);
    std::thread t1(secondary_thread_bulk);
    std::thread t2(third_thread_bulk);
    // start = true;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    pthread_setaffinity_np(t1.native_handle(), sizeof(cpu_set_t), &mask);

    CPU_ZERO(&mask);
    CPU_SET(2, &mask);
    pthread_setaffinity_np(t2.native_handle(), sizeof(cpu_set_t), &mask);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    start = true;
    run = true;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    run = false;

    if (ret < 0)
    {
        rte_exit(EXIT_FAILURE, "Cannot init EAL\n");
    }

    if (rte_eal_process_type() == RTE_PROC_PRIMARY)
    {
        printf("PRIMARY\n");
    }
    else
    {
        printf("SECONDARY\n");
    }

    t1.join();

    return 0;
}