#pragma once

#include "DistributedTask.h"

class StructTask final : public DistributedTask {
public:
    StructTask();
    void execute(int rank, int size) const override;
};
