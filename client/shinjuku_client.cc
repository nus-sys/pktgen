#include "shinjuku_client.h"
#include "pktgen.h"

int shinjuku_client_send(Workload * wl, struct client * cl, uint8_t * pkt, int len) {
    struct shinjuku_message * msg = (struct shinjuku_message *)pkt;
    msg->client_id = cl->sport;
    msg->req_id = cl->req_id++;
    msg->pkts_length = len;
    wl->GenerateNextReq(pkt + sizeof(struct shinjuku_message), len - sizeof(struct shinjuku_message));
    return 0;
}

int shinjuku_client_recv(Workload * wl, struct client * cl, uint8_t * pkt, int len) {
    return 0;
}