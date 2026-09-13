#include "StructTask.h"

#include <mpi.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace {

const int kElementCount = 10000;

struct MyStruct {
    std::uint64_t a;
    std::uint32_t b;
};

MPI_Datatype create_my_struct_type()
{
    MPI_Datatype raw_type;
    MPI_Datatype resized_type;
    int block_lengths[2] = { 1, 1 };
    MPI_Datatype member_types[2] = { MPI_UINT64_T, MPI_UINT32_T };

    // MyStruct 是标准布局（standard-layout）类型，offsetof 良定义，
    // 直接给出成员相对结构体起始的字节偏移，可正确处理对齐和填充。
    // 相比 MPI_Get_address（MPI-4 已废弃）更具可移植性。
    MPI_Aint addresses[2] = {
        static_cast<MPI_Aint>(offsetof(MyStruct, a)),
        static_cast<MPI_Aint>(offsetof(MyStruct, b))
    };

    MPI_Type_create_struct(2, block_lengths, addresses, member_types, &raw_type);
    // 让 MPI 数组步长与 C++ 结构体大小一致。
    MPI_Type_create_resized(raw_type, 0, sizeof(MyStruct), &resized_type);
    MPI_Type_commit(&resized_type);
    MPI_Type_free(&raw_type);
    return resized_type;
}

void distribute_structs(
    int rank,
    int size,
    std::vector<MyStruct>& global,
    std::vector<MyStruct>& local,
    std::vector<int>& counts,
    std::vector<int>& displacements)
{
    counts.resize(size);
    displacements.resize(size);
    if (rank == 0) {
        global.resize(kElementCount);
        for (int index = 0; index < kElementCount; ++index) {
            global[index].a = static_cast<std::uint64_t>(index);
            global[index].b = static_cast<std::uint32_t>(index % 100);
        }
    }

    const int base_count = kElementCount / size;
    const int remainder = kElementCount % size;
    for (int process = 0; process < size; ++process) {
        counts[process] = base_count + (process < remainder ? 1 : 0);
        displacements[process] = process == 0
            ? 0
            : displacements[process - 1] + counts[process - 1];
    }

    local.resize(counts[rank]);
    MPI_Datatype struct_type = create_my_struct_type();
    MPI_Scatterv(
        rank == 0 ? global.data() : nullptr,
        counts.data(),
        displacements.data(),
        struct_type,
        local.data(),
        counts[rank],
        struct_type,
        0,
        MPI_COMM_WORLD);
    MPI_Type_free(&struct_type);
}

}

StructTask::StructTask()
    : DistributedTask("struct", "run the custom structure task", { "run-struct" })
{
}

void StructTask::execute(int rank, int size) const
{
    std::vector<MyStruct> global;
    std::vector<MyStruct> local;
    std::vector<int> counts;
    std::vector<int> displacements;
    distribute_structs(rank, size, global, local, counts, displacements);

    std::uint64_t local_checksum = 0;
    for (const MyStruct& value : local) {
        local_checksum += value.a + value.b;
    }

    std::uint64_t global_checksum = 0;
    MPI_Reduce(
        &local_checksum,
        &global_checksum,
        1,
        MPI_UINT64_T,
        MPI_SUM,
        0,
        MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << "struct complete, checksum: " << global_checksum << std::endl;
    }
}
