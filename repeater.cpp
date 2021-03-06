#include <stdio.h>
#include <thread>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <bitset>
#include <iostream>
#include <atomic>

#include <unordered_map>
#include <vector>
#include <bitset>

#define APP_ID_RING_SIZE 128
#define DATA_RING_COUNT 128

typedef std::unordered_map<unsigned int, std::bitset<16>> int_bitset_map;
typedef std::unordered_map<unsigned int, std::vector<unsigned int>> int_vector_map;
typedef std::unordered_map<unsigned int, std::vector<rte_ring *>> int_vector_ring_map;
typedef std::unordered_map<unsigned int, rte_ring *> int_ring_map;

int print_map(int_vector_map &map)
{
    printf("--------\n");
    printf("int_vector_map\n");
    for (auto &n : map)
    {
        for (auto &t : n.second)
        {
            printf("key: %d\tValue:%d\n", n.first, t);
        }
    }
    printf("xxxxxxxxx\n");
}

int print_map(int_bitset_map &map)
{
    printf("--------\n");
    printf("int_bitset_map\n");
    for (auto &n : map)
    {
        std::cout << "key: " << n.first << "\tValue:" << n.second << std::endl;
    }
    printf("xxxxxxxxx\n");

}

int print_map(int_vector_ring_map& map)
{
    printf("--------\n");
    printf("int_vector_ring_map\n");
    for(auto& item:map)
    {
        printf("channel_id: %d, size: %d\n",item.first, item.second.size());
    }
    printf("xxxxxxxxx\n");

}

rte_ring *app_id_ring = nullptr;
rte_mempool *app_id_mempool = nullptr;

std::atomic<bool> init(false);

struct DHSM_MESSAGE
{
    uint8_t dhsm_id;
    uint8_t channel_id;
    bool flag; // 1 for adding, 0 for removing
};

struct DHSM_RES_MESSAGE
{
    std::string response;
};

struct UDP_MESSAGE
{
    std::bitset<16> data_mask;
};

class Route_Table
{
public:
    int_bitset_map dhsm_channel_map;
    int_vector_map channel_dhsm_map;
    int_ring_map dhsm_res_ring_map;            // aka communication ring which sending dhsm_res_message
    int_vector_ring_map channel_dhsm_ring_map; // aka route

    std::bitset<16> channel_sum;

    // RES_RING_<DHSM_ID>
    std::string res_ring_template = "RES_RING_";
    std::string data_ring_template = "RING_DATA_";

    rte_mempool* data_pool;

    Route_Table()
    {
        this->dhsm_channel_map.clear();
        this->channel_dhsm_map.clear();
        this->channel_sum = 0x0000;
        this->channel_dhsm_ring_map.clear();


        this->data_pool = rte_mempool_create("DATA_MEMPOOL",2048,65000,128,0,nullptr,nullptr,nullptr,nullptr,rte_socket_id(),0);
    }

    int send_return_msg(DHSM_MESSAGE *msg_ptr)
    {
        if (msg_ptr == nullptr)
        {
            return 0;
        }

        if (!(msg_ptr->channel_id >= 0 && msg_ptr->channel_id < 16))
        {
            return 0;
        }

        if (this->dhsm_res_ring_map.find(msg_ptr->dhsm_id) == this->dhsm_res_ring_map.end())
        {
            // not exist
            std::string tmp = this->res_ring_template + std::to_string(msg_ptr->channel_id);
            // std::cout << "tmp " << tmp << std::endl;

            this->dhsm_res_ring_map[msg_ptr->dhsm_id] = rte_ring_create(tmp.c_str(), APP_ID_RING_SIZE, rte_socket_id(), 0);
        }
        else
        {
            // exist
        }

        rte_ring_enqueue(this->dhsm_res_ring_map[msg_ptr->dhsm_id], NULL);
        return 1;
    }

    int generate_map1(DHSM_MESSAGE *msg_ptr)
    {
        //modify map1 with msg_ptr
        if (msg_ptr == nullptr)
        {
            return 0;
        }

        if (this->dhsm_channel_map.find(msg_ptr->dhsm_id) == this->dhsm_channel_map.end())
        {
            // if dhsm_id not in map
            // init

            if (msg_ptr->flag == 0)
            {
                // flag 0 for removing, while the dhsm does not exist, so skip it.
                return 1;
            }
            else
            {
                // flag 1 for adding, so init dhsm and add channel id
                if (msg_ptr->channel_id >= 0 && msg_ptr->channel_id < 16)
                {

                    this->dhsm_channel_map[msg_ptr->dhsm_id] = 0x0000;
                    this->dhsm_channel_map[msg_ptr->dhsm_id][msg_ptr->channel_id] = 1;
                    return 1;
                }
                else
                {
                    // channel_id overflow
                    return 0;
                }
            }
        }
        else
        {
            if (msg_ptr->flag == 0)
            {
                //removing
                if (msg_ptr->channel_id >= 0 && msg_ptr->channel_id < 16)
                {
                    this->dhsm_channel_map[msg_ptr->dhsm_id][msg_ptr->channel_id] = 0;
                }
                return 1;
            }
            else
            {
                if (msg_ptr->channel_id >= 0 && msg_ptr->channel_id < 16)
                {
                    this->dhsm_channel_map[msg_ptr->dhsm_id][msg_ptr->channel_id] = 1;
                }
                return 1;
                // adding
            }
            //modifty
        }
    }

    int generate_map2()
    {
        // generate channel_dhsm_map from dhsm_channel_map
        // int_bitset_map map1;
        // int_vector_map map2;

        // map1[1] = 0x0001;
        // map1[2] = 0x0002;
        // map1[3] = 0xff01;

        // std::bitset<16> bit_sum = 0x0000;

        //reset
        this->channel_dhsm_map.clear();
        this->channel_sum = 0x0000;

        for (auto &t : this->dhsm_channel_map)
        {
            this->channel_sum = this->channel_sum | t.second;
        }
        // print_map(map1);
        // std::cout << "bit_sum: " << bit_sum << std::endl;

        for (int i = 0; i < 16; i++)
        {
            if (this->channel_sum[i] == 1)
            {
                this->channel_dhsm_map[i] = {};
            }
        }

        for (auto &item : this->dhsm_channel_map)
        {
            for (int i = 0; i < 16; i++)
            {
                if (item.second[i] == 1)
                {
                    this->channel_dhsm_map[i].emplace_back(item.first);
                }
            }
        }

        // print_map(this->channel_dhsm_map);
        return 1;
    }

    int free_ring()
    {
        //free all the ring in channel_dhsm_ring_map;
        for (auto &item : this->channel_dhsm_ring_map)
        {
            for (auto sub_item : item.second)
            {
                rte_ring_free(sub_item);
            }
        }

        this->channel_dhsm_ring_map.clear();
        return 1;
    }

    int generate_ring()
    {
        //generate the new route
        for (auto &item : this->channel_dhsm_map)
        {
            // item.first == channel_id
            // item.second = std::vector< dhsm_id >
            printf("channel_id: %d\t", item.first);
            for (auto &sub_item : item.second)
            {
                printf("dhsm_id: %d,", sub_item);

                std::string tmp = this->data_ring_template + "C"+std::to_string(item.first) + "_" + "D"+std::to_string(sub_item);

                std::cout << tmp;

                this->channel_dhsm_ring_map[item.first].emplace_back(rte_ring_create(tmp.c_str(),DATA_RING_COUNT, rte_socket_id(),0));

                // RING_DATA_C<CHANNELID>_D<DHSMID>
                // RING_DATA_C1_D2

                // DHSM_MESSAGE{1,5}
                // RING_DATA_C5_D1

                // POOL_DATA_C<CHANNEL_ID>
                // POOL_DATA_C5
            }
            printf("\n");
        }
    }

    int handle_msg(DHSM_MESSAGE *msg_ptr)
    {
        // stop the repeater lcore

        int ret;
        // ret = send_return_msg(msg_ptr);
        ret = generate_map1(msg_ptr);
        generate_map2();
        free_ring();
        generate_ring();

        // send conf to lcore

        // start the repeater lcore


        return ret;
    }
};

void thread1()
{
    app_id_ring = rte_ring_create("app_id_ring", APP_ID_RING_SIZE, rte_socket_id(), 0);
    app_id_mempool = rte_mempool_create("app_id_mempool", APP_ID_RING_SIZE,
                                        sizeof(DHSM_MESSAGE), 8, 0,
                                        NULL, NULL, NULL, NULL, rte_socket_id(), 0);
    if (app_id_mempool == nullptr)
    {
        rte_panic("app_id_mempool init failed\n");
    }
    if (app_id_ring == nullptr)
    {
        rte_panic("app_id_ring init failed\n");
    }

    init = true;
    void *tmp;
    DHSM_MESSAGE *tmp1;

    Route_Table rt;
    int ret;

    while (true)
    {
        if (rte_ring_dequeue(app_id_ring, &tmp) < 0)
        {
            continue;
        }
        tmp1 = (DHSM_MESSAGE *)tmp;
        ret = rt.handle_msg(tmp1);
        printf("ret: %d\n", ret);
        print_map(rt.dhsm_channel_map);
        printf("----\n");
        // print_map(rt.channel_dhsm_map);
        // printf("----\n");
        print_map(rt.channel_dhsm_ring_map);

        // std::cout << "dhsm id: " << tmp1->dhsm_id << "\tchannel_id: " << tmp1->channel_id  << "\tflag: "<< tmp1->flag<< std::endl;
        printf("dhsm id:%d\tchannel_id:%d\tflag:%d\n", tmp1->dhsm_id, tmp1->channel_id, tmp1->flag);
        rte_mempool_put(app_id_mempool, tmp);
    }
}

void thread2()
{
    while (!init)
    {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    rte_ring *r1 = rte_ring_lookup("app_id_ring");
    rte_mempool *m1 = rte_mempool_lookup("app_id_mempool");
    if (r1 == nullptr)
    {

        rte_panic("app_id_ring lookup failed\n");
    }

    if (m1 == nullptr)
    {
        rte_panic("app_id_mempool lookup failed\n");
    }

    void *tmp;

    uint8_t x = 0;
    bool y = true;

    while (true)
    {
        DHSM_MESSAGE msg = {0, x, y};
        if (rte_mempool_get(m1, &tmp) < 0)
        {
            continue;
        }
        memcpy(tmp, &msg, sizeof(DHSM_MESSAGE));
        rte_ring_enqueue(r1, tmp);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        x++;
        if (x == 16)
        {
            x = 0;
            y = !y;
            // break;
        }
        // y = !y;
        // break;
    }
}

int main(int argc, char *argv[])
{
    int ret;
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
    {
        rte_panic("EAL init failed\n");
    }

    std::thread t1(thread1);
    std::thread t2(thread2);

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    pthread_setaffinity_np(t1.native_handle(), sizeof(cpu_set_t), &mask);

    CPU_ZERO(&mask);
    CPU_SET(2, &mask);
    pthread_setaffinity_np(t2.native_handle(), sizeof(cpu_set_t), &mask);

    t1.join();
}