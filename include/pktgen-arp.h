#ifndef _PKTGEN_ARP_H_
#define _PKTGEN_ARP_H_

#include <stdint.h>
#include <netinet/in.h>
#include <net/ethernet.h>

/** the ARP message, see RFC 826 ("Packet format") */
struct eth_arphdr {
    uint16_t ar_hrd;            /* Format of hardware address.  */
    uint16_t ar_pro;            /* Format of protocol address.  */
    uint8_t ar_hln;             /* Length of hardware address.  */
    uint8_t ar_pln;             /* Length of protocol address.  */
    uint16_t ar_op;             /* ARP opcode (command).  */
    uint8_t ar_sha[ETH_ALEN];   /* Sender hardware address.  */
    uint32_t ar_sip;            /* Sender IP address.  */
    uint8_t ar_tha[ETH_ALEN];   /* Target hardware address.  */
    uint32_t ar_tip;            /* Target IP address.  */
} __attribute__ ((__packed__));

#define ETHADDR_COPY(dst, src)  memcpy(dst, src, ETH_ALEN)

void pktgen_setup_arp(uint16_t lid, uint16_t pid, uint16_t qid);

#endif  /* _PKTGEN_ARP_H_ */