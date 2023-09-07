#ifndef _PKTGEN_H_
#define _PKTGEN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <assert.h>
#include <time.h>
#include <linux/udp.h>

#include <rte_version.h>
#include <rte_config.h>
#include <rte_errno.h>
#include <rte_log.h>
#include <rte_tailq.h>
#include <rte_common.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_timer.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_tcp.h>

#include "client.h"
#include "workload.h"
#include "db_workload.h"
#include "syn_workload.h"
#include "properties.h"
#include "pktgen-constants.h"
#include "pktgen-port-cfg.h"

#define NR_MAX_CPU      64
#define NR_MAX_CLIENT   64

enum {
	DEFAULT_NETMASK         = 0xFFFFFF00,
	DEFAULT_IP_ADDR         = (192 << 24) | (168 << 16),
	DEFAULT_TX_COUNT        = 0,	/* Forever */
	DEFAULT_TX_RATE         = 100,
	DEFAULT_PRIME_COUNT     = 1,
	DEFAULT_SRC_PORT        = 1234,
	DEFAULT_DST_PORT        = 5678,
	DEFAULT_TTL		        = 64,
	DEFAULT_TCP_SEQ_NUMBER  = 0x012345678,
	MAX_TCP_SEQ_NUMBER      = UINT32_MAX,
	DEFAULT_TCP_ACK_NUMBER  = 0x012345690,
	MAX_TCP_ACK_NUMBER      = UINT32_MAX,
	DEFAULT_WND_SIZE        = 8192,
	MIN_VLAN_ID             = 1,
	MAX_VLAN_ID             = 4095,
	DEFAULT_VLAN_ID         = MIN_VLAN_ID,
	MIN_COS             	= 0,
	MAX_COS 	            = 7,
	DEFAULT_COS	         	= MIN_COS,
	MIN_TOS             	= 0,
	MAX_TOS 	            = 255,
	DEFAULT_TOS		        = MIN_TOS,
	MAX_ETHER_TYPE_SIZE     = 0x600,
	OVERHEAD_FUDGE_VALUE    = 50,
};

typedef struct core_info {
	int nb_client;
    const struct client_operations * client_ops;	/**< Client operations */
    struct client clients[NR_MAX_CLIENT];

    /* RX/TX info */
    struct timeval start;
    struct timeval end;
	uint64_t total_rx;
	uint64_t total_tx;

	struct rte_mempool * pkt_mempool;
    struct mbuf_table rx_mbufs[RTE_MAX_ETHPORTS];	/**< Pool pointer for port RX mbufs */
	struct mbuf_table tx_mbufs[RTE_MAX_ETHPORTS];	/**< Pool pointer for default TX mbufs */

	Workload * wl;
} __attribute__ ((aligned(64))) core_info_t;

extern core_info_t core_info[NR_MAX_CPU];

typedef struct pktgen_s {
	uint16_t ident;			/**< IPv4 ident value */
	uint16_t nb_core;		/**< Number of cores in the system */
	uint16_t nb_client;		/**< Number of flows per core */

	uint16_t nb_rxd;	/**< Number of receive descriptors */
	uint16_t nb_txd;	/**< Number of transmit descriptors */

	uint64_t hz;		/**< Number of events per seconds */
    double tx_rate;		/**< Percentage rate for tx packets with fractions */

    const struct client_operations * client_ops;	/**< Client operations */

	Properties props;	/**< Workload properties */
} pktgen_t;

extern pktgen_t pktgen;

int pktgen_launch_one_lcore(void * arg);

#endif  /* _PKTGEN_H_ */