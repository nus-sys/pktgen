#include "pktgen.h"
#include "pktgen-info.h"
#include "pktgen-ip.h"
#include "pktgen-udp.h"

/**
 * Print UDP header information for debug purposes.
 *
 * @param udphdr pointer to the udp header in memory.
 */
void udp_debug_print(struct udphdr * udphdr) {
    printf("UDP header:\n");
    printf("+-------------------------------+\n");
    printf("|     %5u     |     %5u     | (src port, dest port)\n",
                            ntohs(udphdr->source), ntohs(udphdr->dest));
    printf("+-------------------------------+\n");
    printf("|     %5u     |     0x%04x    | (len, chksum)\n",
                            ntohs(udphdr->len), ntohs(udphdr->check));
    printf("+-------------------------------+\n");
}

void pktgen_udp_hdr_ctor(struct client * cl, uint8_t * pkt, int payload_size) {
    struct udphdr * udp = (struct udphdr *)(pkt + ETH_HLEN + sizeof(struct iphdr));
    uint16_t tlen;

    tlen = payload_size + sizeof(struct udphdr);
    udp->len = htons(tlen);
    udp->source = htons(cl->sport);
    udp->dest = htons(pkt_info.dport);

    udp->check = 0;

    // udp_debug_print(udp);
}
