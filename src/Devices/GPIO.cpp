#include "Devices/GPIO.h"

#include <stdexcept>

GPIO::GPIO(
    std::size_t pinCount,
    double logicVoltage,
    double digitalHighThreshold
)
    : selectedPin(0),
      logicVoltage(logicVoltage),
      digitalHighThreshold(digitalHighThreshold),
      pins(pinCount)
{
    if (pinCount == 0)
    {
        throw std::invalid_argument(
            "GPIO must contain at least one pin"
        );
    }
}

std::uint32_t GPIO::read(std::uint32_t address)
{
    switch (address)
    {
        case 0:
            // PIN_SELECT
            return static_cast<std::uint32_t>(
                selectedPin
            );

        case 1:
            // DIRECTION
            return
                pins[selectedPin].getDirection() ==
                PinDirection::Output
                    ? 1u
                    : 0u;

        case 2:
            // OUTPUT
            return
                pins[selectedPin].getOutputLatch()
                    ? 1u
                    : 0u;

        case 3:
		{
            // INPUT
            auto voltage =
				pins[selectedPin].getEffectiveVoltage(
					logicVoltage
				);

			if (!voltage.has_value())
			{
				//Floating digital inputs read LOW deterministically
				return 0;
			}

			return voltage.value() >= digitalHighThreshold
				? 1u
				: 0u;
		}
				
        default:
            throw std::out_of_range(
                "Invalid GPIO register address"
            );
    }
}

void GPIO::write(
    std::uint32_t address,
    std::uint32_t value
)
{
    switch (address)
    {
        case 0:
        {
            // PIN_SELECT

            if (value >= pins.size())
            {
                throw std::out_of_range(
                    "GPIO selected pin is out of range"
                );
            }

            selectedPin =
                static_cast<std::size_t>(value);

            break;
        }

        case 1:
        {
            // DIRECTION

            PinDirection direction =
                (value & 1u)
                    ? PinDirection::Output
                    : PinDirection::Input;

            pins[selectedPin].setDirection(direction);

            break;
        }

        case 2:
        {
            // OUTPUT
            pins[selectedPin].setOutputLatch(
				(value & 1u) != 0
			);

            break;
        }

        case 3:
            throw std::invalid_argument(
                "GPIO input register is read-only"
            );

        default:
            throw std::out_of_range(
                "Invalid GPIO register address"
            );
    }
}

Pin& GPIO::getPin(std::size_t index)
{
    if (index >= pins.size())
    {
        throw std::out_of_range(
            "Invalid GPIO pin index"
        );
    }

    return pins[index];
}

const Pin& GPIO::getPin(std::size_t index) const
{
    if (index >= pins.size())
    {
        throw std::out_of_range(
            "Invalid GPIO pin index"
        );
    }

    return pins[index];
}

std::size_t GPIO::getPinCount() const
{
    return pins.size();
}