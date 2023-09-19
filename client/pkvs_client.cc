#include "pkvs_client.h"
#include "pktgen.h"

struct pkvs_ts {
    uint8_t op_code;
	uint64_t send_start;        /* In CPU cycle */
	uint64_t completion_time;   /* In CPU cycle */
};

__thread int pkvs_nr_latency = 0;
__thread struct pkvs_ts * pkvs_latencies;

static uint64_t rdtsc(void){
    uint64_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

void pkvs_client_init(void) {
    pkvs_latencies = (struct pkvs_ts *)calloc(131072, sizeof(struct pkvs_ts));
}

int pkvs_client_send(Workload * wl, struct client * cl, uint8_t * pkt, int len) {
    struct pkvs_message * msg = (struct pkvs_message *)pkt;
    msg->magic = 0x66;
    msg->op_code = (uint8_t)(wl->GenerateNextReq(pkt + sizeof(struct pkvs_message), len - sizeof(struct pkvs_message)));
    msg->req_id = 0x12345678;
    msg->tsc = rdtsc();
    return 0;
}

int pkvs_client_recv(Workload * wl, uint8_t * pkt, uint16_t len) {
    struct pkvs_message * msg;
    msg = (struct pkvs_message *)pkt;

    if (pkvs_nr_latency < 131072) {
        pkvs_latencies[pkvs_nr_latency].op_code = msg->op_code;
        pkvs_latencies[pkvs_nr_latency].send_start = msg->tsc;
        pkvs_latencies[pkvs_nr_latency].completion_time = rdtsc();
        pkvs_nr_latency++;
    }

    return 0;
}

void pkvs_client_output(Workload * wl) {
    uint8_t op_code;
    uint64_t send_start, completion_time, elapsed;
    std::ofstream result; // outs is an output stream of iostream class

    result.open("latency-" + std::to_string(sched_getcpu()) + ".txt") ; // connect outs to file outFile

    for (int i = 0; i < pkvs_nr_latency; i++) {
        op_code = pkvs_latencies[i].op_code;
        send_start = pkvs_latencies[i].send_start;
        completion_time = pkvs_latencies[i].completion_time;
        elapsed = pkvs_latencies[i].completion_time - pkvs_latencies[i].send_start;
        switch (op_code) {
            case READ:
                result << "READ\t";
                break;
            case UPDATE:
                result << "UPDATE\t";
                break;
            case INSERT:
                result << "INSERT\t";
                break;
            case SCAN:
                result << "SCAN\t";
                break;
            case READMODIFYWRITE:
                result << "READMODIFYWRITE\t";
                break;
        }
        result << send_start << "\t" << completion_time << "\t" << elapsed << "\n";
    }

    result.close() ;    // closing the output file stream

    return;
}