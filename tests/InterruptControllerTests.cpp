#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Interrupts/InterruptController.h"

int main()
{
    InterruptController controller(8);

    assert(!controller.hasPending());

    controller.request(3);

    assert(controller.hasPending());
    assert(controller.isPending(3));
    assert(controller.getNextPending() == 3);

    controller.request(1);

    // Lowest interrupt number wins for now.
    assert(controller.getNextPending() == 1);

    controller.clear(1);

    assert(!controller.isPending(1));
    assert(controller.getNextPending() == 3);

    controller.clear(3);

    assert(!controller.hasPending());

    bool exceptionThrown = false;

    try
    {
        controller.request(8);
    }
    catch (const std::out_of_range&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout
        << "Interrupt controller tests passed.\n";

    return 0;
}
