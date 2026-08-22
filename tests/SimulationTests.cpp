#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Simulation/Simulation.h"
#include "CPU/SimpleCPU/SimpleISA.h"

int main()
{
    // Node creation.
    {
        Simulation simulation;

        assert(simulation.getNodeCount() == 0);

        simulation.createNode();
        simulation.createNode();

        assert(simulation.getNodeCount() == 2);
    }

    // Nodes should have independent memory.
    {
        Simulation simulation;

        Simulator& nodeA = simulation.createNode();
        Simulator& nodeB = simulation.createNode();

        nodeA.getBus().write(100, 111);
        nodeB.getBus().write(100, 222);

        assert(nodeA.getBus().read(100) == 111);
        assert(nodeB.getBus().read(100) == 222);
    }

    // Nodes should have independent CPUs.
    {
        Simulation simulation;

        Simulator& nodeA = simulation.createNode();
        Simulator& nodeB = simulation.createNode();

        nodeA.getBus().write(
            0,
            SimpleISA::encode(
                SimpleISA::Opcode::MOVI,
                1,
                0,
                10
            )
        );

        nodeB.getBus().write(
            0,
            SimpleISA::encode(
                SimpleISA::Opcode::MOVI,
                1,
                0,
                20
            )
        );
		
		nodeA.getBus().write(
			1,
			SimpleISA::encode(
				SimpleISA::Opcode::HALT
			)
		);

		nodeB.getBus().write(
			1,
			SimpleISA::encode(
				SimpleISA::Opcode::HALT
			)
		);

        simulation.advanceTime(
			std::chrono::milliseconds(1)
		);

        assert(nodeA.getCPU().getRegister(1) == 10);
        assert(nodeB.getCPU().getRegister(1) == 20);
    }

    // Invalid node access.
    {
        Simulation simulation;

        bool exceptionThrown = false;

        try
        {
            simulation.getNode(0);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }
	
	//boardconfig test
	{
		BoardConfig config;

		config.clockHz = 32'000'000;
		config.ramWords = 256;

		Simulator simulator(config);

		assert(
			simulator.getConfig().clockHz ==
			32'000'000
		);

		assert(
			simulator.getConfig().ramWords ==
			256
		);
	}
	
	//verify RAM changes
	{
		BoardConfig smallConfig;
		smallConfig.ramWords = 256;

		BoardConfig largeConfig;
		largeConfig.ramWords = 2048;

		Simulator smallBoard(smallConfig);
		Simulator largeBoard(largeConfig);

		// Last valid address on small board.
		smallBoard.getBus().write(255, 123);

		assert(
			smallBoard.getBus().read(255) == 123
		);

		// Address 256 should not exist on the small board.
		bool smallBoardRejected = false;

		try
		{
			smallBoard.getBus().write(256, 123);
		}
		catch (const std::out_of_range&)
		{
			smallBoardRejected = true;
		}

		assert(smallBoardRejected);

		// But address 256 is valid on the larger board.
		largeBoard.getBus().write(256, 456);

		assert(
			largeBoard.getBus().read(256) == 456
		);
	}
	
	// Board should support a custom memory map.
	{
		BoardConfig config;

		config.ramBase   = 0x00000000;
		config.gpioBase  = 0x00010000;
		config.timerBase = 0x00020000;

		Simulator simulator(config);

		Bus& bus = simulator.getBus();

		// RAM
		bus.write(config.ramBase, 123);
		assert(bus.read(config.ramBase) == 123);

		// GPIO direction register
		bus.write(config.gpioBase, 0x01);
		assert(bus.read(config.gpioBase) == 0x01);

		// Timer period register
		bus.write(config.timerBase + 1, 100);
		assert(bus.read(config.timerBase + 1) == 100);
	}
	
	// Overlapping memory regions should be rejected.
	{
		BoardConfig config;

		config.ramWords = 1024;

		// Deliberately put GPIO inside RAM.
		config.gpioBase = 0x00000100;

		bool exceptionThrown = false;

		try
		{
			Simulator simulator(config);
		}
		catch (const std::invalid_argument&)
		{
			exceptionThrown = true;
		}

		assert(exceptionThrown);
	}
	
	{
		BoardConfig config;
		config.clockHz = 16'000'000;

		Simulator simulator(config);

		simulator.getBus().write(
			config.ramBase,
			SimpleISA::encode(
				SimpleISA::Opcode::HALT
			)
		);

		simulator.advanceTime(
			std::chrono::microseconds(500)
		);

		assert(
			simulator.getClock().getCycle() == 8000
		);

		assert(
			simulator.getCPU().isHalted()
		);
	}
	
	//compare two boards
	{
		BoardConfig slowConfig;
		slowConfig.clockHz = 16'000'000;

		BoardConfig fastConfig;
		fastConfig.clockHz = 32'000'000;

		Simulator slowBot(slowConfig);
		Simulator fastBot(fastConfig);

		slowBot.getBus().write(
			slowConfig.ramBase,
			SimpleISA::encode(SimpleISA::Opcode::HALT)
		);

		fastBot.getBus().write(
			fastConfig.ramBase,
			SimpleISA::encode(SimpleISA::Opcode::HALT)
		);

		slowBot.advanceTime(
			std::chrono::milliseconds(1)
		);

		fastBot.advanceTime(
			std::chrono::milliseconds(1)
		);

		assert(
			slowBot.getClock().getCycle() == 16'000
		);

		assert(
			fastBot.getClock().getCycle() == 32'000
		);
	}
	
	//fractional time accumulation test
	{
		BoardConfig config;
		config.clockHz = 1'000'000;

		Simulator simulator(config);

		simulator.getBus().write(
			config.ramBase,
			SimpleISA::encode(SimpleISA::Opcode::HALT)
		);

		for (int i = 0; i < 1000; ++i)
		{
			simulator.advanceTime(
				std::chrono::nanoseconds(1)
			);
		}

		assert(
			simulator.getClock().getCycle() == 1
		);
	}
	
	// Real-time mode should start and stop cleanly.
	{
		Simulator simulator;

		assert(!simulator.isRealTimeRunning());

		simulator.startRealTime();

		assert(simulator.isRealTimeRunning());

		simulator.stopRealTime();

		assert(!simulator.isRealTimeRunning());
	}

	// Updating while real-time mode is stopped should do nothing.
	{
		Simulator simulator;

		std::uint64_t before =
			simulator.getClock().getCycle();

		simulator.updateRealTime();

		std::uint64_t after =
			simulator.getClock().getCycle();

		assert(after == before);
	}
	
	// External world should be able to drive a pin.
	{
		Simulator simulator;

		simulator.setPinVoltage(3, 2.5);

		assert(
			simulator.getGPIO()
				.getPin(3)
				.getVoltage() == 2.5
		);
	}
	
	// External world should be able to observe a pin.
	{
		Simulator simulator;

		simulator.getGPIO()
			.getPin(5)
			.setVoltage(5.0);

		assert(
			simulator.getPinVoltage(5) == 5.0
		);
	}
	
	// Invalid external pin access should fail.
	{
		Simulator simulator;

		bool exceptionThrown = false;

		try
		{
			simulator.setPinVoltage(1000, 5.0);
		}
		catch (const std::out_of_range&)
		{
			exceptionThrown = true;
		}

		assert(exceptionThrown);
	}
	
	// Complete external-world integration test:
	// switch input -> firmware -> light output.
	{
		BoardConfig config;
		config.gpioPins = 2;

		Simulator bot(config);
		Bus& bus = bot.getBus();

		const std::uint32_t gpioSelect =
			config.gpioBase + 0;

		const std::uint32_t gpioDirection =
			config.gpioBase + 1;

		const std::uint32_t gpioOutput =
			config.gpioBase + 2;

		const std::uint32_t gpioInput =
			config.gpioBase + 3;

		// ------------------------------------------------
		// Firmware
		//
		// Pin 0 = switch input
		// Pin 1 = light output
		//
		// Configure pin 1 as output.
		// Then repeatedly:
		//   read pin 0
		//   if LOW  -> pin 1 LOW
		//   if HIGH -> pin 1 HIGH
		// ------------------------------------------------

		// 0: Select pin 1.
		bus.write(
			0,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 1: GPIO PIN_SELECT = 1.
		bus.write(
			1,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioSelect
			)
		);

		// 2: R0 = 1 (Output).
		bus.write(
			2,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 3: GPIO DIRECTION = Output.
		bus.write(
			3,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioDirection
			)
		);

		// ---------------- LOOP ----------------

		// 4: Select switch pin 0.
		bus.write(
			4,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				0
			)
		);

		// 5: GPIO PIN_SELECT = 0.
		bus.write(
			5,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioSelect
			)
		);

		// 6: Read switch into R1.
		bus.write(
			6,
			SimpleISA::encode(
				SimpleISA::Opcode::LOAD,
				1,
				0,
				gpioInput
			)
		);

		// 7: Compare switch with LOW.
		bus.write(
			7,
			SimpleISA::encode(
				SimpleISA::Opcode::CMPI,
				1,
				0,
				0
			)
		);

		// 8: If LOW, jump to OFF handler.
		bus.write(
			8,
			SimpleISA::encode(
				SimpleISA::Opcode::JZ,
				0,
				0,
				14
			)
		);

		// ---------------- ON ----------------

		// 9: Select light pin 1.
		bus.write(
			9,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 10: GPIO PIN_SELECT = 1.
		bus.write(
			10,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioSelect
			)
		);

		// 11: R0 = HIGH.
		bus.write(
			11,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 12: Light output HIGH.
		bus.write(
			12,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioOutput
			)
		);

		// 13: Repeat.
		bus.write(
			13,
			SimpleISA::encode(
				SimpleISA::Opcode::JMP,
				0,
				0,
				4
			)
		);

		// ---------------- OFF ----------------

		// 14: Select light pin 1.
		bus.write(
			14,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 15: GPIO PIN_SELECT = 1.
		bus.write(
			15,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioSelect
			)
		);

		// 16: R0 = LOW.
		bus.write(
			16,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				0
			)
		);

		// 17: Light output LOW.
		bus.write(
			17,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				gpioOutput
			)
		);

		// 18: Repeat.
		bus.write(
			18,
			SimpleISA::encode(
				SimpleISA::Opcode::JMP,
				0,
				0,
				4
			)
		);

		bot.getCPU().reset();

		// ----------------------------------------
		// External world: switch starts OFF.
		// ----------------------------------------

		bot.setPinVoltage(0, 0.0);

		bot.advanceCycles(30);

		assert(
			bot.getPinVoltage(1) == 0.0
		);

		// ----------------------------------------
		// External world: user closes switch.
		// ----------------------------------------

		bot.setPinVoltage(0, 5.0);

		bot.advanceCycles(30);

		assert(
			bot.getPinVoltage(1) == 5.0
		);

		// ----------------------------------------
		// External world: user opens switch again.
		// ----------------------------------------

		bot.setPinVoltage(0, 0.0);

		bot.advanceCycles(30);

		assert(
			bot.getPinVoltage(1) == 0.0
		);
	}
	
	//prove the shared controller really is the same instance the simulator owns
	{
		Simulator simulator;

		InterruptController& controller =
			simulator.getInterruptController();

		controller.request(3);

		assert(controller.hasPending());
		assert(controller.getNextPending() == 3);
	}
	
	// End-to-end timer interrupt test:
	// Timer -> InterruptController -> CPU -> handler -> RETI.
	{
		Simulator simulator;

		Bus& bus = simulator.getBus();
		SimpleCPU& cpu = simulator.getCPU();

		const BoardConfig& config =
			simulator.getConfig();

		const std::uint32_t timerPeriod =
			config.timerBase + 1;

		const std::uint32_t timerEnable =
			config.timerBase + 2;

		const std::uint32_t timerInterruptEnable =
			config.timerBase + 4;

		// --------------------------------------------
		// Normal firmware
		//
		// Keep incrementing R0 forever.
		// The timer interrupt handler will set R1 = 99.
		// --------------------------------------------

		// 0: R0 = 1
		bus.write(
			0,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 1: R2 = 1
		bus.write(
			1,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				2,
				0,
				1
			)
		);

		// 2: R0 += R2
		bus.write(
			2,
			SimpleISA::encode(
				SimpleISA::Opcode::ADD,
				0,
				2,
				0
			)
		);

		// 3: loop back to address 2
		bus.write(
			3,
			SimpleISA::encode(
				SimpleISA::Opcode::JMP,
				0,
				0,
				2
			)
		);

		// --------------------------------------------
		// IRQ 0 vector
		// --------------------------------------------

		bus.write(
			SimpleCPU::InterruptVectorBase,
			100
		);

		// --------------------------------------------
		// Interrupt handler
		// --------------------------------------------

		// 100: R1 = 99
		bus.write(
			100,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				1,
				0,
				99
			)
		);

		// 101: RETI
		bus.write(
			101,
			SimpleISA::encode(
				SimpleISA::Opcode::RETI
			)
		);

		// Configure timer.
		bus.write(timerPeriod, 3);
		bus.write(timerInterruptEnable, 1);
		bus.write(timerEnable, 1);

		cpu.reset();

		// Handler has not run yet.
		assert(cpu.getRegister(1) == 0);

		// Give the complete MCU enough cycles for the timer
		// to expire and for the interrupt handler to execute.
		simulator.advanceCycles(10);

		// Timer-generated IRQ should have caused firmware
		// at address 100 to execute.
		assert(cpu.getRegister(1) == 99);

		// RETI should have completed, so normal firmware
		// should also have resumed.
		assert(cpu.getRegister(0) > 1);
	}
	
	// Simulation should support independently
	// configured controller nodes.
	{
		Simulation simulation;

		BoardConfig configA;
		configA.clockHz = 16'000'000;
		configA.ramWords = 1024;
		configA.gpioPins = 8;

		BoardConfig configB;
		configB.clockHz = 32'000'000;
		configB.ramWords = 2048;
		configB.gpioPins = 100;

		Simulator& nodeA =
			simulation.createNode(configA);

		Simulator& nodeB =
			simulation.createNode(configB);

		assert(simulation.getNodeCount() == 2);

		assert(
			nodeA.getConfig().clockHz ==
			16'000'000
		);

		assert(
			nodeB.getConfig().clockHz ==
			32'000'000
		);

		assert(
			nodeA.getGPIO().getPinCount() == 8
		);

		assert(
			nodeB.getGPIO().getPinCount() == 100
		);
	}
	
	// Two simulated controllers should communicate
	// over the shared CAN bus through memory-mapped
	// CAN registers.
	{
		Simulation simulation;

		Simulator& nodeA =
			simulation.createNode();

		Simulator& nodeB =
			simulation.createNode();

		const std::uint32_t canA =
			nodeA.getConfig().canBase;

		const std::uint32_t canB =
			nodeB.getConfig().canBase;

		Bus& busA = nodeA.getBus();
		Bus& busB = nodeB.getBus();

		// Node A TX frame:
		// ID = 0x123
		// length = 2
		// data = AA 55

		busA.write(canA + 2, 0x123);
		busA.write(canA + 3, 2);

		busA.write(canA + 4, 0xAA);
		busA.write(canA + 5, 0x55);

		// CONTROL bit 0 = transmit.
		busA.write(canA + 0, 1);

		// Node B should now have an RX frame.
		assert(
			busB.read(canB + 1) == 1
		);

		assert(
			busB.read(canB + 12) == 0x123
		);

		assert(
			busB.read(canB + 13) == 2
		);

		assert(
			busB.read(canB + 14) == 0xAA
		);

		assert(
			busB.read(canB + 15) == 0x55
		);

		// Sender should not receive its own frame.
		assert(
			busA.read(canA + 1) == 0
		);
	}
	
	// End-to-end CAN firmware interrupt test:
	//
	// Node A firmware sends a CAN frame.
	// Node B CAN hardware receives it.
	// CAN raises IRQ 1.
	// Node B CPU enters its interrupt handler.
	// Handler reads the frame, acknowledges it,
	// then returns with RETI.
	{
		Simulation simulation;

		Simulator& nodeA =
			simulation.createNode();

		Simulator& nodeB =
			simulation.createNode();

		Bus& busA = nodeA.getBus();
		Bus& busB = nodeB.getBus();

		SimpleCPU& cpuB = nodeB.getCPU();

		const std::uint32_t canA =
			nodeA.getConfig().canBase;

		const std::uint32_t canB =
			nodeB.getConfig().canBase;

		// --------------------------------------------
		// Node A firmware
		//
		// Send:
		// ID     = 0x123
		// Length = 2
		// Data   = AA 55
		// --------------------------------------------

		// 0: R0 = 0x123
		busA.write(
			0,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				0x123
			)
		);

		// 1: TX_ID = R0
		busA.write(
			1,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				canA + 2
			)
		);

		// 2: R0 = 2
		busA.write(
			2,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				2
			)
		);

		// 3: TX_LENGTH = 2
		busA.write(
			3,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				canA + 3
			)
		);

		// 4: R0 = 0xAA
		busA.write(
			4,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				0xAA
			)
		);

		// 5: TX_DATA_0 = 0xAA
		busA.write(
			5,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				canA + 4
			)
		);

		// 6: R0 = 0x55
		busA.write(
			6,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				0x55
			)
		);

		// 7: TX_DATA_1 = 0x55
		busA.write(
			7,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				canA + 5
			)
		);

		// 8: R0 = 1
		busA.write(
			8,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 9: CONTROL bit 0 = transmit
		busA.write(
			9,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				canA
			)
		);

		// 10: remain here after transmission
		busA.write(
			10,
			SimpleISA::encode(
				SimpleISA::Opcode::JMP,
				0,
				0,
				10
			)
		);

		// --------------------------------------------
		// Node B normal firmware
		//
		// Enable CAN receive interrupt, then idle
		// in a firmware loop.
		// --------------------------------------------

		// 0: R0 = 1
		busB.write(
			0,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				0,
				0,
				1
			)
		);

		// 1: RX_INTERRUPT_ENABLE = 1
		busB.write(
			1,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				0,
				0,
				canB + 22
			)
		);

		// 2: idle loop
		busB.write(
			2,
			SimpleISA::encode(
				SimpleISA::Opcode::JMP,
				0,
				0,
				2
			)
		);

		// --------------------------------------------
		// Node B IRQ 1 vector
		// --------------------------------------------

		busB.write(
			SimpleCPU::InterruptVectorBase + 1,
			100
		);

		// --------------------------------------------
		// Node B CAN interrupt handler
		// --------------------------------------------

		// 100: R1 = RX_ID
		busB.write(
			100,
			SimpleISA::encode(
				SimpleISA::Opcode::LOAD,
				1,
				0,
				canB + 12
			)
		);

		// 101: R2 = RX_LENGTH
		busB.write(
			101,
			SimpleISA::encode(
				SimpleISA::Opcode::LOAD,
				2,
				0,
				canB + 13
			)
		);

		// 102: R3 = RX_DATA_0
		busB.write(
			102,
			SimpleISA::encode(
				SimpleISA::Opcode::LOAD,
				3,
				0,
				canB + 14
			)
		);

		// 103: R4 = RX_DATA_1
		busB.write(
			103,
			SimpleISA::encode(
				SimpleISA::Opcode::LOAD,
				4,
				0,
				canB + 15
			)
		);

		// 104: R5 = 2
		//
		// CONTROL bit 1 acknowledges/pops
		// the current RX frame.
		busB.write(
			104,
			SimpleISA::encode(
				SimpleISA::Opcode::MOVI,
				5,
				0,
				2
			)
		);

		// 105: acknowledge RX frame
		busB.write(
			105,
			SimpleISA::encode(
				SimpleISA::Opcode::STORE,
				5,
				0,
				canB
			)
		);

		// 106: return from interrupt
		busB.write(
			106,
			SimpleISA::encode(
				SimpleISA::Opcode::RETI
			)
		);

		nodeA.getCPU().reset();
		nodeB.getCPU().reset();

		// Enough global cycles for:
		// - B to enable its CAN IRQ
		// - A to construct/transmit the frame
		// - B to execute the complete handler
		for (int i = 0; i < 25; ++i)
		{
			simulation.advanceTime(
			std::chrono::milliseconds(1)
		);
		}

		// Node B firmware should have received the
		// actual contents of Node A's frame.
		assert(
			cpuB.getRegister(1) == 0x123
		);

		assert(
			cpuB.getRegister(2) == 2
		);

		assert(
			cpuB.getRegister(3) == 0xAA
		);

		assert(
			cpuB.getRegister(4) == 0x55
		);

		// Handler acknowledged the frame, so the
		// RX queue should now be empty.
		assert(
			busB.read(canB + 1) == 0
		);

		// IRQ should have been accepted and cleared
		// by the CPU.
		assert(
			!nodeB
				.getInterruptController()
				.isPending(1)
		);
	}
	
	{
		Simulation simulation;

		assert(
			simulation.getCurrentTime() ==
			std::chrono::nanoseconds(0)
		);

		simulation.advanceTime(
			std::chrono::milliseconds(5)
		);

		assert(
			simulation.getCurrentTime() ==
			std::chrono::milliseconds(5)
		);
	}
	
	{
		Simulation simulation;

		BoardConfig configA;
		configA.clockHz = 16'000'000;

		BoardConfig configB;
		configB.clockHz = 32'000'000;

		Simulator& nodeA =
			simulation.createNode(configA);

		Simulator& nodeB =
			simulation.createNode(configB);
		
		nodeA.getBus().write(
			0,
			SimpleISA::encode(
				SimpleISA::Opcode::HALT
			)
		);

		nodeB.getBus().write(
			0,
			SimpleISA::encode(
				SimpleISA::Opcode::HALT
			)
		);

		nodeA.getCPU().reset();
		nodeB.getCPU().reset();

		simulation.advanceTime(
			std::chrono::milliseconds(1)
		);

		assert(
			simulation.getCurrentTime() ==
			std::chrono::milliseconds(1)
		);

		assert(
			nodeA.getClock().getCycle() ==
			16'000
		);

		assert(
			nodeB.getClock().getCycle() ==
			32'000
		);
	}
	
    std::cout << "Simulation tests passed.\n";

    return 0;
}