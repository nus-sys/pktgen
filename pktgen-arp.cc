#include "pktgen.h"
#include "pktgen-info.h"
#include "pktgen-arp.h"

void pktgen_setup_arp(uint16_t lid, uint16_t pid, uint16_t qid) {
    struct mbuf_table * mtab = &core_info[lid].tx_mbufs[pid];
    struct rte_mbuf ** pkts = mtab->m_table;
    struct rte_mbuf * tx_pkt;
    uint8_t * pkt;
    int next_pkt;
    struct ethhdr * ethhdr;
    struct eth_arphdr * arphdr;

    if (unlikely(mtab->len == DEFAULT_PKT_BURST)) {
        return;
    }

    next_pkt = mtab->len;
    tx_pkt = pkts[next_pkt];

    tx_pkt->pkt_len = tx_pkt->data_len = sizeof(struct ethhdr) + sizeof(struct eth_arphdr);
    tx_pkt->nb_segs = 1;
    tx_pkt->next = NULL;

    mtab->len++;

    pkt = rte_pktmbuf_mtod(tx_pkt, uint8_t *);

    ethhdr = (struct ethhdr *)pkt;
    arphdr = (struct eth_arphdr *)(pkt + sizeof(struct ethhdr));

    ethhdr->h_proto = htons(ETH_P_ARP);
    ETHADDR_COPY(&ethhdr->h_dest, pkt_info.eth_dst_addr);
    ETHADDR_COPY(&ethhdr->h_source, pkt_info.eth_src_addr);

    arphdr->ar_op = htons(RTE_ARP_OP_REPLY);

    /* Write the ARP MAC-Addresses */
    ETHADDR_COPY(arphdr->ar_sha, pkt_info.eth_src_addr);
    ETHADDR_COPY(arphdr->ar_tha, pkt_info.eth_dst_addr);
    /* Copy struct ip4_addr2 to aligned ip4_addr, to support compilers without
    * structure packing. */
    arphdr->ar_sip = pkt_info.ip_src_addr;
    arphdr->ar_tip = pkt_info.ip_dst_addr;

    arphdr->ar_hrd = htons(1);
    arphdr->ar_pro = htons(ETH_P_IP);
    /* set hwlen and protolen */
    arphdr->ar_hln = 6;
    arphdr->ar_pln = sizeof(uint32_t);
}
