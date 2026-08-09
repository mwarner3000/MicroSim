#include "Devices/GPIO.h"

#include <stdexcept>

GPIO::GPIO()
    : directionRegister(0),
      outputRegister(0)
{
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
                if (pins[i].getInput())
                {
                    inputRegister |=
                        static_cast<std::uint8_t>(1u << i);
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

void GPIO::write(std::uint32_t address,
                 std::uint32_t value)
{
    switch (address)
    {
        case 0:
            directionRegister =
                static_cast<std::uint8_t>(value);
            break;

        case 1:
            outputRegister =
                static_cast<std::uint8_t>(value);

            for (std::size_t i = 0; i < pins.size(); ++i)
            {
                bool state =
                    (outputRegister & (1u << i)) != 0;

                pins[i].setOutput(state);
            }

            break;

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