#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Communication/CANBus.h"

int main()
{
    // A transmitted frame should be placed
    // onto the CAN bus.
    {
        CANBus bus;

        CANFrame frame;
        frame.id = 0x120;
        frame.length = 2;
        frame.data[0] = 0x12;
        frame.data[1] = 0x34;

        assert(!bus.hasPendingFrame());

        bus.transmit(frame);

        assert(bus.hasPendingFrame());
        assert(
            bus.getPendingFrameCount() == 1
        );

        CANFrame received =
            bus.receive();

        assert(received.id == 0x120);
        assert(received.length == 2);
        assert(received.data[0] == 0x12);
        assert(received.data[1] == 0x34);

        assert(!bus.hasPendingFrame());
    }

    // Multiple frames should retain
    // transmission order for now.
    {
        CANBus bus;

        CANFrame first;
        first.id = 0x100;

        CANFrame second;
        second.id = 0x200;

        bus.transmit(first);
        bus.transmit(second);

        assert(
            bus.getPendingFrameCount() == 2
        );

        assert(bus.receive().id == 0x100);
        assert(bus.receive().id == 0x200);
    }

    // Receiving from an empty bus should fail.
    {
        CANBus bus;

        bool exceptionThrown = false;

        try
        {
            bus.receive();
        }
        catch (const std::runtime_error&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Invalid frames must not enter the bus.
    {
        CANBus bus;

        CANFrame frame;
        frame.length = 9;

        bool exceptionThrown = false;

        try
        {
            bus.transmit(frame);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
        assert(!bus.hasPendingFrame());
    }

    std::cout << "CANBusTests passed\n";

    return 0;
}