#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <chrono>

#include "Simulator/Simulator.h"
#include "Board/BoardConfig.h"
#include "Communication/CANBus.h"

class Simulation
{
public:
	Simulation();
	
	Simulator& createNode();
    Simulator& createNode(
		const BoardConfig& config
	);

    Simulator& getNode(std::size_t index);
    const Simulator& getNode(std::size_t index) const;

    std::size_t getNodeCount() const;
	
	std::chrono::nanoseconds getCurrentTime() const;
	
	void advanceTime(
		std::chrono::nanoseconds duration
	);

private:
    std::vector<std::unique_ptr<Simulator>> nodes;
	CANBus canBus;
	std::chrono::nanoseconds elapsedTime;
};