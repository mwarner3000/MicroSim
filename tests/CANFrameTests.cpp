#include <cassert>
#include <iostream>

#include "Communication/CANFrame.h"

int main()
{
    CANFrame frame;

    frame.id = 0x120;
    frame.length = 3;

    frame.data[0] = 0x12;
    frame.data[1] = 0x34;
    frame.data[2] = 0x56;
	
    assert(frame.id == 0x120);
    assert(frame.length == 3);

    assert(frame.data[0] == 0x12);
    assert(frame.data[1] == 0x34);
    assert(frame.data[2] == 0x56);

    std::cout << "CANFrameTests passed\n";

    return 0;
}