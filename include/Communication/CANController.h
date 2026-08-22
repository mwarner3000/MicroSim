#pragma once

#include <cstddef>
#include <queue>
#include <array>
#include <cstdint>

#include "Bus/IBusDevice.h"
#include "Communication/CANFrame.h"
#include "Communication/CANController.h"

class CANBus;

class CANController : public IBusDevice
{
public:
    CANController();

    void connect(CANBus& bus);

    bool isConnected() const;

    void transmit(const CANFrame& frame);

    void receiveFromBus(const CANFrame& frame);

    bool hasReceivedFrame() const;

    CANFrame receive();
	CANController& getCANController();

    std::size_t getReceivedFrameCount() const;
	
	std::uint32_t read(
		std::uint32_t address
	) override;

	void write(
		std::uint32_t address,
		std::uint32_t value
	) override;

private:
    CANBus* bus;
    std::queue<CANFrame> receiveQueue;
	CANFrame transmitFrame;
};