#pragma once

#include <cstddef>
#include <queue>

#include "Communication/CANFrame.h"

class CANBus;

class CANController
{
public:
    CANController();

    void connect(CANBus& bus);

    bool isConnected() const;

    void transmit(const CANFrame& frame);

    void receiveFromBus(const CANFrame& frame);

    bool hasReceivedFrame() const;

    CANFrame receive();

    std::size_t getReceivedFrameCount() const;

private:
    CANBus* bus;
    std::queue<CANFrame> receiveQueue;
};