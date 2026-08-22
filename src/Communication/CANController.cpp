#include "Communication/CANController.h"

#include <stdexcept>

#include "Communication/CANBus.h"

CANController::CANController(
    InterruptController& interruptController,
    std::size_t interruptNumber
)
    : bus(nullptr),
      interruptController(interruptController),
      interruptNumber(interruptNumber),
      receiveInterruptEnabled(false),
	  filterEnabled(false),
	  filterID(0)
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

    bus->transmit(
		*this,
		frame
	);
}

void CANController::receiveFromBus(
    const CANFrame& frame
)
{
    frame.validate();
	
	if (filterEnabled && frame.id != filterID)
	{
		return;
	}
			

    receiveQueue.push(frame);

    if (receiveInterruptEnabled)
    {
        interruptController.request(
            interruptNumber
        );
    }
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

void CANController::write(
    std::uint32_t address,
    std::uint32_t value
)
{
    switch (address)
    {
        case 0:
			if ((value & 1u) != 0)
			{
				transmit(transmitFrame);
			}

			if ((value & 2u) != 0)
			{
				if (!receiveQueue.empty())
				{
					receiveQueue.pop();
				}
			}

			break;

        case 1:
            throw std::invalid_argument(
                "CAN status register is read-only"
            );

        case 2:
            transmitFrame.id = value;
            break;

        case 3:
            if (value > transmitFrame.data.size())
            {
                throw std::out_of_range(
                    "CAN transmit length exceeds 8 bytes"
                );
            }

            transmitFrame.length =
                static_cast<std::size_t>(value);
            break;
			
		case 22:
			receiveInterruptEnabled =
				(value & 1u) != 0;
			break;
			
		case 23:
			filterEnabled =
				(value & 1u) != 0;
			break;

		case 24:
			filterID = value;
			break;

        default:
            if (address >= 4 &&
                address <= 11)
            {
                transmitFrame.data[
                    address - 4
                ] =
                    static_cast<std::uint8_t>(
                        value & 0xFFu
                    );

                return;
            }

            if (address >= 12 &&
                address <= 21)
            {
                throw std::invalid_argument(
                    "CAN receive registers are read-only"
                );
            }

            throw std::out_of_range(
                "Invalid CAN register address"
            );
    }
}

std::uint32_t CANController::read(
    std::uint32_t address
)
{
    switch (address)
    {
        case 0:
            return 0;

        case 1:
            return hasReceivedFrame()
                ? 1u
                : 0u;

        case 2:
            return transmitFrame.id;

        case 3:
            return static_cast<std::uint32_t>(
                transmitFrame.length
            );

        default:
            if (address >= 4 &&
                address <= 11)
            {
                return transmitFrame.data[
                    address - 4
                ];
            }

            break;
    }

    if (receiveQueue.empty())
    {
        if (address >= 12 &&
            address <= 21)
        {
            return 0;
        }
    }
    else
    {
        const CANFrame& frame =
            receiveQueue.front();

        if (address == 12)
        {
            return frame.id;
        }

        if (address == 13)
        {
            return static_cast<std::uint32_t>(
                frame.length
            );
        }

        if (address >= 14 &&
            address <= 21)
        {
            return frame.data[
                address - 14
            ];
        }
		
		if (address == 22)
		{
			return receiveInterruptEnabled ? 1u : 0u;
		}
		
		if (address == 23)
		{
			return filterEnabled ? 1u : 0u;
		}

		if (address == 24)
		{
			return filterID;
		}
    }

    throw std::out_of_range(
        "Invalid CAN register address"
    );
}