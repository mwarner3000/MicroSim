#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Communication/CANBus.h"
#include "Communication/CANController.h"

int main()
{
    // Controller should begin disconnected.
    {
        CANController controller;

        assert(!controller.isConnected());
    }

    // Attaching should connect the controller.
    {
        CANBus bus;
        CANController controller;

        bus.attach(controller);

        assert(controller.isConnected());
    }

    // One transmission should be observed by
    // every attached controller.
    {
        CANBus bus;

        CANController controllerA;
        CANController controllerB;
        CANController controllerC;

        bus.attach(controllerA);
        bus.attach(controllerB);
        bus.attach(controllerC);

        CANFrame frame;
        frame.id = 0x120;
        frame.length = 2;
        frame.data[0] = 0x12;
        frame.data[1] = 0x34;

        controllerA.transmit(frame);

        assert(
            controllerA.getReceivedFrameCount() == 1
        );

        assert(
            controllerB.getReceivedFrameCount() == 1
        );

        assert(
            controllerC.getReceivedFrameCount() == 1
        );

        CANFrame received =
            controllerB.receive();

        assert(received.id == 0x120);
        assert(received.length == 2);
        assert(received.data[0] == 0x12);
        assert(received.data[1] == 0x34);

        // B consumed its own copy only.
        assert(
            controllerB.getReceivedFrameCount() == 0
        );

        assert(
            controllerC.getReceivedFrameCount() == 1
        );
    }

    // A disconnected controller cannot transmit.
    {
        CANController controller;

        CANFrame frame;

        bool exceptionThrown = false;

        try
        {
            controller.transmit(frame);
        }
        catch (const std::runtime_error&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    std::cout
        << "CANControllerTests passed\n";

    return 0;
}