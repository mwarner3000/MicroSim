#include "Devices/GPIO.h"

#include <stdexcept>

GPIO::GPIO(
    std::size_t pinCount,
    double logicVoltage,
    double digitalHighThreshold
)
    : directionRegister(0),
      outputRegister(0),
      logicVoltage(logicVoltage),
      digitalHighThreshold(digitalHighThreshold),
      pins(pinCount)
{
    if (pinCount == 0 || pinCount > 32)
    {
        throw std::invalid_argument(
            "GPIO pin count must be between 1 and 32"
        );
    }
}

std::uint32_t GPIO::read(std::uint32_t address)
{
    switch (address)
    {
        case 0:
            return directionRegister;

        case 1:
            return outputRegister;

        case 2:
        {
            std::uint8_t inputRegister = 0;

            for (std::size_t i = 0; i < pins.size(); ++i)
            {
                if (pins[i].getVoltage() >= digitalHighThreshold)
                {
                    inputRegister |= 1u << i;
                }
            }

            return inputRegister;
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
			directionRegister =
				(value);

			for (std::size_t i = 0; i < pins.size(); ++i)
			{
				bool isOutput =
					(directionRegister & (1u << i)) != 0;

				pins[i].setDirection(
					isOutput
						? PinDirection::Output
						: PinDirection::Input
				);

				if (isOutput)
				{
					bool high =
						(outputRegister & (1u << i)) != 0;

					pins[i].setVoltage(
						high ? logicVoltage : 0.0
					);
				}
			}

			break;
		}

        case 1:
        {
            outputRegister = value;

            for (std::size_t i = 0; i < pins.size(); ++i)
            {
                if (pins[i].getDirection() ==
                    PinDirection::Output)
                {
                    bool high =
                        (outputRegister & (1u << i)) != 0;

                    pins[i].setVoltage(
                        high ? logicVoltage : 0.0
                    );
                }
            }

            break;
        }

        case 2:
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