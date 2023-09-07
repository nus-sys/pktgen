#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>

#include "core/generator.h"
#include "workload.h"

struct client {
    uint16_t req_id;	/* < Request ID */
    uint16_t sport;		/* < Source port number */
	uint64_t last_send;	/* < Last send (in nanosecond) */
	uint64_t interval;	/* < Send interval */
	Generator<uint64_t> * arrival;	/* < Generator the arrival rate */
};

struct client_operations {
	int (*send) (Workload *, struct client *, uint8_t *, int);
	int (*recv) (Workload *, uint8_t *, uint16_t);
};

#endif  /* _CLIENT_H_ */