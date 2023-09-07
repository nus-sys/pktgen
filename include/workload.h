#ifndef _WORKLOAD_H_
#define _WORKLOAD_H_

#include <stdint.h>

#include "properties.h"

class Workload {
public:
    virtual void Init(const Properties &) = 0;
    virtual uint16_t GenerateNextReq(uint8_t *, int) = 0;
    virtual ~Workload() { }
};

#endif  /* _WORKLOAD_H_ */