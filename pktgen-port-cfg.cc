#include <fcntl.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <rte_common.h>
#include <rte_eal.h>
#include <rte_flow.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>

#include "pktgen-constants.h"
#include "pktgen-port-cfg.h"
// #include "pktgen-ip.h"
#include "pktgen.h"

/* RX queue configuration */
static struct rte_eth_rxconf rx_conf = {
    .rx_thresh = {
        .pthresh = 8,
        .hthresh = 8,
        .wthresh = 4,
    },
    .rx_free_thresh = 32,
};

/* TX queue configuration */
static struct rte_eth_txconf tx_conf = {
    .tx_thresh = {
        .pthresh = 36,
        .hthresh = 0,
        .wthresh = 0,
    },
    .tx_free_thresh = 0,
};

/* Port configuration */
struct rte_eth_conf port_conf = {
    .rxmode = {
        .mq_mode        = RTE_ETH_MQ_RX_RSS,
        .split_hdr_size = 0,
    },
    .txmode = {
        .mq_mode = ETH_MQ_TX_NONE,
        .offloads = (DEV_TX_OFFLOAD_IPV4_CKSUM |
                DEV_TX_OFFLOAD_UDP_CKSUM |
                DEV_TX_OFFLOAD_TCP_CKSUM),
    },
	.rx_adv_conf = {
		.rss_conf = {
			.rss_key = NULL,
			.rss_hf = ETH_RSS_IP | ETH_RSS_UDP,
		},
	},
};

static struct rte_mempool * pktgen_mempool_create(const char *type, uint8_t pid, uint8_t queue_id,
			uint32_t nb_mbufs, int socket_id, int cache_size){
	struct rte_mempool * mp;
	char name[RTE_MEMZONE_NAMESIZE];

	snprintf(name, sizeof(name), "%-12s%u:%u", type, pid, queue_id);
	printf("    Create: %-*s - Memory used (MBUFs %5u x (size %u + Hdr %lu)) + %lu = %6lu KB headroom %d %d\n",
		16,
		name,
		nb_mbufs,
		MBUF_SIZE,
		sizeof(struct rte_mbuf),
		sizeof(struct rte_mempool),
		(((nb_mbufs * (MBUF_SIZE + sizeof(struct rte_mbuf)) +
		   sizeof(struct rte_mempool))) + 1023) / 1024,
		RTE_PKTMBUF_HEADROOM,
		RTE_MBUF_DEFAULT_BUF_SIZE);

	/* create the mbuf pool */
	mp = rte_mempool_create(name, nb_mbufs, MBUF_SIZE, cache_size,
							sizeof(struct rte_pktmbuf_pool_private), 
							rte_pktmbuf_pool_init, NULL,
                            rte_pktmbuf_init, NULL,
							socket_id, MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET);
	if (mp == NULL) {
		printf("Cannot create mbuf pool (%s) port %d, queue %d, nb_mbufs %d, socket_id %d: %s\n",
			name, pid, queue_id, nb_mbufs, socket_id, rte_strerror(errno));
    }

	return mp;
}

#define FULL_IP_MASK   0xffffffff /* full mask */
#define EMPTY_IP_MASK  0x0 /* empty mask */

#define FULL_PORT_MASK   0xffff /* full mask */
#define PART_PORT_MASK   0xff00 /* partial mask */
#define EMPTY_PORT_MASK  0x0 /* empty mask */

#define MAX_PATTERN_NUM		4
#define MAX_ACTION_NUM		2

void pktgen_create_flow(uint32_t rx_core) {
	uint32_t pid;
    uint16_t dst_port;
	struct rte_flow_error error;
	struct rte_flow_attr attr;
	struct rte_flow_item pattern[MAX_PATTERN_NUM];
	struct rte_flow_action action[MAX_ACTION_NUM];
	struct rte_flow * flow = NULL;
	struct rte_flow_action_queue queue = { .index = (uint16_t)((rx_core - 1) / 2) };
	struct rte_flow_item_ipv4 ip_spec;
	struct rte_flow_item_ipv4 ip_mask;
	struct rte_flow_item_udp udp_spec;
	struct rte_flow_item_udp udp_mask;
	int res;

	dst_port = (rx_core - 1) << 8;

	RTE_ETH_FOREACH_DEV(pid) {
        memset(pattern, 0, sizeof(pattern));
        memset(action, 0, sizeof(action));

        /*
        * set the rule attribute.
        * in this case only ingress packets will be checked.
        */
        memset(&attr, 0, sizeof(struct rte_flow_attr));
        attr.ingress = 1;
        attr.priority = 0;

        /*
        * create the action sequence.
        * one action only,  move packet to queue
        */
        action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
        action[0].conf = &queue;
        action[1].type = RTE_FLOW_ACTION_TYPE_END;

        /*
        * set the first level of the pattern (ETH).
        */
        pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;

        /*
        * setting the second level of the pattern (IP).
        */
        memset(&ip_spec, 0, sizeof(struct rte_flow_item_ipv4));
        memset(&ip_mask, 0, sizeof(struct rte_flow_item_ipv4));
        pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
        pattern[1].spec = &ip_spec;
        pattern[1].mask = &ip_mask;

		/*
		* setting the third level of the pattern (UDP).
		*/
		memset(&udp_spec, 0, sizeof(struct rte_flow_item_udp));
		memset(&udp_mask, 0, sizeof(struct rte_flow_item_udp));
		udp_spec.hdr.dst_port = htons(dst_port);
		udp_mask.hdr.dst_port = htons(PART_PORT_MASK);
		pattern[2].type = RTE_FLOW_ITEM_TYPE_UDP;
		pattern[2].spec = &udp_spec;
		pattern[2].mask = &udp_mask;

        /* the final level must be always type end */
        pattern[3].type = RTE_FLOW_ITEM_TYPE_END;

		printf("Direct flow to port %x to core %d via queue %d\n", dst_port & PART_PORT_MASK, rx_core, (rx_core - 1) / 2);

        res = rte_flow_validate(pid, &attr, pattern, action, &error);
        if (!res) {
retry:
            flow = rte_flow_create(pid, &attr, pattern, action, &error);
            if (!flow) {
                rte_flow_flush(pid, &error);
                goto retry;
            }
        } else {
            printf("control: invalid flow rule! msg: %s\n", error.message);
        }
    }
}

void pktgen_config_ports(void) {
    int nb_dev_total;
	uint32_t pid;
	int32_t ret, cache_size;
    uint16_t nb_core;
    // struct rte_ether_addr addr;
    struct rte_eth_dev_info dev_info;

    cache_size = RTE_MEMPOOL_CACHE_MAX_SIZE;
    nb_core = pktgen.nb_core;
    nb_dev_total = rte_eth_dev_count_total();

	if (nb_dev_total == 0) {
		perror("*** Did not find any ports to use ***");
    }

	printf("Configuring %d ports, MBUF Size %d, MBUF Cache Size %d\n",
		    nb_dev_total, MBUF_SIZE, MBUF_CACHE_SIZE);

    RTE_ETH_FOREACH_DEV(pid) {
        printf("Initialize Port %d -- TxQ %d, RxQ %d\n", pid, nb_core, nb_core);

    	/* Get Ethernet device info */
        ret = rte_eth_dev_info_get(pid, &dev_info);
	    if (ret < 0) {
            printf("Error during getting device (port %u) info: %s\n", pid, strerror(-ret));
        }

        /* Configure # of RX and TX queue for port */
        ret = rte_eth_dev_configure(pid, nb_core, nb_core, &port_conf);
    	if (ret < 0) {
	    	printf("Cannot configure device: err=%d, port=%u\n", ret, pid);
	    }

        for (uint16_t i = 0; i < nb_core; i++) {
            /* 1. Create mempool for each core */
            core_info[i].pkt_mempool = pktgen_mempool_create("Default mp", pid, i, MAX_MBUFS_PER_PORT, rte_socket_id(), cache_size);
            if (!core_info[i].pkt_mempool) {
                printf("Cannot init port %d for Default RX mbufs\n", pid);
            }

            /* 2. Configure RX/TX queue for each core */
            ret = rte_eth_rx_queue_setup(pid, i, pktgen.nb_rxd, rte_socket_id(), &rx_conf, core_info[i].pkt_mempool);
            if (ret < 0) {
                printf("rte_eth_rx_queue_setup: err=%d, port=%d, %s\n", ret, pid, rte_strerror(-ret));
            }

            ret = rte_eth_tx_queue_setup(pid, i, pktgen.nb_txd,
		    		rte_eth_dev_socket_id(pid), &tx_conf);
    		if (ret < 0) {
                printf("rte_eth_rx_queue_setup: err=%d, port=%d, %s\n", ret, pid, rte_strerror(-ret));
		    }

            for (int j = 0; j < DEFAULT_PKT_BURST; j++) {
                /* Allocate TX packet buffer in DPDK context memory pool */
                core_info[i].tx_mbufs[pid].m_table[j] = rte_pktmbuf_alloc(core_info[i].pkt_mempool);
                assert(core_info[i].tx_mbufs[pid].m_table[j] != NULL);
            }

            core_info[i].tx_mbufs[pid].len = 0;
        }
    }

    RTE_ETH_FOREACH_DEV(pid) {
        ret = rte_eth_promiscuous_enable(pid);
        if (ret != 0) {
            printf("rte_eth_promiscuous_enable:err = %d, port = %u\n", ret, (unsigned)pid);
        }

        /* Start device */
        ret = rte_eth_dev_start(pid);
		if (ret != 0) {
            printf("rte_eth_dev_start: port=%d, %s\n", pid, rte_strerror(-ret));
        }
    }

	for (uint16_t i = 0; i < pktgen.nb_core; i++) {
		if ((i + 1) % 2 == 0) {
			/* Receive core */
			pktgen_create_flow(i + 1);
		}
	}
}
