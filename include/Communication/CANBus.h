#pragma once

#include <cstddef>
#include <queue>

#include "Communication/CANFrame.h"

class CANBus
{
public:
    void transmit(const CANFrame& frame);

    bool hasPendingFrame() const;

    CANFrame receive();

    std::size_t getPendingFrameCount() const;

private:
    std::queue<CANFrame> pendingFrames;
};