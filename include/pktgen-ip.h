#ifndef _PKTGEN_IP_H_
#define _PKTGEN_IP_H_

#include <stdint.h>
#include <string.h>
#include <rte_ip.h>
#include <linux/if_ether.h>

#define LE_IP_FMT(ip)   ((uint8_t *)&(ip))[3], \
					    ((uint8_t *)&(ip))[2], \
 					    ((uint8_t *)&(ip))[1], \
				        ((uint8_t *)&(ip))[0]

#define BE_IP_FMT(ip)   ((uint8_t *)&(ip))[0], \
					    ((uint8_t *)&(ip))[1], \
 					    ((uint8_t *)&(ip))[2], \
				        ((uint8_t *)&(ip))[3]

#if __BYTE_ORDER == __LITTLE_ENDIAN
#	define HOST_IP_FMT(ip)	LE_IP_FMT(ip)
#elif __BYTE_ORDER == __BIG_ENDIAN
#	define HOST_IP_FMT(ip)	BE_IP_FMT(ip)
#endif

#define NET_IP_FMT(ip)	BE_IP_FMT(ip)

#define IPv4_VERSION    4

/* Macros to get struct ip_hdr fields: */
#define IPH_V(hdr)  ((hdr)->version)
#define IPH_HL(hdr) ((hdr)->ihl)
#define IPH_TOS(hdr) ((hdr)->tos)
#define IPH_LEN(hdr) ((hdr)->tot_len)
#define IPH_ID(hdr) ((hdr)->id)
#define IPH_OFFSET(hdr) ((hdr)->frag_off)
#define IPH_TTL(hdr) ((hdr)->ttl)
#define IPH_PROTO(hdr) ((hdr)->protocol)
#define IPH_CHKSUM(hdr) ((hdr)->check)

void pktgen_ipv4_ctor(uint8_t * pkt, int tot_size);

#endif  /* _PKTGEN_IP_H_ */