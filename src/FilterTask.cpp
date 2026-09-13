#include "FilterTask.h"

#include <mpi.h>

#include <iostream>
#include <vector>

namespace {

const int kElementCount = 10000;

// FilterTask 的专用数据类型，数据元素保持 double 类型。
struct DistributedData {
    std::vector<double> global;
    std::vector<double> local;
    std::vector<int> counts;
    std::vector<int> displacements;
};

DistributedData distribute_input(int rank, int size)
{
    DistributedData data;
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
        MPI_DOUBLE,
        data.local.data(),
        data.counts[rank],
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD);

    return data;
}

}

FilterTask::FilterTask()
    : DistributedTask("filter", "run the distributed filter task", { "run-filter" })
{
}

void FilterTask::execute(int rank, int size) const
{
    const DistributedData data = distribute_input(rank, size);
    std::vector<double> local_filtered;
    for (double value : data.local) {
        if (value < 100.0) {
            local_filtered.push_back(value);
        }
    }

    const int local_count = static_cast<int>(local_filtered.size());
    std::vector<int> all_counts(size);
    MPI_Gather(&local_count, 1, MPI_INT, all_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> all_displacements(size, 0);
    int total_filtered = 0;
    if (rank == 0) {
        for (int process = 0; process < size; ++process) {
            all_displacements[process] = total_filtered;
            total_filtered += all_counts[process];
        }
    }

    std::vector<double> global_filtered(rank == 0 ? total_filtered : 0);
    MPI_Gatherv(
        local_filtered.data(),
        local_count,
        MPI_DOUBLE,
        rank == 0 ? global_filtered.data() : nullptr,
        all_counts.data(),
        all_displacements.data(),
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "filter complete, total_filtered: " << total_filtered << std::endl;
    }
}
