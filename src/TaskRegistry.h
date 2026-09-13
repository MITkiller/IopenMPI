#pragma once

#include "DistributedTask.h"

#include <memory>
#include <vector>

class TaskRegistry {
public:
    void add(std::unique_ptr<DistributedTask> task);
    const DistributedTask* find(const std::string& name) const;
    const std::vector<std::unique_ptr<DistributedTask>>& tasks() const;

private:
    std::vector<std::unique_ptr<DistributedTask>> tasks_;
};

TaskRegistry create_task_registry();
void print_help(const TaskRegistry& registry);
