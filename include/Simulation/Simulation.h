#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Simulator/Simulator.h"
#include "Board/BoardConfig.h"

class Simulation
{
public:
	Simulator& createNode();
    Simulator& createNode(
		const BoardConfig& config
	);

    Simulator& getNode(std::size_t index);
    const Simulator& getNode(std::size_t index) const;

    std::size_t getNodeCount() const;

    void tick();

private:
    std::vector<std::unique_ptr<Simulator>> nodes;
};