/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include "Data_Struct.h"

//

std::atomic<double> sec(0);

#ifdef RING
// struct rte_mempool *send_pool;
// struct rte_ring *send_ring, *recv_ring;
// std::string PRI_2_SEC;
// std::string _MSG_POOL;

#ifndef VM
const unsigned flags = 0;
const unsigned ring_size = 1048576 * 2;
const unsigned pool_size = 1048576 * 2;
const unsigned pool_cache = 320;
const unsigned priv_data_sz = 0;
#endif

#ifdef VM
const unsigned flags = 0;
const unsigned ring_size = 1024;
const unsigned pool_size = 1024;
const unsigned pool_cache = 320;
const unsigned priv_data_sz = 0;
#endif

#define STR_TOKEN_SIZE 1472
#endif

#define TIME_STAMP 1 // sec

double last_sec = 0;
int first_time = true;

std::string log_dir = "logs1/";

static uint32_t l2fwd_enabled_port_mask = 0;
static uint16_t nb_port_pair_params;

struct port_pair_params
{
#define NUM_PORTS 2
	uint16_t port[NUM_PORTS];
} __rte_cache_aligned;

static struct port_pair_params *port_pair_params;
uint16_t nb_rxd = RX_RING_SIZE;
uint16_t nb_txd = TX_RING_SIZE;

static const struct rte_eth_conf port_conf_default = {
	.link_speeds = 0,
	.rxmode = {
		.mq_mode = ETH_MQ_RX_NONE,
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,
	},
};

static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = 1;

	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;

	retval = rte_eth_dev_info_get(port, &dev_info);
	if (retval != 0)
	{
		printf("Error during getting device (port %u) info: %s\n",
			   port, strerror(-retval));
		return retval;
	}

	if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		port_conf.txmode.offloads |=
			DEV_TX_OFFLOAD_MBUF_FAST_FREE;

	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++)
	{
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
										rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++)
	{
		retval = rte_eth_tx_queue_setup(port, q, nb_txd,
										rte_eth_dev_socket_id(port), &txconf);
		if (retval < 0)
			return retval;
	}

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	/* Display the port MAC address. */
	struct rte_ether_addr addr;
	retval = rte_eth_macaddr_get(port, &addr);
	if (retval != 0)
		return retval;

	printf("HELLO WORLD \n");
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
		   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
		   port,
		   addr.addr_bytes[0], addr.addr_bytes[1],
		   addr.addr_bytes[2], addr.addr_bytes[3],
		   addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	// retval = rte_eth_promiscuous_enable(port);
	// if (retval != 0)
	// return retval;

	return 0;
}

void print_pkt(struct rte_mbuf *buf)
{
	for (int j = 0; j < buf->data_len; j++)
	{
		if (j % 8 == 0 && j != 0)
		{

			if (j % 16 == 0 && j != 0)
			{
				printf("\n");
			}
			else
			{
				printf("\t|\t");
			}
		}
		printf("%x ", *(uint8_t *)((uint8_t *)buf->buf_addr + buf->data_off + 42 + j));
	}
	printf("\n");
	// ip = (struct iphdr *)(bufs[i]->buf_addr + bufs[i]->data_off + 14);
	// printf("tos:%hu, tot_len:%hu, id:%hu, frag_off:%hu, ttl:%hu, protoco:%hu, check:%hu, saddr:%d, daddr:%d\n", ip->tos, ip->tot_len, ip->id, ip->frag_off, ip->ttl, ip->protocol, ip->check, ip->saddr, ip->daddr);

	// udp_header = (struct udphdr *)(bufs[i]->buf_addr + bufs[i]->data_off + 34);
	// printf("udp:\nsourec: %hu, dest:%hu, len:%hu, check:%hu\n", udp_header->source, udp_header->dest, udp_header->len, udp_header->check);
	// printf("udp_len_hex:%x %x\n", *(uint8_t *)(bufs[i]->buf_addr + bufs[i]->data_off + 38), *(uint8_t *)(bufs[i]->buf_addr + bufs[i]->data_off + 39));
	uint16_t tmp_len;
	tmp_len = (*(uint8_t *)((uint8_t *)buf->buf_addr + buf->data_off + 38) << 8) | (*(uint8_t *)((uint8_t *)buf->buf_addr + buf->data_off + 39));
	// printf("udp_len:%d\n", *(uint16_t *)(bufs[i]->buf_addr + bufs[i]->data_off + 38));
	printf("udp_len_new:%d\n", tmp_len);
	printf("------------------\n");
}

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */
//NOZOMI
static __rte_noreturn int
lcore_main(void *arg)
{
	uint16_t port;

	// int pp = *(int *)arg;
	Thread_arg *sub = (Thread_arg *)arg;

	/*
	 * Check that the port is on the same NUMA node as the polling thread
	 * for best performance.
	 */
	RTE_ETH_FOREACH_DEV(port)
	if (rte_eth_dev_socket_id(port) > 0 &&
		rte_eth_dev_socket_id(port) !=
			(int)rte_socket_id())
		printf("WARNING, port %u is on remote NUMA node to "
			   "polling thread.\n\tPerformance will "
			   "not be optimal.\n",
			   port);

	printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
		   rte_lcore_id());

	/* Run until the application is quit or killed. */
	uint16_t count[2] = {0, 0};
	udpPacket_1460 *tmp_packet_ptr;
	void *msg = NULL;
	for (;;)
	{
		/*
		 * Receive packets on a port and forward them on the paired
		 * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
		 */
		// RTE_ETH_FOREACH_DEV(port)
		// {
		port = sub->proc_id;

		/* Get burst of RX packets, from first port of pair. */
		struct rte_mbuf *bufs[BURST_SIZE];
		const uint16_t nb_rx = rte_eth_rx_burst(port, 0,
												bufs, BURST_SIZE);

		if (unlikely(nb_rx == 0))
			continue;
		// printf("nb_rx:%d\n", nb_rx);
		// for(int i = 0;i<nb_rx;i++)
		// {
		// 	printf("Preamble :%x %x %x %x %x %x %x\nSFD:%x\n",bufs[i][0],bufs[i][1],bufs[i][2],bufs[i][3],bufs[i][4],bufs[i][5],bufs[i][6],bufs[i][7]);
		// }
		// struct ip* ip_packet;

		//TODO: switch to bulk

		for (int i = 0; i < nb_rx; i++)
		{
			// if (bufs[i]->pkt_len == 1514)
			// if(true)
			// {
			if (rte_mempool_get(sub->send_pool, &msg) < 0)
			{
				rte_panic("Fail to get message buffer\n");
			}
			// printf("pkt_len:%hu data_len:%d buf_len:%d data_off:%d\n", bufs[i]->pkt_len, bufs[i]->data_len, bufs[i]->buf_len, bufs[i]->data_off);
			//printf("%x %x %x %x %x %x %x\n", *(char*)(bufs[i]->buf_addr + bufs[i]->data_off), 0, 0, 0, 0, 0, 0);
			tmp_packet_ptr = (udpPacket_1460 *)((char *)bufs[i]->buf_addr + bufs[i]->data_off + 42);
			// for (int i = 0; i < 1472; i++)
			// {
			// 	printf("%x,", *((unsigned char *)bufs[i]->buf_addr + bufs[i]->data_off + 42 + i));
			// }
			// printf("\n");
			rte_memcpy(msg, tmp_packet_ptr, 1472);
			if (rte_ring_enqueue(sub->ring1_2, msg) < 0)
			{
				rte_panic("Fail to send message, message discard\n");
			}
			// printf("frameSeq:%d, packetSeq:%d, packetLen:%d\n", tmp_packet_ptr->frameSeq, tmp_packet_ptr->packetSeq, tmp_packet_ptr->packetLen);
			// print_pkt(bufs[i]);
			// count[port] += 1;
			// }
		}
		// printf("count:%d port_id:%d\n----------\n", count[port], port);

		/* Send burst of TX packets, to second port of pair. */
		// const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, 0,
		// 		bufs, nb_rx);
		const uint16_t nb_tx = 0;

		//TODO: remove free packets here and add tao DOB thread
		/* Free any unsent packets. */
		if (unlikely(nb_tx < nb_rx))
		{
			uint16_t buf;
			for (buf = nb_tx; buf < nb_rx; buf++)
				rte_pktmbuf_free(bufs[buf]);
		}
		// }
	}
}

static int check_port_pair_config(void)
{
	uint32_t port_pair_config_mask = 0;
	uint32_t port_pair_mask = 0;
	uint16_t index, i, portid;

	for (index = 0; index < nb_port_pair_params; index++)
	{
		port_pair_mask = 0;

		for (i = 0; i < NUM_PORTS; i++)
		{
			portid = port_pair_params[index].port[i];
			if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
			{
				printf("port %u is not enabled in port mask\n",
					   portid);
				return -1;
			}
			if (!rte_eth_dev_is_valid_port(portid))
			{
				printf("port %u is not present on the board\n",
					   portid);
				return -1;
			}

			port_pair_mask |= 1 << portid;
		}

		if (port_pair_config_mask & port_pair_mask)
		{
			printf("port %u is used in other port pairs\n", portid);
			return -1;
		}
		port_pair_config_mask |= port_pair_mask;
	}

	l2fwd_enabled_port_mask &= port_pair_config_mask;

	return 0;
}

//DOB
void consumer_thread(Thread_arg *sub)
{
	udpFramesIndex65536_1460 *pUDPFrameIndex;
	udpFramesPool_1460 *pUDPFramePool;
	// int offset;
	unsigned char tmp[1472];
	udpPacket_1460 *packet_ptr = nullptr;
	pUDPFrameIndex = sub->local_UDPFrameIndex;
	pUDPFramePool = sub->local_UDPFramePool;
	// Thread_arg *sub = (Thread_arg *)arg;

	unsigned short last_frameSeq = 0;

	unsigned short frameSeq = 0;
	unsigned short packetSeq = 0;

#ifndef QUEUE

#ifdef RING

	// sub->recv_ring = rte_ring_lookup(sub->PRI_2_SEC.c_str());

	while (!sub->assem_init)
	{
		void *tmpp;
		if (rte_ring_dequeue(sub->ring1_2, &tmpp) < 0)
		{
			continue;
		}
		rte_mempool_put(sub->send_pool, tmpp);
	}
	spdlog::info("BEFORE INIT");
	while (true)
	{
		void *tmpp;
		if (rte_ring_dequeue(sub->ring1_2, &tmpp) < 0)
		{
			continue;
		}
		packet_ptr = (udpPacket_1460 *)tmpp;
		printf("%d: %x, %x, %x, %x, %x, %x, %x, %x\n", packet_ptr->packetSeq, *(packet_ptr->data), *(packet_ptr->data + 1), *(packet_ptr->data + 2), *(packet_ptr->data + 3), *(packet_ptr->data + 4), *(packet_ptr->data + 5), *(packet_ptr->data + 6), *(packet_ptr->data + 7));

		if (packet_ptr->packetSeq == PACK_NUM - 1)
		{
			rte_mempool_put(sub->send_pool, tmpp);
			break;
		}

		rte_mempool_put(sub->send_pool, tmpp);
	}
	spdlog::warn("INIT DOB");

#endif

#ifndef RING
	while (sub->cur_consumer->is_empty == true)
	{
		std::this_thread::yield();
	}

	if (sub->cur_consumer->is_empty == false)
	{
		packet_ptr = (udpPacket_1460 *)sub->cur_consumer->data;
		frameSeq = *(unsigned short *)(packet_ptr->data + 2);

		// frameSeq = packet_ptr->frameSeq;
		// packetSeq = packet_ptr->packetSeq;
		arg->cur_consumer->is_empty = true;
		arg->cur_consumer = arg->cur_consumer->next_node;
	}
#endif
#endif

#ifdef QUEUE
	while (!assem_init)
	{
		arg->mem_queue.try_dequeue(offset);
	}

	spdlog::info("BEFORE INIT");
	while (1)
	{
		if (arg->mem_queue.try_dequeue(offset))
		{
			memcpy(tmp, (arg->mem_pool + offset * 1472), 1472);
			packet_ptr = (udpPacket_1460 *)tmp;
			// spdlog::info("nodeNum:{},frameSeq:{}, packetNum:{}, packetSeq:{}, packetLen:{}",packet_ptr->nodeNum,packet_ptr->frameSeq,packet_ptr->packetNum,packet_ptr->packetSeq,packet_ptr->packetLen);

			if (packet_ptr->packetSeq == 43)
			{
				break;
			}
		}
	}
	spdlog::warn("INIT DOB");
#endif
	while (1)
	{
#ifdef QUEUE
		// QUEUE VERSION
		if (arg->mem_queue.try_dequeue(offset))
#endif

#ifndef QUEUE
#ifndef RING
			if (sub->cur_consumer->is_empty == false)
#endif

#ifdef RING

				if (true)

#endif

#endif
				{
#ifdef QUEUE
					//QUEUE VERSION
					memcpy(tmp, (arg->mem_pool + offset * 1472), 1472);

					// memcpy(tmp, (arg->mem_pool + offset), 1472);
					packet_ptr = (udpPacket_1460 *)tmp;
#endif

#ifndef QUEUE

#ifdef RING
					void *tmpp;
					if (rte_ring_dequeue(sub->ring1_2, &tmpp) < 0)
					{

						// rte_panic("Fail to dequeue\n");
						std::this_thread::yield();
						continue;
					}
					//TODO: remove rte_mempool_put and switch to rte_pkt_free
					memcpy(tmp, tmpp, 1472);
					rte_mempool_put(sub->send_pool, tmpp);
					packet_ptr = (udpPacket_1460 *)tmp;
#endif

#ifndef RING
					packet_ptr = (udpPacket_1460 *)sub->cur_consumer->data;
#endif

#endif
					// spdlog::info("P1");

					/*
                
                last_frameSeq++;
                frameSeq = *(unsigned short *)(packet_ptr->data + 2);
                arg->async_log1->info("frameSeq:{}",frameSeq);
                if (last_frameSeq != frameSeq)
                {
                    mis++;
                    arg->async_log1->error("frameSeq:{}", frameSeq);
                    // printf("%d\n",frameSeq);
                }

                last_frameSeq = frameSeq;
                */

					// spdlog::info("frame:{}, nodeNum:{}, packetLen:{}, packetNum:{}, packetSet:{} ",packet_ptr->frameSeq,packet_ptr->nodeNum, packet_ptr->packetLen,packet_ptr->packetNum, packet_ptr->packetSeq);

					// spdlog::info("packetNum:{}",packet_ptr->packetNum);

					/*
                packetSeq++;
                printf("%d\n",(unsigned char)packet_ptr->packetNum);
                if (packetSeq == packet_ptr->packetNum)
                {
                    frameSeq++;
                    packetSeq = 0;
                }
                if (packetSeq != packet_ptr->packetSeq || frameSeq != packet_ptr->frameSeq)
                {
                    mis++;
                    packetSeq = packet_ptr->packetSeq;
                    frameSeq = packet_ptr->frameSeq;
                }
                */
					// spdlog::info("P2");
					// printf("frameSeq: %d, packetSeq: %d\n", packet->frameSeq, packet->packetSeq);
					//            async_file->info("frameSeq:{}", packet_ptr->frameSeq);
					// unsigned int frameSeq = *(unsigned int*)(packet_ptr->data+2);
					// async_file->info("frameSeq:{}",frameSeq);
					// async_file->info("frameSeq:{},packetSeq:{}", packet_ptr->frameSeq, packet_ptr->packetSeq);
					sub->global_count++;
					//            sub->local_UDPFrameIndex->pUDPFrame[send_id] = nullptr;
					frameSeq = packet_ptr->frameSeq;

					if (packetSeq != packet_ptr->packetSeq)
					{
#ifdef LOG
						arg->async_log1->error("packetSeq:{},{}", packetSeq, packet_ptr->packetSeq);
#endif
						sub->mis++;
					}
					else
					{
#ifdef LOG
						arg->async_log1->info("frameSeq:{}, packetSeq:{},{}, packetNum:{}", frameSeq, packet_ptr->packetSeq, packetSeq, packet_ptr->packetNum);
#endif
					}
					packetSeq = (packet_ptr->packetSeq + 1) % PACK_NUM;

					// if(packet_ptr->packetSeq == 43)
					// {
					//     printf("%x, %x, %x, %x\n",packet_ptr->data[1084],packet_ptr->data[1085],packet_ptr->data[1086],packet_ptr->data[1087]);
					// }
					// spdlog::info("nodeNum:{},frameSeq:{}, packetNum:{}, packetSeq:{}, packetLen:{}",packet_ptr->nodeNum,packet_ptr->frameSeq,packet_ptr->packetNum,packet_ptr->packetSeq,packet_ptr->packetLen);

					// printf("%d: %x, %x, %x, %x, %x, %x, %x, %x\n", packet_ptr->packetSeq, *(packet_ptr->data), *(packet_ptr->data + 1), *(packet_ptr->data + 2), *(packet_ptr->data + 3), *(packet_ptr->data + 4), *(packet_ptr->data + 5), *(packet_ptr->data + 6), *(packet_ptr->data + 7));

					// packetSeq++;
					// spdlog::info("packetSeq:{},packet_ptr:{}",packetSeq,packet_ptr->packetSeq);
					// if(packetSeq != packet_ptr->packetSeq)
					// {
					//     mis++;
					//     spdlog::error("mis:{}",mis);
					//     packetSeq = (packet_ptr->packetSeq +1)% 43;

					// }
#ifdef GODOT
					if (nullptr == pUDPFrameIndex->pUDPFrame[frameSeq])
					{
						// spdlog::info("pass 1");
						if (pUDPFramePool->pUDPPacket[pUDPFramePool->currIndex].count == 0)
						{
							pUDPFrameIndex->pUDPFrame[frameSeq] = &(pUDPFramePool->pUDPPacket[pUDPFramePool->currIndex]);
							// memset(pUDPFrameIndex->pUDPFrame[frameSeq],0,1464*44);
							pUDPFramePool->pUDPPacket[pUDPFramePool->currIndex].count++;

							memcpy(pUDPFrameIndex->pUDPFrame[frameSeq]->data[packet_ptr->packetSeq], packet_ptr->data, packet_ptr->packetLen);
// spdlog::info("MOD1");
#ifdef DROP
							pUDPFrameIndex->pUDPFrame[frameSeq]->flags[packet_ptr->packetSeq] = true;
#endif
							// spdlog::info("MOD2");
						}
						else
						{
							int i = 0;
							for (i = 0; i < POOL_SIZE; i++)
							{

								if (pUDPFramePool->pUDPPacket[(pUDPFramePool->currIndex + i) % POOL_SIZE].count == 0)
								{
									// spdlog::info("INTO 2");
									pUDPFramePool->currIndex = (pUDPFramePool->currIndex + i) % POOL_SIZE;

									pUDPFrameIndex->pUDPFrame[frameSeq] = &(pUDPFramePool->pUDPPacket[pUDPFramePool->currIndex]);
									pUDPFramePool->pUDPPacket[pUDPFramePool->currIndex].count++;

									// memset(pUDPFrameIndex->pUDPFrame[frameSeq],0,1464*44);
									memcpy(pUDPFramePool->pUDPPacket[pUDPFramePool->currIndex].data[packet_ptr->packetSeq], packet_ptr->data, packet_ptr->packetLen);
#ifdef DROP
									pUDPFrameIndex->pUDPFrame[frameSeq]->flags[packet_ptr->packetSeq] = true;
#endif
									break;
								}
							}
						}
						pUDPFrameIndex->pUDPFrame[frameSeq]->packetNum = PACK_NUM;
						pUDPFrameIndex->pUDPFrame[frameSeq]->frameSeq = frameSeq;
						// pUDPFrameIndex->pUDPFrame[frameSeq]->packetNum = packet_ptr->packetNum;
						// sub->frame_queue.enqueue(frameSeq);

						//TODO: switch rte_ring from lockless queue
						rte_ring_enqueue(sub->ring2_3, pUDPFrameIndex->pUDPFrame[frameSeq]);
					}
					else
					{
						// spdlog::info("P3");
						pUDPFrameIndex->pUDPFrame[frameSeq]->count += 1;
						memcpy(pUDPFrameIndex->pUDPFrame[frameSeq]->data[packet_ptr->packetSeq], packet_ptr->data, packet_ptr->packetLen);
#ifdef DROP
						pUDPFrameIndex->pUDPFrame[frameSeq]->flags[packet_ptr->packetSeq] = true;
#endif
					}

#ifndef QUEUE

#ifdef RING
					// rte_mempool_put(send_pool,tmpp);
#endif

#ifndef RING

					arg->cur_consumer->is_empty = true;
					arg->cur_consumer = arg->cur_consumer->next_node;
#endif
#endif
#endif
				}
				else
				{
					std::this_thread::yield();
				}
	}
}

//GODOT
void align_thread(Thread_arg *sub)
{
	// sleep(1);
	// Thread_arg *sub = (Thread_arg *)arg;

	int id;
	// while (!align_init)
	// {
	//     std::this_thread::yield();
	// }

	bool drop = false;
	uint64_t prev_tsc = 0, cur_tsc, diff_tsc;

	spdlog::warn("ALIGN_INIT");
	void *tmpp;
	udpFrame_1460 *udpFrame_ptr = nullptr;
	while (1)
	{
		// printf("empty: %d\n",!sub->frame_queue.empty());

		if (rte_ring_dequeue(sub->ring2_3, &tmpp) < 0)
		{
			continue;
		}
		udpFrame_ptr = (udpFrame_1460 *)tmpp;

		if (udpFrame_ptr != nullptr)
		{
			cur_tsc = rte_rdtsc();
			prev_tsc = cur_tsc;
			while (udpFrame_ptr->count != udpFrame_ptr->packetNum)
			{
#ifdef DROP
				cur_tsc = rte_rdtsc();
				diff_tsc = cur_tsc - prev_tsc;
				if (diff_tsc > TIMER_RESOLUTION_CYCLES * 10)
				{
					sub->drop_count++;
					drop = true;
					break;
				}
				// sub->local_UDPFrameIndex->pUDPFrame[id]->count = 0;
				// sub->local_UDPFrameIndex->pUDPFrame[id] = nullptr;
				// break;
				// if (sec - last_sec > 0.01)
				// {

				// }
#endif
#ifdef LOG
				sub->async_log2->info("queue head:{}, count:{}", id, sub->local_UDPFrameIndex->pUDPFrame[id]->count);
#endif
				std::this_thread::yield();
			}

			// 		if (sub->frame_queue.try_dequeue(id))
			// 		{
			// 			// id = sub->frame_queue.front();
			// 			// async_file1->warn("try_dequeue:{}", id);

			// 			if (sub->local_UDPFrameIndex->pUDPFrame[id] != NULL)
			// 			{
			// 				cur_tsc = rte_rdtsc();
			// 				prev_tsc = cur_tsc;
			// 				// printf("NOT NULL\n");
			// 				// printf("CURRENT COUNT: %d\n",sub->local_UDPFrameIndex->pUDPFrame[id]->count);
			// 				//     printf("current_pack_num: %d\n",sub->local_UDPFrameIndex->pUDPFrame[id]->count);
			// 				while (sub->local_UDPFrameIndex->pUDPFrame[id]->count != sub->local_UDPFrameIndex->pUDPFrame[id]->packetNum)
			// 				{
			// #ifdef DROP
			// 					cur_tsc = rte_rdtsc();
			// 					diff_tsc = cur_tsc - prev_tsc;
			// 					if (diff_tsc > TIMER_RESOLUTION_CYCLES * 10)
			// 					{
			// 						sub->drop_count++;
			// 						drop = true;
			// 						break;
			// 					}
			// 					// sub->local_UDPFrameIndex->pUDPFrame[id]->count = 0;
			// 					// sub->local_UDPFrameIndex->pUDPFrame[id] = nullptr;
			// 					// break;
			// 					// if (sec - last_sec > 0.01)
			// 					// {

			// 					// }
			// #endif
			// // if (sub->frame_queue.try_pop(id))
			// // {
			// // sub->queue_to_send.push(id);
			// // }
			// // printf("%d: queue_to_send: %lu | frame_queue: %lu\n", sub->id, sub->queue_to_send.size(),sub->frame_queue.size());

			// // char *tmppp = (char *)malloc(sizeof(char)*1000);
			// // sprintf(tmppp, "%d: queue_to_send: %lu | frame_queue: %lu\n", sub->id, sub->queue_to_send.size(),sub->frame_queue.size());
			// // simple_log("thread_align.txt", tmppp);
			// // free(tmppp);
			// // spdlog::info("head count: {}",sub->local_UDPFrameIndex->pUDPFrame[id]->count);
			// #ifdef LOG
			// 					sub->async_log2->info("queue head:{}, count:{}", id, sub->local_UDPFrameIndex->pUDPFrame[id]->count);
			// #endif
			// 					std::this_thread::yield();
			// 				}

#ifdef DROP

			if (drop)
			{
				for (int i = 0; i < 44; i++)
				{
					if (sub->local_UDPFrameIndex->pUDPFrame[id]->flags[i] == false)
					{
						memset(sub->local_UDPFrameIndex->pUDPFrame[id]->data[i], 0, DATA_LENGTH);
					}
				}
				drop = false;
			}
#endif

			sub->align_num++;
			// sub->queue_to_send.enqueue(id);
			rte_ring_enqueue(sub->ring3_4, udpFrame_ptr);

			// async_file1->warn("queue_to_send| enqueue: {}", id);
			// std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	// else
	// {
	// 	std::this_thread::yield();
	// }
	//}
}

void send_to_pulsar(void *arg)
{
	// sleep(1);

#ifdef FAKE_DATA
	spdlog::info("MALLOC MESSAGE CACHE");
	char *data1 = (char *)rte_malloc(NULL, (MESSAGE_LENGTH)*60 * sizeof(unsigned char), 0);
	// unsigned char data1[44][1464];
	std::ifstream inFile("JD02ZY01_data.dat", std::ios::in | std::ios::binary);
	if (!inFile)
	{
		rte_panic("Could not find data file\n");
	}
	spdlog::info("BEFORE READ");
	if (!inFile.read(data1, MESSAGE_LENGTH * 42))
	{
		rte_panic("read error\n");
	}
	inFile.close();
	spdlog::info("DONE INIT READ FAKE");

#endif
	//    Send_arg *send_arg = (Send_arg *) arg;
	//    udpFramesIndex65536_1460 **global_index = send_arg->p_index;
	//    udpFramesPool_1460 **global_pool = send_arg->p_pool;
	//    Thread_arg **args = send_arg->args;

	Thread_arg *sub = (Thread_arg *)arg;

	// unsigned int thread_id = sub->id;

	//    Client *clients[ACTIVE_THREAD];
	//    Producer producer[ACTIVE_THREAD];
	Producer producer;
	ProducerConfiguration producerconfiguration;
#ifdef SEND_BATCH
	producerconfiguration.setBatchingEnabled(true);
	producerconfiguration.setBatchingMaxAllowedSizeInBytes(MESSAGE_LENGTH * P_BATCH_SEND_NUM * 2);
	producerconfiguration.setBatchingMaxMessages(P_BATCH_SEND_NUM);
	producerconfiguration.setBatchingMaxPublishDelayMs(1);
#endif

#ifdef SEND_COMPRESS
	producerconfiguration.setCompressionType(pulsar::CompressionLZ4);
#endif
	// producerconfiguration.setCompressionType(pulsar::CompressionZSTD);
	// producerconfiguration.setCompressionType(pulsar::CompressionZLib);
	// Client *client = new Client("pulsar://192.168.20.32:6650,192.168.20.37:6650,192.168.20.36:6650");
	// Client *client = new Client("pulsar://192.168.20.50:6650");
	Client *client = new Client("pulsar://localhost:6650");

	//        clients[i] = new Client("pulsar://localhost:6650");
	// std::string topic_name = "LD_" + std::to_string(thread_id);
	//    std::string topic_name = "LD_0003";
	//std::string topic_name = "persistent://public/default/test-topic";
	std::string topic_name = sub->pulsar_topic_name;

	// Result res = client->createProducer(topic_name, producer);
	Result res = client->createProducer(topic_name, producerconfiguration, producer);
	if (res != ResultOk)
	{
		spdlog::error("Create Producer Failed");
	}

	// byte tmp[23][1460];
	// byte tmp[(MESSAGE_LENGTH + 2) * 30000];
	// byte *tmp = (byte *)malloc((MESSAGE_LENGTH + 2) * 30000);
	byte *tmp = (byte *)rte_malloc(NULL, (MESSAGE_LENGTH + 2) * 100, 0);

	Message tmp_msg[16];

	// int sent_frames = 0;
	int send_id;
	unsigned int timeflag = 0;
	unsigned int last_timeflag = 0;
	// spdlog::info("KAZE INIT START");
	Message msg;

	sub->send_thread_init = true;

	udpFrame_1460 *udpFrame_ptr = nullptr;

	while (true)
	{
		if (rte_ring_dequeue(sub->ring3_4, (void **)&udpFrame_ptr))
		{
			continue;
		}
		memset(tmp, 0, MESSAGE_LENGTH + 2);
// async_file1->warn("delete:{}", send_id);
#ifdef LOG
		sub->async_log2->info("sent:{}", send_id);
#endif
		// spdlog::warn("delete:{} ", send_id);,send_id

		// memcpy(tmp + 2, sub->local_UDPFrameIndex->pUDPFrame[send_id]->data,
		//        sizeof(byte) * PACK_NUM * DATA_LENGTH);
		// spdlog::info("START KAZE INIT MEMCPY");

		memcpy(tmp + 2, udpFrame_ptr->data,
			   MESSAGE_LENGTH);
#ifdef FAKE_DATA
		memcpy(tmp + 2 + 29, data1,
			   MESSAGE_LENGTH - 29);
#endif
		// spdlog::info("DONE KAZE INIT MEMCPY");
		memcpy(tmp, &send_id, sizeof(unsigned short));
		memcpy(&timeflag, tmp + 26, sizeof(int));

#ifdef LOG
		sub->async_log3->info("init sent timeflag:{}", timeflag);
#endif
		// last_timeflag = timeflag;
		last_timeflag = (timeflag + 1000) % 5600000;
		// async_file2->error("timeflag:{}", timeflag);
		//                memcpy(tmp,sub->local_UDPFrameIndex)

		// spdlog::info("KAZE INIT MESG BUILD");
		msg = MessageBuilder().setContent(tmp, MESSAGE_LENGTH + 2).build();
#ifdef SEND
		// producer.sendAsync(msg, NULL);
#endif
		// producer.send(msg);
		// sub->local_UDPFrameIndex->pUDPFrame[send_id]->count = 0;
		// spdlog::info("frameSeq:{}",udpFrame_ptr->frameSeq);
		udpFrame_ptr->count = 0;
		memset(udpFrame_ptr->flags, 0, PACK_NUM);
		// memset(sub->local_UDPFrameIndex->pUDPFrame[send_id]->flags, 0, PACK_NUM);
		// memset(sub->local_UDPFrameIndex->pUDPFrame[send_id],0,sizeof(udpFrame_1460));
		// udpFrame_ptr = nullptr;
		// sub->local_UDPFrameIndex->pUDPFrame[send_id] = nullptr;
		if (sub->local_UDPFrameIndex->pUDPFrame[udpFrame_ptr->frameSeq] != nullptr)
		{
			sub->local_UDPFrameIndex->pUDPFrame[udpFrame_ptr->frameSeq] = nullptr;
		}

		sub->sent_frame++;

		// printf("pulsar: ->%d: sent frame # %d | total sent: %d\n", sub->id, send_id, sent_frames.load());

		// sub->queue_to_send.try_pop(send_id);
		// break;
	}

	// int i = 0;
	spdlog::info("KAZE INIT DONE");
	int queue_size = 0;
	timeflag = 0;

	while (1)
	{
		queue_size = sub->queue_to_send.size_approx();
		// printf("queue_size:%d\n",queue_size);
		if (queue_size > 10)
		{
			queue_size = 10;
			// usleep(1);
			//    printf("queque length: %d\n",send_arg->queue_to_send.size());
			// printf("send_to_queue: %d\n", sub->queue_to_send.size());

			for (int i = 0; i < queue_size; i++)
			{
				sub->queue_to_send.try_dequeue(send_id);
				memcpy(tmp + 2 + (MESSAGE_LENGTH + 2) * i, sub->local_UDPFrameIndex->pUDPFrame[send_id]->data,
					   MESSAGE_LENGTH);

#ifdef FAKE_DATA
				memcpy(tmp + 2 + 29 + (MESSAGE_LENGTH + 2) * i, data1 + (MESSAGE_LENGTH + 2) * i,
					   MESSAGE_LENGTH - 29);
#endif

				memcpy(tmp + (MESSAGE_LENGTH + 2) * i, &send_id, sizeof(unsigned short));
				// memcpy(&timeflag, tmp + 26 + (MESSAGE_LENGTH + 2) * i, sizeof(int));

				// memcpy(&timeflag, tmp + 26 + (MESSAGE_LENGTH + 2) * i, sizeof(int));
				memcpy(tmp + 26 + (MESSAGE_LENGTH + 2) * i, &timeflag, sizeof(int));
				timeflag += 1000;
				// spdlog::info("timeflag:{}",timeflag);
#ifdef LOG

				sub->async_log3->info("init sent timeflag:{}", timeflag);
#endif

#ifdef DROP
				// memset(sub->local_UDPFrameIndex->pUDPFrame[send_id]->flags, 0, PACK_NUM * sizeof(bool));
				for (int i = 0; i < PACK_NUM; i++)
				{
					sub->local_UDPFrameIndex->pUDPFrame[send_id]->flags[i] = false;
				}
#endif
				sub->local_UDPFrameIndex->pUDPFrame[send_id]->count = 0;
				sub->local_UDPFrameIndex->pUDPFrame[send_id] = nullptr;
				//TODO: clear the indices here

				sub->sent_frame++;
				if (timeflag != last_timeflag)
				{
					sub->mis_msg++;
				}
				last_timeflag = (timeflag + 1000); //% 5600000;
			}

			msg = MessageBuilder().setContent(tmp, (MESSAGE_LENGTH + 2) * queue_size).build();

#ifdef SEND
			producer.sendAsync(msg, NULL);
#endif
		}
		else
		{
			// std::this_thread::yield();
		}

		//        usleep(100);
	}
	for (int i = 0; i < ACTIVE_THREADS; i++)
	{
		client->close();
	}
	free(tmp);
}

void timer_thread(std::vector<Thread_arg *> *args_vec)
{

#ifdef GRAFANA
	zmq::context_t context{1};
	zmq::socket_t socket{context, zmq::socket_type::req};
	socket.connect("tcp://localhost:" + std::to_string(5555 + args->proc_id));
	// socket.connect("tcp://localhost:5555");
#endif
	// std::this_thread::sleep_for(std::chrono::seconds(5));
	// assem_init = true;

#ifndef KAZE
	for (int i = 0; i < args_vec->size(); i++)
	{
		(*args_vec)[i]->send_thread_init = true;
	}

#endif

	while (!(*args_vec)[0]->timer_init || !(*args_vec)[0]->send_thread_init)
	{
		std::this_thread::yield();
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (int i = 0; i < args_vec->size(); i++)
	{
		(*args_vec)[i]->assem_init = true;
	}

	int last_count = 0;
	int diff_count = 0;

	int diff_forward = 0;
	int last_forward = 0;
	spdlog::flush_every(std::chrono::seconds(1));

	while (1)
	{
		for (ssize_t i = 0; i < args_vec->size(); i++)
		{
			diff_count = (*args_vec)[i]->global_count - (*args_vec)[i]->last_count;
			(*args_vec)[i]->last_count = (*args_vec)[i]->global_count;

			diff_forward = (*args_vec)[i]->forward_packet - (*args_vec)[i]->last_forward;
			(*args_vec)[i]->last_forward = (*args_vec)[i]->forward_packet;

			// if (!(args)->frame_queue.empty() && !(args + 1)->frame_queue.empty())
			// {

			//     int queue1_front_id = (args)->frame_queue.front();
			//     int queue2_front_id = (args + 1)->frame_queue.front();
			//     if ((args)->local_UDPFrameIndex->pUDPFrame[queue1_front_id] != nullptr && (args + 1)->local_UDPFrameIndex->pUDPFrame[queue2_front_id] != nullptr)
			//     {
			//         int queue1_front_count = (args)->local_UDPFrameIndex->pUDPFrame[queue1_front_id]->count;
			//         int queue2_front_count = (args + 1)->local_UDPFrameIndex->pUDPFrame[queue1_front_id]->count;
			//         spdlog::info("Sec: {}, Queue Size 0: {},Queue1_front:{} ,Queue Size 2: {}, Queue2_front:{}", sec, (args)->frame_queue.size(), queue1_front_count, (args + 1)->frame_queue.size(), queue2_front_count);
			//     }
			// }
			// spdlog::info("Sec:{}, queue_to_send1: {}, queue_to_send2: {}, Sent Frames: {}, Forward Packets: {}", sec, (args)->queue_to_send.size(), (args + 1)->queue_to_send.size(), sent_frames, forward_packet);
#ifdef DEBUG

			// spdlog::info("Sec:{0:.1f}, Align: {1} ,Sent: {2}, ForwardP: {3},consumerP:{7}, mis:{8}, mis_msg:{9}, c_Speed:{10:.2f}, f_Speed:{11:.2f},queue1:{4}, queue2:{5}, queue3:{6}, drop:{12}", sec, align_num, sent_frames, forward_packet, args->mem_queue.size_approx(), args->frame_queue.size_approx(), args->queue_to_send.size_approx(), global_count, mis, mis_msg, diff_count * 0.00001123046875 / TIME_STAMP, diff_forward * 0.00001123046875 / TIME_STAMP, drop_count);

#ifdef GRAFANA
			std::string output_to_mq = fmt::format("nodeNum:{13},Sec:{0:.1f},align_frames:{1},sent_frames:{2},forward_packets:{3},consume_packets:{7},missing_packets:{8},missing_frames:{9},consumer_speed:{10:.2f},forward_speed:{11:.2f},queue1:{4},queue2:{5},queue3:{6},drop_frames:{12}", sec, align_num, sent_frames, forward_packet, args->mem_queue.size_approx(), args->frame_queue.size_approx(), args->queue_to_send.size_approx(), global_count, mis, mis_msg, diff_count * 0.00001123046875 / TIME_STAMP, diff_forward * 0.00001123046875 / TIME_STAMP, drop_count, args->channel_id / 4);
			socket.send(zmq::buffer(output_to_mq), zmq::send_flags::none);

			zmq::message_t reply{};
			socket.recv(reply, zmq::recv_flags::none);
#endif
#ifdef DEBUG_DISPLAY

			// spdlog::info("C{11}:Sec:{0:.1f}, align num: {1} ,Sent Frames: {2}, Forward Packets: {3}, queue1:{4}, queue2:{5}, queue3:{6}, global_count:{7}, mis:{8}, mis_msg:{9}, Speed:{10:.2f}", sec, (*args_vec)[i]->align_num, (*args_vec)[i]->sent_frame, (*args_vec)[i]->forward_packet, (*args_vec)[i]->mem_queue.size_approx(), (*args_vec)[i]->frame_queue.size_approx(), (*args_vec)[i]->queue_to_send.size_approx(), (*args_vec)[i]->global_count, (*args_vec)[i]->mis, (*args_vec)[i]->mis_msg, diff_count * 0.00001123046875 / TIME_STAMP, i);
			spdlog::info("C{11}:Sec:{0:.1f}, align num: {1} ,Sent Frames: {2}, Forward Packets: {3}, queue1:{4}, queue2:{5}, queue3:{6}, global_count:{7}, mis:{8}, mis_msg:{9}, Speed:{10:.2f}", sec, (*args_vec)[i]->align_num, (*args_vec)[i]->sent_frame, (*args_vec)[i]->forward_packet, rte_ring_count((*args_vec)[i]->ring1_2), rte_ring_count((*args_vec)[i]->ring2_3), rte_ring_count((*args_vec)[i]->ring3_4), (*args_vec)[i]->global_count, (*args_vec)[i]->mis, (*args_vec)[i]->mis_msg, diff_count * 0.00001123046875 / TIME_STAMP, i);

#endif
#endif

#ifndef DEBUG
			spdlog::info("秒:{0}, 组帧: {2} ,发送: {3}, 收包: {4}, 拆包:{5}，速率：{1:.2f} Gbps, mis:{6}, mis_msg:{7}", sec, diff_count * 0.00001123046875, align_num, sent_frames, forward_packet, global_count, mis, mis_msg);

#endif
		}
		std::cout << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(TIME_STAMP * 1000)));

		sec = sec + TIME_STAMP;
	}
}

/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int main(int argc, char *argv[])
{

	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;
	uint16_t portid;
	unsigned lcore_id;

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;
	argv += ret;

	int i = 0;
	RTE_LCORE_FOREACH_WORKER(lcore_id)
	{
		// std::cout << lcore_id <<" ," <<std::endl;
		// args_vec[i]->channel_id = lcore_id;
		i++;
	}
	const int NUM_CHANNELS = i;

	// Thread_arg args;
	std::vector<Thread_arg *> args_vec;
	std::vector<std::thread> align_threads;
	std::vector<std::thread> send_threads;
	std::vector<std::thread> assemble_threads;

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		args_vec.emplace_back(new Thread_arg());
		args_vec[i]->id = 1400 + i;
		args_vec[i]->pulsar_topic_name = "persistent://public/default/test-topi" + std::to_string(i);
		// args_vec[i]->channel_id = (10 + i) * 2;
		args_vec[i]->proc_id = i;
	}
	i = 0;
	RTE_LCORE_FOREACH_WORKER(lcore_id)
	{
		// std::cout << lcore_id <<" ," <<std::endl;
		args_vec[i]->channel_id = lcore_id;
		i++;
	}

	// for(int i = 0;i<NUM_CHANNELS;i++)
	// {
	// 	args_vec[i]->id = 1400+i;
	// }

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		printf("id:%d\n", args_vec[i]->id);
		std::cout << "pulsar_topic_name: " << args_vec[i]->pulsar_topic_name << std::endl;
		std::cout << "channel_id: " << args_vec[i]->channel_id << std::endl;
		std::cout << "proc_id: " << args_vec[i]->proc_id << std::endl;
	}
	std::cout << "______" << std::endl;

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		printf("id:%d\n", args_vec[i]->id);
		std::cout << "topic:" << args_vec[i]->pulsar_topic_name << std::endl;
		std::cout << "channel_id:" << args_vec[i]->channel_id << std::endl;
		std::cout << "proc_id:" << args_vec[i]->proc_id << std::endl;
	}
	// return 0;

	// args.id = 1400;
	// args.offset = 0;
	// args.pulsar_topic_name = "persistent://public/my-namespace/test-topic1";
	// printf("topic:%s\n", args.pulsar_topic_name.c_str());
	// args.channel_id = 10 * 2;
	// args.proc_id = args.channel_id;
	// printf("\n--------%d\n\n", args.channel_id);

#ifndef QUEUE

#ifdef RING
	std::string PRI_2_SEC;
	std::string _MSG_POOL;

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		PRI_2_SEC = "PRI_2_SEC" + std::to_string(i);
		_MSG_POOL = "MSG_POOL" + std::to_string(i);
		spdlog::info("try to malloc ring:{}", i);
		args_vec[i]->ring1_2 = rte_ring_create(PRI_2_SEC.c_str(), ring_size, rte_socket_id(), 0);
		if (args_vec[i]->ring1_2 == nullptr)
		{
			spdlog::error("malloc ring failed");
			return -1;
		}

		PRI_2_SEC = "PRI_2_SEC_1" + std::to_string(i);
		// _MSG_POOL = "MSG_POOL_1" + std::to_string(i);
		spdlog::info("try to malloc ring2:{}", i);
		args_vec[i]->ring2_3 = rte_ring_create(PRI_2_SEC.c_str(), ring_size, rte_socket_id(), 0);
		if (args_vec[i]->ring2_3 == nullptr)
		{
			spdlog::error("malloc ring failed");
			return -1;
		}

		PRI_2_SEC = "PRI_2_SEC_2" + std::to_string(i);
		// _MSG_POOL = "MSG_POOL" + std::to_string(i);
		spdlog::info("try to malloc ring3:{}", i);
		args_vec[i]->ring3_4 = rte_ring_create(PRI_2_SEC.c_str(), ring_size, rte_socket_id(), 0);
		if (args_vec[i]->ring3_4 == nullptr)
		{
			spdlog::error("malloc ring failed");
			return -1;
		}

		spdlog::info("try to malloc pool:{}", i);
		args_vec[i]->send_pool = rte_mempool_create(_MSG_POOL.c_str(), pool_size, STR_TOKEN_SIZE, pool_cache, priv_data_sz, NULL, NULL, NULL, NULL, rte_socket_id(), flags);
		if (args_vec[i]->send_pool == nullptr)
		{
			spdlog::error("malloc mempool failed:{}", i);
			return -1;
		}
		spdlog::info("done ring & mempool:{}", i);
	}

#endif

#ifndef RING

	spdlog::info("START TO ALLOC");
	auto *start_pointer = (list_node *)malloc(sizeof(list_node) * LINKED_NODE_NUM);
	for (int i = 0; i < LINKED_NODE_NUM; i++)
	{
		(start_pointer + i)->next_node = (start_pointer + i + 1);
		(start_pointer + i)->is_empty = true;
	}
	(start_pointer + LINKED_NODE_NUM - 1)->next_node = start_pointer;
	args.cur_consumer = start_pointer;
	args.cur_producer = start_pointer;
	spdlog::info("DONE ALLOC");
#endif

#endif

	for (int i = 0; i < NUM_CHANNELS; i++)
	{

		spdlog::info("START TO ALLOCATE POOL & INDEX:{}", i);
		args_vec[i]->local_UDPFrameIndex = new udpFramesIndex65536_1460();
		args_vec[i]->local_UDPFramePool = new udpFramesPool_1460();

		// memset(args.local_UDPFramePool,0,sizeof(udpFramesPool_1460));
		// memset(args.local_UDPFrameIndex,0,sizeof(udpFramesIndex65536_1460));
		spdlog::info("ALLOCATED POOL & INDEX:{}", i);
	}

	// LOG FILE
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		args_vec[i]->async_log1 = spdlog::basic_logger_mt("basic_logger_" + std::to_string(i), log_dir + std::to_string(i) + "_t2.log");
		args_vec[i]->async_log2 = spdlog::basic_logger_mt("basic_logger1_" + std::to_string(i), log_dir + std::to_string(i) + "_t3.log");
		args_vec[i]->async_log3 = spdlog::basic_logger_mt("basic_logger3_" + std::to_string(i), log_dir + std::to_string(i) + "_t4.log");
		args_vec[i]->store_log = spdlog::basic_logger_mt("basic_logger4_" + std::to_string(i), log_dir + std::to_string(i) + "_t5.log");
	}

	cpu_set_t mask;
#ifdef DOB
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		assemble_threads.emplace_back(consumer_thread, args_vec[i]);
		CPU_ZERO(&mask);
		CPU_SET(args_vec[i]->channel_id + 1, &mask);
		pthread_setaffinity_np(assemble_threads[i].native_handle(), sizeof(cpu_set_t), &mask);
	}
	spdlog::warn("INIT DOB");

#endif

#ifdef GODOT

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		align_threads.emplace_back(align_thread, args_vec[i]);
		// cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(args_vec[i]->channel_id + 2, &mask);
		pthread_setaffinity_np(align_threads[i].native_handle(), sizeof(cpu_set_t), &mask);
	}

#endif

#ifdef KAZE
	for (int i = 0; i < NUM_CHANNELS; i++)
	{

		send_threads.emplace_back(send_to_pulsar, args_vec[i]);
		// cpu_set_t mask;
		CPU_ZERO(&mask);
		// CPU_SET(args_vec[i]->channel_id + 1 + 40, &mask);
		CPU_SET(args_vec[i]->channel_id + 3, &mask);
		pthread_setaffinity_np(send_threads[i].native_handle(), sizeof(cpu_set_t), &mask);
	}
#endif
	// unsigned num_cpus = std::thread::hardware_concurrency();

	std::thread t1(timer_thread, &args_vec);
	CPU_ZERO(&mask);
	// CPU_SET(20, &mask);
	CPU_SET(4, &mask);
	pthread_setaffinity_np(t1.native_handle(), sizeof(cpu_set_t), &mask);

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count_avail();
	if (nb_ports == 0)
	{
		rte_exit(EXIT_FAILURE, "No ethernet ports --bye\n");
	}

	// nb_port == lcore_number

	// if (nb_ports < 2 || (nb_ports & 1))
	// 	rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

	/* Creates a new mempool in memory to hold the mbufs. */

	unsigned int nb_lcores = rte_lcore_count();

	unsigned int nb_mbufs;
	nb_mbufs = RTE_MAX(nb_ports * (nb_rxd + nb_txd + MAX_PKT_BURST +
								   nb_lcores * MEMPOOL_CACHE_SIZE),
					   8192U);

	printf("nb_port:%d, nb_mbufs:%d, nb_lcores:%d\n", nb_ports, nb_mbufs, nb_lcores);
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
										MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	printf("RTE_MBUF_DEFAULT_BUF_SIZE:%d\n", RTE_MBUF_DEFAULT_BUF_SIZE);
	// TODO: switch to multi numa socket version, allocate mbuf_pool on each numa socket.

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	/* Initialize all ports. */
	RTE_ETH_FOREACH_DEV(portid)
	if (port_init(portid, mbuf_pool) != 0)
		rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu16 "\n",
				 portid);

	// if (rte_lcore_count() > 1)
	// printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the main core only. */
	// lcore_main();

	// int ports[2] = {0, 1};
	// int i = 0;
	// spdlog::info("KKKKK");
	i = 0;
	RTE_LCORE_FOREACH_WORKER(lcore_id)
	{
		rte_eal_remote_launch(lcore_main, args_vec[i], lcore_id);

		i++;
	}

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		args_vec[i]->timer_init = true;
	}
	// timer_init = true;

	RTE_LCORE_FOREACH_WORKER(lcore_id)
	{
		rte_eal_wait_lcore(lcore_id);
	}

	return 0;
}
