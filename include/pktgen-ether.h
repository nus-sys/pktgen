#ifndef _PKTGEN_ETHER_H_
#define _PKTGEN_ETHER_H_

#include <netinet/in.h>
#include <net/ethernet.h>

#define ETH_ALEN	6

void pktgen_ether_hdr_ctor(uint8_t * pkt);

#endif  /* _PKTGEN_ETHER_H_ */