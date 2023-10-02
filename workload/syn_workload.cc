#include "syn_workload.h"
#include "properties.h"

#include "core/const_generator.h"
#include "core/uniform_generator.h"
#include "core/bimodal_generator.h"

const std::string SynWorkload::SERVICE_TIME_DISTRIBUTION_PROPERTY = "service_time_dist";
const std::string SynWorkload::SERVICE_TIME_DISTRIBUTION_DEFAULT = "constant";

const std::string SynWorkload::SERVICE_TIME_ARG1_PROPERTY = "service_time_arg1";
const std::string SynWorkload::SERVICE_TIME_ARG1_DEFAULT = "0";

const std::string SynWorkload::SERVICE_TIME_ARG2_PROPERTY = "service_time_arg2";
const std::string SynWorkload::SERVICE_TIME_ARG2_DEFAULT = "1000";

inline void SynWorkload::Init(const Properties &p) {
    int service_time_arg1 = std::stod(p.GetProperty(SERVICE_TIME_ARG1_PROPERTY, 
                                            SERVICE_TIME_ARG1_DEFAULT));
    int service_time_arg2 = std::stod(p.GetProperty(SERVICE_TIME_ARG2_PROPERTY, 
                                            SERVICE_TIME_ARG2_DEFAULT));
    std::string service_time_dist = p.GetProperty(SERVICE_TIME_DISTRIBUTION_PROPERTY, 
                                                SERVICE_TIME_DISTRIBUTION_DEFAULT);

    if (service_time_dist == "constant") {
        service_time_generator_ = new ConstGenerator(service_time_arg1);
    } else if (service_time_dist == "uniform") {
        service_time_generator_ = new UniformGenerator(service_time_arg1, service_time_arg2);
    } else if (service_time_dist == "bimodal") {
        service_time_generator_ = new BimodalGenerator(service_time_arg1, 0.995, service_time_arg2, 0.005);
    } 
}