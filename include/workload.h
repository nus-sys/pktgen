#ifndef _WORKLOAD_H_
#define _WORKLOAD_H_

#include <stdbool.h>
#include <stdint.h>

#include "properties.h"

class Workload {
public:
    virtual void Init(const Properties &) = 0;
    virtual uint16_t GenerateNextReq(uint8_t *, int, int *) = 0;
    virtual uint16_t RecordReply(bool, uint8_t *) = 0;
    virtual void PrintResult(void) = 0;
    virtual ~Workload() { }
};

#endif  /* _WORKLOAD_H_ */