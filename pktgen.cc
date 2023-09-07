#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <chrono>

#include "core/exponential_generator.h"
#include "pktgen.h"
#include "pktgen-info.h"
#include "pktgen-ip.h"
#include "pktgen-udp.h"
#include "pktgen-ether.h"

pktgen_t pktgen;
pkt_info_t pkt_info;
core_info_t core_info[NR_MAX_CPU];

static inline uint64_t CurrentTime_nanoseconds() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void pktgen_init_core(uint16_t lid) {
    core_info_t * info = &core_info[lid];
    struct client * cl;
	std::string wl;

    printf("CPU %02d | initializing...\n", lid);
    info->nb_client = pktgen.nb_client;
    info->client_ops = pktgen.client_ops;

    wl = pktgen.props.GetProperty("workload", "synthetic");
	if (wl == "rocksdb") {
		info->wl = new DBWorkload();
	} else if (wl == "synthetic") {
		info->wl = new SynWorkload();
	} else {
		std::cout << "Unknown workload: " << wl << std::endl;
	}

    info->wl->Init(pktgen.props);

    for (int i = 0; i < info->nb_client; i++) {
        cl = &info->clients[i];
        cl->sport = (lid << 8) | (i + 1);
        cl->last_send = CurrentTime_nanoseconds();
        cl->arrival = new ExponentialGenerator(pktgen.nb_core * pktgen.nb_client * 1.0e6 / pktgen.tx_rate);
        cl->interval = cl->arrival->Next();
    }
}

static void pktgen_setup_packets(uint16_t pid, uint16_t lid, uint16_t qid, struct client * cl, int payload_len) {
    struct mbuf_table * mtab = &core_info[lid].tx_mbufs[pid];
    struct rte_mbuf ** pkts = mtab->m_table;
    struct rte_mbuf * tx_pkt;
    uint8_t * pkt;
    int next_pkt;
    int pkt_size;

    if (unlikely(mtab->len == DEFAULT_PKT_BURST)) {
        return;
    }

    next_pkt = mtab->len;
    tx_pkt = pkts[next_pkt];
    pkt_size = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len;

    tx_pkt->nb_segs = 1;
    tx_pkt->next = NULL;
    
    mtab->len++;

    tx_pkt->l2_len = ETH_HLEN;
    tx_pkt->l3_len = sizeof(struct iphdr);
    tx_pkt->l4_len = sizeof(struct udphdr);

#if RTE_VERSION_NUM >= RTE_VERSION_NUM(21, 11, 0, 0)
    tx_pkt->ol_flags = RTE_MBUF_F_TX_UDP_CKSUM | RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_IPV4;
#else if RTE_VERSION_NUM >= RTE_VERSION_NUM(20, 11, 0, 0)
    tx_pkt->ol_flags = DEV_TX_OFFLOAD_UDP_CKSUM | DEV_TX_OFFLOAD_IPV4_CKSUM;
#endif

    pkt = rte_pktmbuf_mtod(tx_pkt, uint8_t *);
    memset(pkt, 0, pkt_size);

    /* Fill payload */
    core_info[lid].client_ops->send(core_info[lid].wl, cl, pkt, payload_len);

    tx_pkt->pkt_len = tx_pkt->data_len = pkt_size;

    /* Construct the UDP header */
    pktgen_udp_hdr_ctor(cl, pkt, payload_len);

    /* IPv4 Header constructor */
    pktgen_ipv4_ctor(pkt, payload_len);

    pktgen_ether_hdr_ctor(pkt);

    return;
}

void pktgen_main_transmit(uint16_t pid, uint16_t lid, uint16_t qid) {
    struct mbuf_table * mtab = &core_info[lid].tx_mbufs[pid];
    struct rte_mbuf ** pkts = mtab->m_table;
    int pkt_cnt = mtab->len;

    if (pkt_cnt > 0) {
        int ret;
        do {
            /* Send packets until there is none in TX queue */
            ret = rte_eth_tx_burst(pid, qid, pkts, pkt_cnt);
            pkts += ret;
            pkt_cnt -= ret;
        } while (pkt_cnt > 0);

        /* Allocate new packet memory buffer for TX queue (WHY NEED NEW BUFFER??) */
        for (int i = 0; i < mtab->len; i++) {
            /* Allocate new buffer for sended packets */
            mtab->m_table[i] = rte_pktmbuf_alloc(core_info[lid].pkt_mempool);
            if (unlikely(mtab->m_table[i] == NULL)) {
                rte_exit(EXIT_FAILURE, "Failed to allocate %d:wmbuf[%d] on device %d!\n", rte_lcore_id(), i, pid);
            }
        }

        mtab->len = 0;
    }
}

int pktgen_main_receive(uint16_t pid, uint16_t lid, uint16_t qid, struct rte_mbuf * pkts_burst[], uint16_t nb_pkts) {
    uint16_t nb_rx;

    /*
     * Read packet from RX queues and free the mbufs
     */
    if ((nb_rx = rte_eth_rx_burst(pid, qid, pkts_burst, nb_pkts)) == 0) {
        return nb_rx;
    }

    rte_pktmbuf_free_bulk(pkts_burst, nb_rx);

    return nb_rx;
}

int pktgen_launch_one_lcore(void * arg __rte_unused) {
    int i;
    uint16_t lid, qid;
    struct rte_mbuf * pkts_burst[DEFAULT_PKT_BURST];
    struct timeval curr;
    int payload_len;
    uint64_t curr_tsc;
    struct client * cl;
    core_info_t * info;

    lid = rte_lcore_id();
    qid = lid;
    info = &core_info[lid];

    pktgen_init_core(lid);

    gettimeofday(&core_info[lid].start, NULL);

    while (true) {
        gettimeofday(&curr, NULL);
        if (curr.tv_sec > core_info[lid].start.tv_sec + 20) {
            break;
        }

    	RTE_ETH_FOREACH_DEV(i) {
            pktgen_main_receive(i, lid, qid, pkts_burst, DEFAULT_PKT_BURST);

            payload_len = std::stod(pktgen.props.GetProperty("payload_len", "64"));

            curr_tsc = CurrentTime_nanoseconds();

            for (int i = 0; i < info->nb_client; i++) {
                cl = &info->clients[i];
                /* Determine when is the next time to send packets */
                if (curr_tsc >= cl->last_send + cl->interval) {
                    cl->last_send = curr_tsc;
                    cl->interval = cl->arrival->Next();
                    // printf("CPU %02d | send via port: %d, qid: %d, payload len: %u, current: %lu, last send: %lu, interval: %lu\n", 
                    //         lid, i, qid, payload_len, curr_tsc, cl->last_send, cl->interval);
                    pktgen_setup_packets(i, lid, qid, cl, payload_len);
                }
            }

            pktgen_main_transmit(i, lid, qid);
        }
    }

    return 0;
}