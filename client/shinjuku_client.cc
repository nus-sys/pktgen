#include "shinjuku_client.h"
#include "pktgen.h"

void shinjuku_client_init(void) {
    return;
}

int shinjuku_client_send(Workload * wl, struct client * cl, uint8_t * pkt, int len) {
    struct shinjuku_message * msg = (struct shinjuku_message *)pkt;
    msg->client_id = cl->sport;
    msg->req_id = cl->req_id++;
    msg->pkts_length = len;
    wl->GenerateNextReq(pkt + sizeof(struct shinjuku_message), len - sizeof(struct shinjuku_message));
    return 0;
}

int shinjuku_client_recv(Workload * wl, uint8_t * pkt, uint16_t len) {
    wl->RecordReply(pkt + sizeof(struct shinjuku_message));
    return 0;
}

void shinjuku_client_output(Workload * wl, uint64_t duration) {
    wl->PrintResult(duration);
    return;
}