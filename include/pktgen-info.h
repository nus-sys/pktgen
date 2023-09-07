#ifndef _PKTGEN_SEQ_H_
#define _PKTGEN_SEQ_H_

#include <net/ethernet.h>

typedef struct pkt_info_s {
	uint8_t eth_src_addr[ETH_ALEN];	/**< Source Ethernet address */
	uint8_t eth_dst_addr[ETH_ALEN];	/**< Destination Ethernet address */

	uint32_t ip_src_addr;	/**< Source IPv4 address in network bytes order */
	uint32_t ip_dst_addr;	/**< Destination IPv4 address in network bytes order */

	uint16_t dport;			/**< Destination port */
} pkt_info_t;

extern pkt_info_t pkt_info;

#endif  /* _PKTGEN_SEQ_H_ */