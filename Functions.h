#include "Data_Struct.h"


void *align_frame(void *arg)
{
    sleep(1);
    Thread_arg *sub = (Thread_arg *)arg;

    int init = false;
    while (!init)
    {

        if (sub->frame_queue.size() > 6)
        {
            for (int i = 0; i < 5; i++)
            {
                int id = sub->frame_queue.front();
                if (sub->local_UDPFrameIndex->pUDPFrame[id] != NULL)
                {
                    sub->frame_queue.pop();
                    sub->local_UDPFrameIndex->pUDPFrame[id]->count = 0;
                    sub->local_UDPFrameIndex->pUDPFrame[id] = NULL;
                }
            }
        }
        init = true;
    }

    while (1)
    {
        // printf("empty: %d\n",!sub->frame_queue.empty());
        if (!sub->frame_queue.empty())
        {
            // printf("exec\n");
            int id = sub->frame_queue.front();
            // printf("id:%d\n",id);
            if (sub->local_UDPFrameIndex->pUDPFrame[id] != NULL)
            {
                // printf("NOT NULL\n");
                // printf("CURRENT COUNT: %d\n",sub->local_UDPFrameIndex->pUDPFrame[id]->count);
                //     printf("current_pack_num: %d\n",sub->local_UDPFrameIndex->pUDPFrame[id]->count);
                if (sub->local_UDPFrameIndex->pUDPFrame[id]->count == PACK_NUM)
                {
                    sub->queue_to_send.push(id);
                    sub->frame_queue.pop();

                    // printf("%d: queue_to_send: %lu | frame_queue: %lu\n", sub->id, sub->queue_to_send.size(),sub->frame_queue.size());


                    // char *tmppp = (char *)malloc(sizeof(char)*1000);
                    // sprintf(tmppp, "%d: queue_to_send: %lu | frame_queue: %lu\n", sub->id, sub->queue_to_send.size(),sub->frame_queue.size());
                    // simple_log("thread_align.txt", tmppp);
                    // free(tmppp);
                }
            }
        }
        usleep(10);
    }
}
void *send_to_pulsar(void *arg)
{
    sleep(1);
    //    Send_arg *send_arg = (Send_arg *) arg;
    //    udpFramesIndex65536_1460 **global_index = send_arg->p_index;
    //    udpFramesPool_1460 **global_pool = send_arg->p_pool;
    //    Thread_arg **args = send_arg->args;

    Thread_arg *sub = (Thread_arg *)arg;

    unsigned int thread_id = sub->id;

    //    Client *clients[ACTIVE_THREAD];
    //    Producer producer[ACTIVE_THREAD];
    Producer producer;
    Client *client = new Client("pulsar://localhost:6650");
    //        clients[i] = new Client("pulsar://localhost:6650");
    std::string topic_name = "LD_000" + std::to_string(thread_id);
    Result res = client->createProducer(topic_name, producer);
    if (res != ResultOk)
    {
        printf("Create Producer Failed\n");
    }

    byte tmp[23][1460];
    int sent_frames = 0;

    while (1)
    {
        usleep(1);
        //    printf("queque length: %d\n",send_arg->queue_to_send.size());
        // printf("send_to_queue: %d\n", sub->queue_to_send.size());
        if (sub->queue_to_send.size() > 0)
        {
            int send_id = sub->queue_to_send.front();

            //                    byte tmp[23][1460];
            //                    memcpy(tmp, (*(args) + i)->pUDPFrame[ids[i]]->data, sizeof(byte) * 23 * 1460);
            memcpy(tmp, sub->local_UDPFrameIndex->pUDPFrame[send_id]->data,
                   sizeof(byte) * 23 * 1460);
            //                memcpy(tmp,sub->local_UDPFrameIndex)
            Message msg = MessageBuilder().setContent(tmp, sizeof(byte) * 23 * 1460).build();
            producer.sendAsync(msg, NULL);
            // producer.send(msg);
            sub->local_UDPFrameIndex->pUDPFrame[send_id]->count = 0;
            sub->local_UDPFrameIndex->pUDPFrame[send_id] = NULL;

            sent_frames++;

            printf("pulsar: ->%d: sent frame # %d | total sent: %d\n", sub->id, send_id, sent_frames);

            // char *tmppp = (char *)malloc(sizeof(": sent frame   ") + sizeof(int) * 2);
            // sprintf(tmppp, "%d: sent frame %d", sub->id, send_id);
            // simple_log("thread_pulsar.txt", tmppp);
            // free(tmppp);
            //                    continue;

            sub->queue_to_send.pop();
        }
        //        usleep(100);
    }
    for (int i = 0; i < ACTIVE_THREADS; i++)
    {
        client->close();
    }
    free(tmp);
}
// void *send_to_pulsar(void *arg)
// {
//     sleep(1);
//     Thread_arg *sub = (Thread_arg *)arg;
//     while (1)
//     {

//         printf("frame_queue Length: %d\n", sub->frame_queue.size());
//         printf("queue_to_send Length: %d\n", sub->queue_to_send.size());
//         usleep(10);
//     }
// }