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
            controllerA.getReceivedFrameCount() == 0
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
	
	// Controller registers should construct and
	// transmit a CAN frame.
	{
		CANBus bus;

		CANController sender;
		CANController receiver;

		bus.attach(sender);
		bus.attach(receiver);

		// TX ID
		sender.write(2, 0x321);

		// TX length
		sender.write(3, 2);

		// TX payload
		sender.write(4, 0xAA);
		sender.write(5, 0x55);

		// Transmit command.
		sender.write(0, 1);

		assert(receiver.read(1) == 1);

		assert(receiver.read(12) == 0x321);
		assert(receiver.read(13) == 2);
		assert(receiver.read(14) == 0xAA);
		assert(receiver.read(15) == 0x55);
	}
	
	//boundary test
	{
		CANController controller;

		bool exceptionThrown = false;

		try
		{
			controller.write(3, 9);
		}
		catch (const std::out_of_range&)
		{
			exceptionThrown = true;
		}

		assert(exceptionThrown);
	}
	
	// Firmware should be able to acknowledge one
	// received frame and expose the next queued frame.
	{
		CANBus bus;

		CANController sender;
		CANController receiver;

		bus.attach(sender);
		bus.attach(receiver);

		CANFrame first;
		first.id = 0x100;

		CANFrame second;
		second.id = 0x200;

		sender.transmit(first);
		sender.transmit(second);

		assert(receiver.read(12) == 0x100);

		// CONTROL bit 1 = acknowledge/pop RX frame.
		receiver.write(0, 2);

		assert(receiver.read(12) == 0x200);

		receiver.write(0, 2);

		assert(receiver.read(1) == 0);
	}

    std::cout
        << "CANControllerTests passed\n";

    return 0;
}