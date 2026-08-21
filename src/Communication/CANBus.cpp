#include "Communication/CANBus.h"
#include "Communication/CANController.h"

#include <stdexcept>

void CANBus::transmit(
    const CANFrame& frame
)
{
    frame.validate();

    pendingFrames.push(frame);

    for (CANController* controller :
         controllers)
    {
        controller->receiveFromBus(frame);
    }
}

bool CANBus::hasPendingFrame() const
{
    return !pendingFrames.empty();
}

CANFrame CANBus::receive()
{
    if (pendingFrames.empty())
    {
        throw std::runtime_error(
            "CAN bus has no pending frame"
        );
    }

    CANFrame frame =
        pendingFrames.front();

    pendingFrames.pop();

    return frame;
}

std::size_t CANBus::getPendingFrameCount() const
{
    return pendingFrames.size();
}

void CANBus::attach(
    CANController& controller
)
{
    controllers.push_back(&controller);
    controller.connect(*this);
}