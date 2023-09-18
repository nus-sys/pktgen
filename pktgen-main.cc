#include <stdint.h>
#include <termios.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <future>
#include <link.h>
#include <atomic>

#include "client.h"
#include "dummy_client.h"
#include "shinjuku_client.h"

#include "pktgen.h"
#include "pktgen-info.h"

static int pktgen_parse_args(int argc, char ** argv) {
	int opt, num_core, num_flow;
	int option_index;
	double rate;
	static struct option lgopts[] = {
		{"crc-strip", 0, 0, 0},
		{NULL, 0, 0, 0}
	};
	std::ifstream input;

	std::cout << "Test configuration >>" << std::endl;

	while ((opt = getopt_long(argc, argv, "c:f:r:l:p:h", lgopts, &option_index)) != EOF)
		switch (opt) {
		case 'c':	/* Number of cores we are using */
			num_core = strtol(optarg, NULL, 10);
			pktgen.nb_core = num_core;
			break;

		case 'f':	/* Number of flows per core */
			num_flow = strtol(optarg, NULL, 10);
			pktgen.nb_client = num_flow;
			break;

		case 'r':	/* Transmit rate */
			rate = strtod(optarg, NULL);
			pktgen.tx_rate = rate;
			break;
        
        case 'l':
			if (!strcmp(optarg, "dummy")) {
				printf("\t * Client: DUMMY\n");
				pktgen.client_ops = &dummy_ops;
			} else if (!strcmp(optarg, "shinjuku")) {
				printf("\t * Client: SHINJUKU\n");
				pktgen.client_ops = &shinjuku_ops;
			}
			break;

		case 'p':
			input.open(optarg);
			printf("\t * Config file: %s\n", optarg);
			/* Load workload configuration */
            try {
                pktgen.props.Load(input);
            } catch (const std::string &message) {
                std::cout << message << std::endl;
                exit(0);
            }
            input.close();
			break;

		case 'h':	/* print out the help message */
			// pktgen_usage(prgname);
			return -1;

		case 0:	/* crc-strip for all ports */
			break;
		default:
			return -1;
		}
    return 0;
}

#define MAX_BACKTRACE	32

static void sig_handler(int v __rte_unused) {
	void *array[MAX_BACKTRACE];
	size_t size;
	char **strings;
	size_t i;

	if (v == SIGINT) {
		return;
	}

	printf("\n======");
	if (v == SIGSEGV) {
		printf("Pktgen got a Segment Fault\n");
	} else if (v == SIGHUP) {
		printf("Pktgen received a SIGHUP\n");
	} else if (v == SIGPIPE) {
		printf("Pktgen received a SIGPIPE\n");
		return;
	} else {
		printf("Pktgen received signal %d\n", v);
	}

	printf("\n");

	size = backtrace(array, MAX_BACKTRACE);
	strings = backtrace_symbols(array, size);

	printf("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++) {
		printf("%s\n", strings[i]);
	}

	free(strings);

	abort();
}

int pktgen_config_info(void) {
	std::string src_mac_addr, dst_mac_addr;
	std::string src_ip_addr, dst_ip_addr;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dst_mac[ETH_ALEN];
	uint32_t src_ip, dst_ip;
	uint16_t dport;

	src_mac_addr = pktgen.props.GetProperty("src_mac", "00:00:00:00:00:00");
	dst_mac_addr = pktgen.props.GetProperty("dst_mac", "00:00:00:00:00:00");
	src_ip_addr = pktgen.props.GetProperty("src_ip", "10.10.1.2");
	dst_ip_addr = pktgen.props.GetProperty("dst_ip", "10.10.1.1");
	dport = std::stoi(pktgen.props.GetProperty("dport", "1234"));

	if (sscanf(src_mac_addr.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &src_mac[0], &src_mac[1], &src_mac[2], &src_mac[3], &src_mac[4], &src_mac[5])) {
	    memcpy(&pkt_info.eth_src_addr, src_mac, ETH_ALEN);
	}

	if (sscanf(dst_mac_addr.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &dst_mac[0], &dst_mac[1], &dst_mac[2], &dst_mac[3], &dst_mac[4], &dst_mac[5])) {
		memcpy(&pkt_info.eth_dst_addr, dst_mac, ETH_ALEN);
	}

	inet_pton(AF_INET, src_ip_addr.c_str(), &src_ip);
	inet_pton(AF_INET, dst_ip_addr.c_str(), &dst_ip);

	pkt_info.ip_src_addr = src_ip;
	pkt_info.ip_dst_addr = dst_ip;
	pkt_info.dport = dport;

	return 0;
}

int main(int argc, char **argv) {
	uint32_t i;
	int32_t ret;

	signal(SIGSEGV, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGPIPE, sig_handler);

	pktgen.ident	= 0x1234;
	pktgen.nb_rxd	= DEFAULT_RX_DESC;
	pktgen.nb_txd	= DEFAULT_TX_DESC;
	pktgen.tx_rate	= 1.0;

	/* initialize EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0) {
		return -1;
    }

	argc -= ret;
	argv += ret;

	/* parse application arguments (after the EAL ones) */
	ret = pktgen_parse_args(argc, argv);
	if (ret < 0) {
		return -1;
	}

	pktgen.hz = rte_get_timer_hz();	/* Get the starting HZ value. */

	printf(">>> Packet Burst %d, RX Desc %d, TX Desc %d, mbufs/port %d, mbuf cache %d\n",
			DEFAULT_PKT_BURST, DEFAULT_RX_DESC, DEFAULT_TX_DESC, MAX_MBUFS_PER_PORT, MBUF_CACHE_SIZE);

	/* Configure and initialize the ports */
	pktgen_config_ports();

	pktgen_config_info();

	/* launch per-lcore init on every lcore except initial and initial + 1 lcores */
#if RTE_VERSION >= RTE_VERSION_NUM(20, 11, 0, 0)
	ret = rte_eal_mp_remote_launch(pktgen_launch_one_lcore, NULL, CALL_MAIN);
#else
	ret = rte_eal_mp_remote_launch(pktgen_launch_one_lcore, NULL, CALL_MASTER);
#endif
	if (ret != 0) {
		printf("Failed to start lcore, return %d\n", ret);
	}

	rte_delay_ms(250);	/* Wait for the lcores to start up. */

	/* Wait for all of the cores to stop running and exit. */
	rte_eal_mp_wait_lcore();

	RTE_ETH_FOREACH_DEV(i) {
		rte_eth_dev_stop(i);
		rte_delay_ms(100);
	}

	return 0;
}
