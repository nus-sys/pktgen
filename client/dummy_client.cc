#include "dummy_client.h"
#include "pktgen.h"

int dummy_client_send(Workload * wl, struct client *, uint8_t * pkt, int len) {
    wl->GenerateNextReq(pkt, len);
    return 0;
}

int dummy_client_recv(Workload * wl, uint8_t * pkt, uint16_t len) {
    return 0;
}