#include <iostream>
#include <sstream>
#include <string>

#include <mpi.h>

#include "Command.h"
#include "IopenMPI.h"
#include "TaskRegistry.h"

int IMPI_main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank = 0;
    int size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    const TaskRegistry task_registry = create_task_registry();

    if (rank == 0) {
        std::cout << "IopenMPI worker is ready. Type 'help' for commands.\n";
    }

    bool running = true;
    while (running) {
        // 所有进程必须按相同顺序进入广播和任务函数，避免 MPI 集体调用失配。
        const std::string command = read_command(rank);
        std::istringstream command_stream(command);
        std::string operation;
        command_stream >> operation;

        if (operation == "run") {
            command_stream >> operation;
        }

        if (operation == "help") {
            if (rank == 0) {
                print_help(task_registry);
            }
        } else if (operation == "exit" || operation == "quit") {
            running = false;
        } else if (const DistributedTask* task = task_registry.find(operation)) {
            task->execute(rank, size);
        } else if (rank == 0) {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (rank == 0) {
        std::cout << "IopenMPI worker stopped.\n";
    }
    MPI_Finalize();
    return 0;
}

int main(int argc, char** argv)
{
    return IMPI_main(argc, argv);
}
