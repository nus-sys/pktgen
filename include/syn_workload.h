#ifndef _SYN_WORKLOAD_H_
#define _SYN_WORKLOAD_H_

#include <vector>
#include <string>
#include <chrono>

#include <string.h>

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

        printf("Send start: %lu, service time: %lu\n", msg->send_start, msg->service_time);

        return 0;
    }

    SynWorkload() :
        service_time_generator_(NULL) {
    }
    
    virtual ~SynWorkload() {
        if (service_time_generator_) delete service_time_generator_;
    }
  
protected:
    Generator<uint64_t> *service_time_generator_;

private:
    uint64_t CurrentTime_nanoseconds() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
};

#endif  /* _SYN_WORKLOAD_H_ */