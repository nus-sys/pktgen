#ifndef _PKVS_CLIENT_H_
#define _PKVS_CLIENT_H_

#include "client.h"

struct pkvs_message {
    uint8_t magic;
    uint8_t op_code;
    uint32_t req_id;
    uint64_t resp;
    uint64_t tsc;
} __attribute__((__packed__));

void pkvs_client_init(void);
int pkvs_client_send(Workload *, struct client *, uint8_t *, int);
int pkvs_client_recv(Workload *, uint8_t *, uint16_t);
void pkvs_client_output(Workload *);

static const struct client_operations pkvs_ops = {
    .init   = pkvs_client_init,
    .send   = pkvs_client_send,
    .recv   = pkvs_client_recv,
    .output = pkvs_client_output,
};

#endif  /* _PKVS_CLIENT_H_ */