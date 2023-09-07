#include "pktgen.h"
#include "pktgen-info.h"
#include "pktgen-ip.h"

void ip4_debug_print(struct iphdr * iphdr) {
    printf("IP header:\n");
    printf("+-------------------------------+\n");
    printf("|%2u |%2u |  0x%02u |     %5u     | (v, hl, tos, len)\n",
            (uint16_t)IPH_V(iphdr), (uint16_t)IPH_HL(iphdr), (uint16_t)IPH_TOS(iphdr), ntohs(IPH_LEN(iphdr)));
    printf("+-------------------------------+\n");
    printf("|    %5u      |%u%u%u|    %4u   | (id, flags, offset)\n", 
                        ntohs(IPH_ID(iphdr)),
                        (uint16_t)((iphdr->frag_off) >> 15 & 1),
                        (uint16_t)((iphdr->frag_off) >> 14 & 1),
                        (uint16_t)((iphdr->frag_off) >> 13 & 1), IPH_OFFSET(iphdr));
    printf("+-------------------------------+\n");
    printf("|  %3u  |  %3u  |    0x%04u     | (ttl, proto, chksum)\n", (uint16_t)IPH_TTL(iphdr), (uint16_t)IPH_PROTO(iphdr), ntohs(IPH_CHKSUM(iphdr)));
    printf("+-------------------------------+\n");
    printf("|  %3u  .  %3u  .  %3u  .  %3u  | (src)\n", NET_IP_FMT(iphdr->saddr));
    printf("+-------------------------------+\n");
    printf("|  %3u  .  %3u  .  %3u  .  %3u  | (dest)\n", NET_IP_FMT(iphdr->daddr));
    printf("+-------------------------------+\n");
}

void pktgen_ipv4_ctor(uint8_t * pkt, int payload_size) {
    struct iphdr * ip = (struct iphdr *)(pkt + ETH_HLEN);
    uint16_t tlen;

    /* IPv4 Header constructor */
    tlen = payload_size + sizeof(struct udphdr) + sizeof(struct iphdr);

    /* Zero out the header space */
    memset((char *)ip, 0, sizeof(struct iphdr));

    ip->version = IPv4_VERSION;
    ip->ihl = sizeof(struct iphdr) / 4;

    ip->tot_len = htons(tlen);
    ip->ttl = DEFAULT_TTL;
    ip->tos = DEFAULT_TOS;

    pktgen.ident    += 27; /* bump by a prime number */
    ip->id  = htons(pktgen.ident);
    ip->frag_off    = 0;
    ip->protocol    = IPPROTO_UDP;
    ip->saddr   = pkt_info.ip_src_addr;
    ip->daddr   = pkt_info.ip_dst_addr;
    // ip->check   = rte_ipv4_cksum((const rte_ipv4_hdr *)ip);

    // ip4_debug_print(ip);
}