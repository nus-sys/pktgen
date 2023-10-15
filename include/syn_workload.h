#ifndef _SYN_WORKLOAD_H_
#define _SYN_WORKLOAD_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <string.h>

#include <vector>
#include <string>
#include <chrono>

#include "properties.h"
#include "core/generator.h"
#include "core/discrete_generator.h"
#include "core/counter_generator.h"
#include "workload.h"
#include "utils.h"

struct syn_message {
    uint64_t service_time;
    uint64_t send_start;
} __attribute__((__packed__));

struct syn_ts {
	uint64_t send_start;
	uint64_t completion_time;
};

class SynWorkload : public Workload {
public:
    static const std::string SERVICE_TIME_DISTRIBUTION_PROPERTY;
    static const std::string SERVICE_TIME_DISTRIBUTION_DEFAULT;

    static const std::string SERVICE_TIME_ARG1_PROPERTY;
    static const std::string SERVICE_TIME_ARG1_DEFAULT;

    static const std::string SERVICE_TIME_ARG2_PROPERTY;
    static const std::string SERVICE_TIME_ARG2_DEFAULT;

    ///
    /// Initialize the scenario.
    /// Called once, in the main client thread, before any operations are started.
    ///
    virtual void Init(const Properties &p);

    virtual size_t NextServiceTime() { return service_time_generator_->Next(); }

    uint16_t GenerateNextReq(uint8_t * buf, int len) {
        struct syn_message * msg;
        msg = (struct syn_message *)buf;

        msg->send_start = this->CurrentTime_nanoseconds();
        msg->service_time = this->NextServiceTime();

        nr_tx++;

        // printf("Send start: %lu, service time: %lu\n", msg->send_start, msg->service_time);

        return 0;
    }

    uint16_t RecordReply(uint8_t * buf) {
        struct syn_message * msg;
        msg = (struct syn_message *)buf;

        if (nr_latency < 131072) {
            latencies[nr_latency].send_start = msg->send_start;
            latencies[nr_latency].completion_time = CurrentTime_nanoseconds();
            nr_latency++;
        }

        nr_rx++;
        if (msg->service_time < 15000) {
            nr_rx_1++;
        } else {
            nr_rx_2++;
        }

        return 0;
    }

    void PrintResult(uint64_t duration) {
        uint64_t send_start, completion_time, elapsed;
        std::ofstream result; // outs is an output stream of iostream class
        char name[32];
        FILE * thp_result;

        result.open("latency-" + std::to_string(sched_getcpu()) + ".txt") ; // connect outs to file outFile

        for (int i = 0; i < nr_latency; i++) {
            send_start = latencies[i].send_start;
            completion_time = latencies[i].completion_time;
            elapsed = latencies[i].completion_time - latencies[i].send_start;
            result << send_start << "\t" << completion_time << "\t" << elapsed << "\n";
        }

        result.close() ;    // closing the output file stream

        sprintf(name, "thp-%d.txt", sched_getcpu());
        thp_result = fopen(name, "w");
        if (!thp_result) {
            printf("Error!\n");
        }

        fprintf(thp_result, "%lu\t%.4f\t%lu\t%.4f\t%lu\t%.4f\t%lu\t%.4f\n", 
                nr_rx, ((float)nr_rx) / duration, 
                nr_rx_1, ((float)nr_rx_1) / duration, 
                nr_rx_2, ((float)nr_rx_2) / duration, 
                nr_tx, ((float)nr_tx) / duration);
        fclose(thp_result);

        printf("CPU %02d| Duration: %lu s, Rx: %lu, Rx rate: %.4f (Mpps), Tx: %lu, Tx rate: %.4f (Mpps)\n", 
                sched_getcpu(), duration / 1000000, nr_rx, ((float)nr_rx) / duration, nr_tx, ((float)nr_tx) / duration);
    }

    SynWorkload() :
        nr_latency(0), service_time_generator_(NULL), nr_rx(0), nr_tx(0), nr_rx_1(0), nr_rx_2(0) {
            latencies = (struct syn_ts *)calloc(131072, sizeof(struct syn_ts));
    }
    
    virtual ~SynWorkload() {
        if (service_time_generator_) delete service_time_generator_;
    }
  
protected:
    int nr_latency;
    struct syn_ts * latencies;
    Generator<uint64_t> *service_time_generator_;
    uint64_t nr_rx;
    uint64_t nr_rx_1;
    uint64_t nr_rx_2;
    uint64_t nr_tx;

private:
    uint64_t CurrentTime_nanoseconds() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
};

#endif  /* _SYN_WORKLOAD_H_ */