#pragma once

#include "DistributedTask.h"

class SumTask final : public DistributedTask {
public:
    SumTask();
    void execute(int rank, int size) const override;
};
