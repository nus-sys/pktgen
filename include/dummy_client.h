#ifndef _DUMMY_CLIENT_H_
#define _DUMMY_CLIENT_H_

#include "client.h"

void dummy_client_init(void);
int dummy_client_send(Workload *, struct client *, uint8_t *, int);
int dummy_client_recv(Workload *, uint8_t *, uint16_t);
void dummy_client_output(Workload *, uint64_t);

static const struct client_operations dummy_ops = {
    .init   = dummy_client_init,
    .send   = dummy_client_send,
    .recv   = dummy_client_recv,
    .output = dummy_client_output,
};

#endif  /* _DUMMY_CLIENT_H_ */