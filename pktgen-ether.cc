#include <string.h>

#include "pktgen-info.h"
#include "pktgen-ether.h"

void pktgen_ether_hdr_ctor(uint8_t * pkt) {
    struct ethhdr * eth = (struct ethhdr *)pkt;

    /* src and dest addr */
    memcpy(&eth->h_source, &pkt_info.eth_src_addr, ETH_ALEN);
    memcpy(&eth->h_dest, &pkt_info.eth_dst_addr, ETH_ALEN);

    /* normal ethernet header */
    eth->h_proto    = htons(ETH_P_IP);
}