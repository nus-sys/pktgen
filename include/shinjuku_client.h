#ifndef _SHINJUKU_CLIENT_H_
#define _SHINJUKU_CLIENT_H_

#include "client.h"

struct shinjuku_message {
    uint16_t type;
    uint16_t seq_num;
    uint32_t queue_length[3];
    uint16_t client_id;
    uint32_t req_id;
    uint32_t pkts_length;
} __attribute__((__packed__));

int shinjuku_client_send(Workload *, struct client *, uint8_t *, int);
int shinjuku_client_recv(Workload *, struct client *, uint8_t *, int);

static const struct client_operations shinjuku_ops = {
    .send   = shinjuku_client_send,
    .recv   = shinjuku_client_recv,
};

#endif  /* _SHINJUKU_CLIENT_H_ */