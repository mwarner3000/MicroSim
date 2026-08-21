#include "Communication/CANController.h"

#include <stdexcept>

#include "Communication/CANBus.h"

CANController::CANController()
    : bus(nullptr)
{
}

void CANController::connect(CANBus& newBus)
{
    bus = &newBus;
}

bool CANController::isConnected() const
{
    return bus != nullptr;
}

void CANController::transmit(
    const CANFrame& frame
)
{
    if (bus == nullptr)
    {
        throw std::runtime_error(
            "CAN controller is not connected to a CAN bus"
        );
    }

    frame.validate();

    bus->transmit(frame);
}

void CANController::receiveFromBus(
    const CANFrame& frame
)
{
    frame.validate();

    receiveQueue.push(frame);
}

bool CANController::hasReceivedFrame() const
{
    return !receiveQueue.empty();
}

CANFrame CANController::receive()
{
    if (receiveQueue.empty())
    {
        throw std::runtime_error(
            "CAN controller has no received frame"
        );
    }

    CANFrame frame =
        receiveQueue.front();

    receiveQueue.pop();

    return frame;
}

std::size_t
CANController::getReceivedFrameCount() const
{
    return receiveQueue.size();
}