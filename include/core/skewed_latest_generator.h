#ifndef _SKEWED_LATEST_GENERATOR_H_
#define _SKEWED_LATEST_GENERATOR_H_

#include <atomic>
#include <cstdint>

#include "core/generator.h"
#include "core/counter_generator.h"
#include "core/zipfian_generator.h"

class SkewedLatestGenerator : public Generator<uint64_t> {
public:
    SkewedLatestGenerator(CounterGenerator &counter) :
        basis_(counter), zipfian_(basis_.Last()) {
        Next();
    }
    
    uint64_t Next();
    uint64_t Last() { return last_; }
    std::string GetType() { return "Skewed latest distribution"; }
private:
    CounterGenerator &basis_;
    ZipfianGenerator zipfian_;
    std::atomic<uint64_t> last_;
};

inline uint64_t SkewedLatestGenerator::Next() {
    uint64_t max = basis_.Last();
    return last_ = max - zipfian_.Next(max);
}

#endif // _SKEWED_LATEST_GENERATOR_H_
