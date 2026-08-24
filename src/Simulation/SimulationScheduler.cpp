#include <stdexcept>

#include "Simulation/SimulationScheduler.h"

void SimulationScheduler::schedule(const ScheduledEvent& event)
{
	events.push_back(event);
}

bool SimulationScheduler::hasPendingEvents() const
{
	return !events.empty();
}

ScheduledEvent SimulationScheduler::popNextEvent()
{
	if(events.empty())
	{
		throw std::runtime_error("No scheduled events");
	}
	
	std::size_t earliestIndex = 0;
	
	for(size_t i = 0; i < events.size(); i++)
	{
		if(events[i].time < events[earliestIndex].time)
		{
			earliestIndex = i;
		}
	}
	
	ScheduledEvent nextEvent = events[earliestIndex];
	
	events.erase(events.begin() + earliestIndex);
	
	currentTime = nextEvent.time;
	
	return nextEvent;
}

std::chrono::nanoseconds 
	SimulationScheduler::getCurrentTime() const
{
    return currentTime;
}

void SimulationScheduler::scheduleMCUClock(
    std::size_t nodeId,
    std::chrono::nanoseconds time
)
{
	schedule(
		ScheduledEvent{
			time,
			nodeId,
			ScheduledEventType::MCUClock
		}
	);
}