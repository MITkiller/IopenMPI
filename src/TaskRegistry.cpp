#include "TaskRegistry.h"

#include "FilterTask.h"
#include "StructTask.h"
#include "SumTask.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

void TaskRegistry::add(std::unique_ptr<DistributedTask> task)
{
    tasks_.push_back(std::move(task));
}

const DistributedTask* TaskRegistry::find(const std::string& name) const
{
    for (const std::unique_ptr<DistributedTask>& task : tasks_) {
        if (task->name() == name ||
            std::find(task->aliases().begin(), task->aliases().end(), name) != task->aliases().end()) {
            return task.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<DistributedTask>>& TaskRegistry::tasks() const
{
    return tasks_;
}

TaskRegistry create_task_registry()
{
    TaskRegistry registry;

    // 新增任务时继承 DistributedTask，然后在这里注册一个实例即可。
    registry.add(std::make_unique<FilterTask>());
    registry.add(std::make_unique<SumTask>());
    registry.add(std::make_unique<StructTask>());

    // 删除任务时只需移除对应注册项，主循环无需修改。
    return registry;
}

void print_help(const TaskRegistry& registry)
{
    std::cout << "Commands:\n";
    for (const std::unique_ptr<DistributedTask>& task : registry.tasks()) {
        std::cout << "  " << task->name() << "       " << task->description() << "\n";
    }
    std::cout << "  help         show this help\n"
        << "  exit         stop all MPI processes\n";
}
