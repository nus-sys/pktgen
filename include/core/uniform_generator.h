#ifndef _UNIFORM_GENERATOR_H_
#define _UNIFORM_GENERATOR_H_

#include "core/generator.h"

#include <atomic>
#include <random>

class UniformGenerator : public Generator<uint64_t> {
public:
    // Both min and max are inclusive
    UniformGenerator(uint64_t min, uint64_t max) : dist_(min, max) {
        generator_.seed(rand());
        Next(); 
    }
    
    uint64_t Next();
    uint64_t Last();
  
private:
    std::mt19937_64 generator_;
    std::uniform_int_distribution<uint64_t> dist_;
    uint64_t last_int_;
};

inline uint64_t UniformGenerator::Next() {
    return last_int_ = dist_(generator_);
}

inline uint64_t UniformGenerator::Last() {
    return last_int_;
}

#endif // YCSB_C_UNIFORM_GENERATOR_H_
