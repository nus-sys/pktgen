#include "dummy_client.h"
#include "pktgen.h"

void dummy_client_init(void) {
    return;
}

int dummy_client_send(Workload * wl, struct client *, uint8_t * pkt, int len) {
    wl->GenerateNextReq(pkt, len);
    return 0;
}

int dummy_client_recv(Workload * wl, uint8_t * pkt, uint16_t len) {
    return 0;
}

void dummy_client_output(Workload * wl, uint64_t duration) {
    return;
}