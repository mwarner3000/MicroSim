#pragma once

#include <cstddef>
#include <queue>
#include <vector>

#include "Communication/CANFrame.h"

class CANController;

class CANBus
{
public:
    void transmit(const CANFrame& frame);

    bool hasPendingFrame() const;

    CANFrame receive();

    std::size_t getPendingFrameCount() const;
	
	void attach(CANController& controller);

private:
    std::queue<CANFrame> pendingFrames;
	std::vector<CANController*> controllers;
};