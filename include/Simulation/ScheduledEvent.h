#pragma once

#include <chrono>
#include <cstddef>

enum class ScheduledEventType
{
    MCUClock
};

struct ScheduledEvent
{
    std::chrono::nanoseconds time;
    std::size_t id;
	ScheduledEventType type;
};