#include "common.h"

#define PACK_NUM 44
#define DATA_LENGTH 1464
#define INDEX_SIZE 68000
#define MESSAGE_LENGTH 64040
#define P_BATCH_SEND_NUM 4

#define VM

#ifdef VM

#define POOL_SIZE 100
#define CACHE_SIZE 134217722
#define CACHE_NUM 70000

#endif

#ifndef VM

#define POOL_SIZE 50000
#define CACHE_SIZE 134217722
#define CACHE_NUM 70000


#endif
// constexpr CACHE_SIZE 70*1024*1024*1024

#define NUM_THREADS 1
#define ACTIVE_THREADS 1
#define NUM_CONSUMER_PER_PRODUCER 1

typedef unsigned char byte;

// class udpPacket_1460
// {
// public:
//     unsigned short frameSeq;
//     unsigned short packetSeq;
//     unsigned short packetLen;
//     unsigned short reserver;
//     unsigned char data[1460];
//     unsigned char checksum[4];
// };

// class udpPacket_1460{
//     public:
//     unsigned char data[1472];
// };

class udpPacket_1460
{
public:
    unsigned short nodeNum;   // 节点编号
    unsigned short frameSeq;  // 数据帧编号
    unsigned char packetNum;  // 分包总数
    unsigned char packetSeq;  // 分包编号
    unsigned short packetLen; // 数据包长度
    unsigned char data[1464]; // 分包数据
};

// class udpPacket_1460
// {
// public:
//     // unsigned short first;
//     // unsigned int frameSeq;
//     unsigned char data[1472];
// };

struct udpFrame_1460
{
    std::atomic<unsigned int> count;
    unsigned int frameSeq;
    unsigned char packetNum;
    unsigned char data[PACK_NUM][DATA_LENGTH];
    bool flags[PACK_NUM];
};

class list_node
{
public:
    list_node() = default;
    unsigned char data[1472];
    std::atomic<bool> is_empty;
    list_node *next_node;
};

struct udpFramesPool_1460
{
    unsigned int currIndex;
    // std::atomic<unsigned int> currIndex;
    struct udpFrame_1460 *pUDPPacket;

    udpFramesPool_1460()
    {
        this->currIndex = 0;
        // this->pUDPPacket = (struct udpFrame_1460 *)malloc(POOL_SIZE * sizeof(struct udpFrame_1460));
        this->pUDPPacket = (struct udpFrame_1460 *)rte_malloc(NULL, POOL_SIZE * sizeof(struct udpFrame_1460), 0);

        if (this->pUDPPacket == nullptr)
        {
            spdlog::error("FRAME POOL FAILED");
            return;
        }
        else
        {
            spdlog::info("FRAME POOL OK");
        }
    }
    ~udpFramesPool_1460()
    {
        free(this->pUDPPacket);
    }
};

struct udpFramesIndex65536_1460
{
    unsigned int currIndex;
    struct udpFrame_1460 **pUDPFrame;

    udpFramesIndex65536_1460()
    {
        this->currIndex = 0;
        // this->pUDPFrame = (struct udpFrame_1460 **)malloc(INDEX_SIZE * sizeof(struct udpFrame_1460 *));
        this->pUDPFrame = (struct udpFrame_1460 **)rte_malloc(NULL, INDEX_SIZE * sizeof(struct udpFrame_1460 *), 0);
    }

    udpFramesIndex65536_1460(int curr)
    {
        this->currIndex = curr;
        // this->pUDPFrame = (struct udpFrame_1460 **)malloc(INDEX_SIZE * sizeof(struct udpFrame_1460 *));
        this->pUDPFrame = (struct udpFrame_1460 **)rte_malloc(NULL, INDEX_SIZE * sizeof(struct udpFrame_1460 *), 0);
    }
    ~udpFramesIndex65536_1460()
    {
        free(this->pUDPFrame);
    }
};

struct Thread_arg
{
    unsigned int id;
    // struct sockaddr_in* sockaddr_input;
    udpFramesIndex65536_1460 *local_UDPFrameIndex;
    udpFramesPool_1460 *local_UDPFramePool;
    // std::queue<int> frame_queue;
    // std::queue<int> queue_to_send;
    moodycamel::ConcurrentQueue<int> mem_queue;
    moodycamel::ConcurrentQueue<int> frame_queue;
    moodycamel::ConcurrentQueue<int> queue_to_send;

    //FILE POINTER
    std::shared_ptr<spdlog::logger> async_log1;
    std::shared_ptr<spdlog::logger> async_log2;
    std::shared_ptr<spdlog::logger> async_log3;
    std::shared_ptr<spdlog::logger> store_log;

    void *mem_pool;
    unsigned short offset;
    std::string pulsar_topic_name;

    int channel_id;
    int proc_id;

    list_node *cur_producer;
    list_node *cur_consumer;

    // ThreadSafe_Queue<int> frame_queue;
    // ThreadSafe_Queue<int> queue_to_send;
};

struct Send_arg
{
    udpFramesIndex65536_1460 **p_index;
    udpFramesPool_1460 **p_pool;
    Thread_arg *args[NUM_THREADS];
};

struct Sync_arg
{
    byte *mem_buffer;
    int *send_flag;
    int send_loc;
};