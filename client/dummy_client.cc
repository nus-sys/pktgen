#include "dummy_client.h"
#include "pktgen.h"

int dummy_client_send(Workload * wl, struct client *, uint8_t * pkt, int len) {
    wl->GenerateNextReq(pkt, len);
    return 0;
}

int dummy_client_recv(Workload * wl, struct client *, uint8_t * pkt, int len) {
    return 0;
}