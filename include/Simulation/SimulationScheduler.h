#pragma once

#include <vector>
#include <chrono>

#include "Simulation/ScheduledEvent.h"

class SimulationScheduler
{
public:
    void schedule(const ScheduledEvent& event);

    bool hasPendingEvents() const;

    ScheduledEvent popNextEvent();
	
	std::chrono::nanoseconds getCurrentTime() const;
	void scheduleMCUClock(
		std::size_t nodeId,
		std::chrono::nanoseconds time
	);

private:
    std::vector<ScheduledEvent> events;
	std::chrono::nanoseconds currentTime{0};
};