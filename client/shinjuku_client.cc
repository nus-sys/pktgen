#include "shinjuku_client.h"
#include "pktgen.h"

void shinjuku_client_init(void) {
    return;
}

int shinjuku_client_send(Workload * wl, struct client * cl, uint8_t * pkt, int max_len, int * len) {
    struct shinjuku_message * msg = (struct shinjuku_message *)pkt;
    msg->client_id = cl->sport;
    msg->req_id = cl->req_id++;
    wl->GenerateNextReq(pkt + sizeof(struct shinjuku_message), max_len - sizeof(struct shinjuku_message), len);
    msg->pkts_length = max_len;
    return 0;
}

int shinjuku_client_recv(Workload * wl, uint8_t * pkt, uint16_t len) {
    wl->RecordReply(start_lat_record, pkt + sizeof(struct shinjuku_message));
    return 0;
}

void shinjuku_client_output(Workload * wl, uint64_t duration) {
    wl->PrintResult();
    return;
}