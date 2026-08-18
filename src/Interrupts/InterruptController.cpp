#include "Interrupts/InterruptController.h"

#include <stdexcept>

InterruptController::InterruptController(
    std::size_t interruptCount
)
    : pending(interruptCount, false)
{
}

void InterruptController::request(
    std::size_t interruptNumber
)
{
    if (interruptNumber >= pending.size())
    {
        throw std::out_of_range(
            "Invalid interrupt number"
        );
    }

    pending[interruptNumber] = true;
}

void InterruptController::clear(
    std::size_t interruptNumber
)
{
    if (interruptNumber >= pending.size())
    {
        throw std::out_of_range(
            "Invalid interrupt number"
        );
    }

    pending[interruptNumber] = false;
}

bool InterruptController::isPending(
    std::size_t interruptNumber
) const
{
    if (interruptNumber >= pending.size())
    {
        throw std::out_of_range(
            "Invalid interrupt number"
        );
    }

    return pending[interruptNumber];
}

bool InterruptController::hasPending() const
{
    for (bool value : pending)
    {
        if (value)
        {
            return true;
        }
    }

    return false;
}

std::size_t InterruptController::getNextPending() const
{
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        if (pending[i])
        {
            return i;
        }
    }

    throw std::runtime_error(
        "No interrupt is pending"
    );
}