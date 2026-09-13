#include "SumTask.h"

#include <mpi.h>

#include <iostream>
#include <vector>

namespace {

const int kElementCount = 10000;

// SumTask 的专用数据类型，整个求和流程使用 int。
struct SumDistributedData {
    std::vector<int> global;
    std::vector<int> local;
    std::vector<int> counts;
    std::vector<int> displacements;
};

SumDistributedData distribute_sum_input(int rank, int size)
{
    SumDistributedData data;
    data.counts.resize(size);
    data.displacements.resize(size);

    if (rank == 0) {
        data.global.resize(kElementCount);
        for (int index = 0; index < kElementCount; ++index) {
            data.global[index] = (index % 200) - 50;
        }
    }

    const int base_count = kElementCount / size;
    const int remainder = kElementCount % size;
    for (int process = 0; process < size; ++process) {
        data.counts[process] = base_count + (process < remainder ? 1 : 0);
        data.displacements[process] = process == 0
            ? 0
            : data.displacements[process - 1] + data.counts[process - 1];
    }

    data.local.resize(data.counts[rank]);
    MPI_Scatterv(
        rank == 0 ? data.global.data() : nullptr,
        data.counts.data(),
        data.displacements.data(),
        MPI_INT,
        data.local.data(),
        data.counts[rank],
        MPI_INT,
        0,
        MPI_COMM_WORLD);

    return data;
}

}

SumTask::SumTask()
    : DistributedTask("sum", "run the distributed sum task", { "run-sum" })
{
}

void SumTask::execute(int rank, int size) const
{
    const SumDistributedData data = distribute_sum_input(rank, size);
    int local_sum = 0;
    for (int value : data.local) {
        local_sum += value;
    }

    int global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << "sum complete, global_sum: " << global_sum << std::endl;
    }
}
