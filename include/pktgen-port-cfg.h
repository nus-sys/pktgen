#ifndef _PKTGEN_PORT_CFG_H_
#define _PKTGEN_PORT_CFG_H_

#include <stdio.h>
#include <string.h>
#include <rte_version.h>
#include <rte_atomic.h>
#include <rte_spinlock.h>
#include <rte_pci.h>

#include "pktgen-constants.h"

struct mbuf_table {
	uint16_t len;
	struct rte_mbuf *m_table[DEFAULT_PKT_BURST];
};

void pktgen_config_ports(void);

#endif  /* _PKTGEN_PORT_CFG_H_ */