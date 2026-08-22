#pragma once

#include <vector>

#include "Communication/CANFrame.h"

class CANController;

class CANBus
{
public:
	void attach(CANController& controller);
	
    void transmit(
		CANController& sender,
		const CANFrame& frame
	);
	
private:
	std::vector<CANController*> controllers;
};