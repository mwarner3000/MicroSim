#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class InterruptController
{
public:
    explicit InterruptController(
        std::size_t interruptCount = 8
    );

    void request(std::size_t interruptNumber);
    void clear(std::size_t interruptNumber);

    bool isPending(std::size_t interruptNumber) const;

    bool hasPending() const;

    std::size_t getNextPending() const;

private:
    std::vector<bool> pending;
};