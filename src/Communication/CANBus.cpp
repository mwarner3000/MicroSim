#include "Communication/CANBus.h"
#include "Communication/CANController.h"

#include <stdexcept>

void CANBus::transmit(
    CANController& sender,
    const CANFrame& frame
)
{
    frame.validate();

    for (CANController* controller :
         controllers)
    {
        if (controller != &sender)
        {
            controller->receiveFromBus(frame);
        }
    }
}

void CANBus::attach(
    CANController& controller
)
{
    controllers.push_back(&controller);
    controller.connect(*this);
}