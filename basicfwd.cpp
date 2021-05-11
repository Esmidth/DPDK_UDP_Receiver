/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include "Data_Struct.h"
#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <sys/socket.h>
#include <netinet/udp.h>

#include <iostream>

#include <netinet/ip.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define MAX_PKT_BURST 32
#define MEMPOOL_CACHE_SIZE 256

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32



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
/* basicfwd.c: Basic DPDK skeleton forwarding example. */

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
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
		printf("%x ", *(uint8_t *)(buf->buf_addr + buf->data_off + j));
	}
	printf("\n");
	// ip = (struct iphdr *)(bufs[i]->buf_addr + bufs[i]->data_off + 14);
	// printf("tos:%hu, tot_len:%hu, id:%hu, frag_off:%hu, ttl:%hu, protoco:%hu, check:%hu, saddr:%d, daddr:%d\n", ip->tos, ip->tot_len, ip->id, ip->frag_off, ip->ttl, ip->protocol, ip->check, ip->saddr, ip->daddr);

	// udp_header = (struct udphdr *)(bufs[i]->buf_addr + bufs[i]->data_off + 34);
	// printf("udp:\nsourec: %hu, dest:%hu, len:%hu, check:%hu\n", udp_header->source, udp_header->dest, udp_header->len, udp_header->check);
	// printf("udp_len_hex:%x %x\n", *(uint8_t *)(bufs[i]->buf_addr + bufs[i]->data_off + 38), *(uint8_t *)(bufs[i]->buf_addr + bufs[i]->data_off + 39));
	uint16_t tmp_len;
	tmp_len = (*(uint8_t *)(buf->buf_addr + buf->data_off + 38) << 8) | (*(uint8_t *)(buf->buf_addr + buf->data_off + 39));
	// printf("udp_len:%d\n", *(uint16_t *)(bufs[i]->buf_addr + bufs[i]->data_off + 38));
	printf("udp_len_new:%d\n", tmp_len);
	printf("------------------\n");
}

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */
static __rte_noreturn void
lcore_main(void)
{
	uint16_t port;

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
	for (;;)
	{
		/*
		 * Receive packets on a port and forward them on the paired
		 * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
		 */
		RTE_ETH_FOREACH_DEV(port)
		{

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
			udpPacket_1460 *tmp_packet_ptr;
			for (int i = 0; i < nb_rx; i++)
			{
				if (bufs[i]->pkt_len == 1514)
				// if(true)
				{
					// printf("pkt_len:%hu data_len:%d buf_len:%d data_off:%d\n", bufs[i]->pkt_len, bufs[i]->data_len, bufs[i]->buf_len, bufs[i]->data_off);
					//printf("%x %x %x %x %x %x %x\n", *(char*)(bufs[i]->buf_addr + bufs[i]->data_off), 0, 0, 0, 0, 0, 0);
					tmp_packet_ptr = (udpPacket_1460 *)(bufs[i]->buf_addr + bufs[i]->data_off + 42);
					printf("frameSeq:%d, packetSeq:%d, packetLen:%d\n", tmp_packet_ptr->frameSeq, tmp_packet_ptr->packetSeq, tmp_packet_ptr->packetLen);
					// print_pkt(bufs[i]);
					count[port] += 1;
					printf("count:%d port_id:%d\n----------\n", count[port], port);
				}
			}

			/* Send burst of TX packets, to second port of pair. */
			// const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, 0,
			// 		bufs, nb_rx);
			const uint16_t nb_tx = 0;

			/* Free any unsent packets. */
			if (unlikely(nb_tx < nb_rx))
			{
				uint16_t buf;
				for (buf = nb_tx; buf < nb_rx; buf++)
					rte_pktmbuf_free(bufs[buf]);
			}
		}
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

/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int main(int argc, char *argv[])
{
	std::cout << "HELLO WORLD FROM CPP" << std::endl;
	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;
	uint16_t portid;

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;
	argv += ret;

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

	unsigned int nb_lcores = nb_ports;

	unsigned int nb_mbufs;
	nb_mbufs = RTE_MAX(nb_ports * (nb_rxd + nb_txd + MAX_PKT_BURST +
								   nb_lcores * MEMPOOL_CACHE_SIZE),
					   8192U);

	printf("nb_port:%d, nb_mbufs:%d, nb_lcores:%d\n",nb_ports,nb_mbufs,nb_lcores);
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
										MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	printf("RTE_MBUF_DEFAULT_BUF_SIZE:%d\n", RTE_MBUF_DEFAULT_BUF_SIZE);

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	/* Initialize all ports. */
	RTE_ETH_FOREACH_DEV(portid)
	if (port_init(portid, mbuf_pool) != 0)
		rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu16 "\n",
				 portid);

	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the main core only. */
	lcore_main();

	return 0;
}
