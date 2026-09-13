#pragma once

#include <string>
#include <vector>

class DistributedTask {
public:
    DistributedTask(
        std::string name,
        std::string description,
        std::vector<std::string> aliases);
    virtual ~DistributedTask() = default;

    const std::string& name() const;
    const std::string& description() const;
    const std::vector<std::string>& aliases() const;

    virtual void execute(int rank, int size) const = 0;

private:
    std::string name_;
    std::string description_;
    std::vector<std::string> aliases_;
};
