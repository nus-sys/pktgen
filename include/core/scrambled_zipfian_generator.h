#ifndef _SCRAMBLED_ZIPFIAN_GENERATOR_H_
#define _SCRAMBLED_ZIPFIAN_GENERATOR_H_

#include <atomic>
#include <cstdint>

#include "utils.h"
#include "core/generator.h"
#include "core/zipfian_generator.h"

class ScrambledZipfianGenerator : public Generator<uint64_t> {
public:
    ScrambledZipfianGenerator(uint64_t min, uint64_t max,
        double zipfian_const = ZipfianGenerator::kZipfianConst) :
        base_(min), num_items_(max - min + 1),
        generator_(min, max, zipfian_const) { }
    
    ScrambledZipfianGenerator(uint64_t num_items) :
        ScrambledZipfianGenerator(0, num_items - 1) { }
    
    uint64_t Next();
    uint64_t Last();
    std::string GetType() { return "Scrambled zipfian distribution"; }
    
private:
    const uint64_t base_;
    const uint64_t num_items_;
    ZipfianGenerator generator_;

    uint64_t Scramble(uint64_t value) const;
};

inline uint64_t ScrambledZipfianGenerator::Scramble(uint64_t value) const {
    return base_ + FNVHash64(value) % num_items_;
}

inline uint64_t ScrambledZipfianGenerator::Next() {
    return Scramble(generator_.Next());
}

inline uint64_t ScrambledZipfianGenerator::Last() {
    return Scramble(generator_.Last());
}

#endif // _SCRAMBLED_ZIPFIAN_GENERATOR_H_
