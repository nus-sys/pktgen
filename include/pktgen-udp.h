#ifndef _PKTGEN_UDP_H_
#define _PKTGEN_UDP_H_

#include <stdint.h>
#include <rte_ip.h>
#include <rte_udp.h>

void pktgen_udp_hdr_ctor(struct client * cl, uint8_t * pkt, int tot_size);

#endif  /* _PKTGEN_UDP_H_ */