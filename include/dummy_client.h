#ifndef _DUMMY_CLIENT_H_
#define _DUMMY_CLIENT_H_

#include "client.h"

int dummy_client_send(Workload *, struct client *, uint8_t *, int);
int dummy_client_recv(Workload *, uint8_t *, uint16_t);

static const struct client_operations dummy_ops = {
    .send   = dummy_client_send,
    .recv   = dummy_client_recv,
};

#endif  /* _DUMMY_CLIENT_H_ */