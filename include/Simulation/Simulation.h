#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Simulator/Simulator.h"

class Simulation
{
public:
    Simulator& createNode();

    Simulator& getNode(std::size_t index);
    const Simulator& getNode(std::size_t index) const;

    std::size_t getNodeCount() const;

    void tick();

private:
    std::vector<std::unique_ptr<Simulator>> nodes;
};