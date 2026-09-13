#include "Command.h"

#include <mpi.h>

#include <algorithm>
#include <iostream>

namespace {

const int kCommandSize = 256;

}

std::string read_command(int rank)
{
    char command[kCommandSize] = {};

    // 只有 0 号进程读取控制台，避免多个 MPI 进程争抢标准输入。
    if (rank == 0) {
        std::cout << "IopenMPI> " << std::flush;
        std::string input;
        if (!std::getline(std::cin, input)) {
            input = "exit";
        }
        const size_t length = std::min(input.size(), static_cast<size_t>(kCommandSize - 1));
        std::copy(input.begin(), input.begin() + length, command);
    }

    // 所有进程必须执行同一次广播，才能获得一致的任务指令。
    MPI_Bcast(command, kCommandSize, MPI_CHAR, 0, MPI_COMM_WORLD);
    return std::string(command);
}
