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

void shinjuku_client_init(void);
int shinjuku_client_send(Workload *, struct client *, uint8_t *, int, int *);
int shinjuku_client_recv(Workload *, uint8_t *, uint16_t);
void shinjuku_client_output(Workload *, uint64_t);

static const struct client_operations shinjuku_ops = {
    .init   = shinjuku_client_init,
    .send   = shinjuku_client_send,
    .recv   = shinjuku_client_recv,
    .output = shinjuku_client_output,
};

#endif  /* _SHINJUKU_CLIENT_H_ */