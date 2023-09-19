#include <iomanip>

#include "pkvs_client.h"
#include "pktgen.h"

struct pkvs_ts {
    uint8_t op_code;
	uint64_t send_start;        /* In CPU cycle */
	uint64_t completion_time;   /* In CPU cycle */
};

__thread uint64_t pkvs_nb_rx = 0;
__thread uint64_t pkvs_nb_tx = 0;
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
    msg->magic = 0x65;
    msg->op_code = (uint8_t)(wl->GenerateNextReq(pkt + sizeof(struct pkvs_message), len - sizeof(struct pkvs_message)));
    msg->req_id = 0x12345678;
    msg->tsc = rdtsc();

    pkvs_nb_tx++;

    return 0;
}

int pkvs_client_recv(Workload * wl, uint8_t * pkt, uint16_t len) {
    struct pkvs_message * msg;
    msg = (struct pkvs_message *)pkt;

    pkvs_nb_rx++;

    if (/*start_lat_record && */pkvs_nr_latency < 131072) {
        pkvs_latencies[pkvs_nr_latency].op_code = msg->op_code;
        pkvs_latencies[pkvs_nr_latency].send_start = msg->tsc;
        pkvs_latencies[pkvs_nr_latency].completion_time = rdtsc();
        pkvs_nr_latency++;
    }

    return 0;
}

void pkvs_client_output(Workload * wl, uint64_t duration) {
    uint8_t op_code;
    uint64_t send_start, completion_time, elapsed;
    std::ofstream lat_result; // outs is an output stream of iostream class
    FILE * thp_result;
    char name[32];

    lat_result.open("latency-" + std::to_string(sched_getcpu()) + ".txt") ; // connect outs to file outFile

    for (int i = 0; i < pkvs_nr_latency; i++) {
        op_code = pkvs_latencies[i].op_code;
        send_start = pkvs_latencies[i].send_start;
        completion_time = pkvs_latencies[i].completion_time;
        elapsed = pkvs_latencies[i].completion_time - pkvs_latencies[i].send_start;
        lat_result << std::to_string(op_code) << "\t" << send_start << "\t" << completion_time << "\t" << elapsed << "\n";
    }

    lat_result.close();

    sprintf(name, "thp-%d.txt", sched_getcpu());
    thp_result = fopen(name, "w");
    if (!thp_result) {
        printf("Error!\n");
    }

    fprintf(thp_result, "%lu\t%.4f\t%lu\t%.4f\n", pkvs_nb_rx, ((float)pkvs_nb_rx) / duration, pkvs_nb_tx, ((float)pkvs_nb_tx) / duration);
    fclose(thp_result);

    printf("CPU %02d| Duration: %lu s, Rx: %lu, Rx rate: %.4f (Mpps), Tx: %lu, Tx rate: %.4f (Mpps)\n", 
            sched_getcpu(), duration / 1000000, pkvs_nb_rx, 
            ((float)pkvs_nb_rx) / duration, pkvs_nb_tx, ((float)pkvs_nb_tx) / duration);
    return;
}