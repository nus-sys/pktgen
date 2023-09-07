#include "pkvs_client.h"
#include "pktgen.h"

int pkvs_client_send(Workload * wl, struct client * cl, uint8_t * pkt, int len) {
    struct pkvs_message * msg = (struct pkvs_message *)pkt;
    msg->op_code = wl->GenerateNextReq(pkt + sizeof(struct pkvs_message), len -  + sizeof(struct pkvs_message));
    return 0;
}

int pkvs_client_recv(Workload * wl, struct client * cl, uint8_t * pkt, int len) {
    return 0;
}