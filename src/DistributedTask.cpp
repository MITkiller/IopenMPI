#include "DistributedTask.h"

#include <utility>

DistributedTask::DistributedTask(
    std::string name,
    std::string description,
    std::vector<std::string> aliases)
    : name_(std::move(name)),
      description_(std::move(description)),
      aliases_(std::move(aliases))
{
}

const std::string& DistributedTask::name() const
{
    return name_;
}

const std::string& DistributedTask::description() const
{
    return description_;
}

const std::vector<std::string>& DistributedTask::aliases() const
{
    return aliases_;
}
