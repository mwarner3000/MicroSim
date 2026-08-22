#include <cassert>
#include <iostream>

#include "Communication/CANBus.h"
#include "Communication/CANController.h"

int main()
{
    // A frame transmitted by one controller should
    // be delivered to every other attached controller.
    {
        CANBus bus;

        InterruptController interruptsA;
        InterruptController interruptsB;
		InterruptController interruptsC;
		
		CANController controllerA(
			interruptsA,
			1
		);

		CANController controllerB(
			interruptsB,
			1
		);
		CANController controllerC(
			interruptsC,
			1
		);

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
            !controllerA.hasReceivedFrame()
        );

        assert(
            controllerB.hasReceivedFrame()
        );

        assert(
            controllerC.hasReceivedFrame()
        );

        CANFrame frameB =
            controllerB.receive();

        CANFrame frameC =
            controllerC.receive();

        assert(frameB.id == 0x120);
        assert(frameC.id == 0x120);

        assert(frameB.length == 2);
        assert(frameC.length == 2);

        assert(frameB.data[0] == 0x12);
        assert(frameB.data[1] == 0x34);

        assert(frameC.data[0] == 0x12);
        assert(frameC.data[1] == 0x34);
    }

    // Receiving a frame on one controller must
    // not consume another controller's copy.
    {
        CANBus bus;

        InterruptController senderInterrupts;
		InterruptController receiverAInterrupts;
		InterruptController receiverBInterrupts;

		CANController sender(
			senderInterrupts,
			1
		);

		CANController receiverA(
			receiverAInterrupts,
			1
		);
		
		CANController receiverB(
			receiverBInterrupts,
			1
		);

        bus.attach(sender);
        bus.attach(receiverA);
        bus.attach(receiverB);

        CANFrame frame;
        frame.id = 0x200;

        sender.transmit(frame);

        receiverA.receive();

        assert(
            !receiverA.hasReceivedFrame()
        );

        assert(
            receiverB.hasReceivedFrame()
        );
    }

    // Invalid frames must not be delivered.
    {
        CANBus bus;

        InterruptController senderInterrupts;
		InterruptController receiverInterrupts;

		CANController sender(
			senderInterrupts,
			1
		);

		CANController receiver(
			receiverInterrupts,
			1
		);

        bus.attach(sender);
        bus.attach(receiver);

        CANFrame frame;
        frame.length = 9;

        bool exceptionThrown = false;

        try
        {
            sender.transmit(frame);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);

        assert(
            !receiver.hasReceivedFrame()
        );
    }

    std::cout << "CANBusTests passed\n";

    return 0;
}