#pragma once

#include "DistributedTask.h"

class FilterTask final : public DistributedTask {
public:
    FilterTask();
    void execute(int rank, int size) const override;
};
