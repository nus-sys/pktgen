#ifndef _PKVS_CLIENT_H_
#define _PKVS_CLIENT_H_

#include "client.h"

struct pkvs_message {
    uint32_t req_id;
    uint16_t op_code;
    uint64_t resp1;
    uint64_t resp2;
} __attribute__((__packed__));

int pkvs_client_send(Workload *, struct client *, uint8_t *, int);
int pkvs_client_recv(Workload *, struct client *, uint8_t *, int);

static const struct client_operations pkvs_ops = {
    .send   = pkvs_client_send,
    .recv   = pkvs_client_recv,
};

#endif  /* _PKVS_CLIENT_H_ */