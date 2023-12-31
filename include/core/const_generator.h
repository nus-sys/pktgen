#ifndef _CONST_GENERATOR_H_
#define _CONST_GENERATOR_H_

#include "core/generator.h"
#include <cstdint>

class ConstGenerator : public Generator<uint64_t> {
public:
    ConstGenerator(int constant) : constant_(constant) { }
    uint64_t Next() { return constant_; }
    uint64_t Last() { return constant_; }
private:
    uint64_t constant_;
};

#endif // _CONST_GENERATOR_H_
