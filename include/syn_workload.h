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
    uint64_t send_start;
    uint64_t service_time;
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

        return 0;
    }

    void PrintResult(void) {
        uint64_t send_start, completion_time, elapsed;
        std::ofstream result; // outs is an output stream of iostream class

        result.open("latency-" + std::to_string(sched_getcpu()) + ".txt") ; // connect outs to file outFile

        for (int i = 0; i < nr_latency; i++) {
            send_start = latencies[nr_latency].send_start;
            completion_time = latencies[nr_latency].completion_time;
            elapsed = latencies[nr_latency].completion_time - latencies[nr_latency].send_start;
            result << send_start << "\t" << completion_time << "\t" << elapsed << "\n";
        }

        result.close() ;    // closing the output file stream
    }

    SynWorkload() :
        nr_latency(0), service_time_generator_(NULL) {
            latencies = (struct syn_ts *)calloc(131072, sizeof(struct syn_ts));
    }
    
    virtual ~SynWorkload() {
        if (service_time_generator_) delete service_time_generator_;
    }
  
protected:
    int nr_latency;
    struct syn_ts * latencies;
    Generator<uint64_t> *service_time_generator_;

private:
    uint64_t CurrentTime_nanoseconds() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
};

#endif  /* _SYN_WORKLOAD_H_ */