# MicroSim

MicroSim is a modular microcontroller simulation framework written in C++.

The goal of MicroSim is to provide configurable simulated microcontrollers that behave as self-contained embedded controllers. A simulated controller can execute firmware, interact with peripherals, receive external input signals, and produce output signals without needing to know anything about the environment in which it is being used.

MicroSim is intended for experimentation, prototyping, robotics simulation, embedded-system development, and testing generic microcontroller configurations.

## Project Goals

MicroSim is designed around several core goals:

- Simulate configurable microcontroller hardware.
- Keep simulated hardware separate from assembly syntax, parsers, compilers, and other software toolchains.
- Keep the microcontroller independent from the external simulated world.
- Allow external environments to drive and observe controller pins through a small public interface.
- Support ordinary real-time execution without requiring the host application to calculate elapsed time itself.
- Preserve explicit cycle/time advancement for deterministic testing and simulation hosts that manage their own time.
- Support multiple independent simulated controllers in the same system.
- Allow future communication between controllers through interfaces such as CAN.
- Allow users to approximate different classes of microcontrollers by changing characteristics such as clock speed, RAM size, GPIO count, electrical parameters, and memory-map locations.
- Keep peripherals modular so additional devices can be added without redesigning the core simulator.

MicroSim is not intended to be an exact transistor-level, circuit-level, or brand-specific reproduction of a commercial microcontroller. Instead, it aims to model embedded-controller behavior and resource constraints at a useful level of abstraction.

## Current Status

MicroSim is under active development. Implemented components currently include:

- System bus
- Configurable RAM size and base address
- GPIO with runtime-configurable pin count
- Voltage-based `Pin` model
- Pin-selected GPIO register interface
- Configurable GPIO and timer base addresses
- Timer peripheral
- Simulation clock
- Clock-frequency-based time advancement
- Real-time synchronization using a monotonic host clock
- Public pin-voltage accessors for external environments
- Simple CPU
- Simple instruction set
- Board configuration
- Multi-node simulation infrastructure
- Automated tests for major subsystems
- End-to-end external pin integration testing

The project should currently be considered experimental and its APIs may change during development.

## Hardware / Software Separation

MicroSim separates simulated hardware from the software used to program it.

The CPU executes machine instructions defined by its instruction set. Assembly-language syntax is not part of the simulated hardware. This allows future assemblers, compilers, parsers, or other development tools to target a MicroSim CPU without requiring changes to the CPU, peripherals, or other simulated hardware.

The current SimpleISA instruction set exists to develop and test the simulator and does not prevent other instruction sets or software toolchains from being added in the future.

## Time and Execution

One MicroSim clock cycle represents one hardware clock cycle of the configured simulated controller.

`BoardConfig::clockHz` determines the relationship between elapsed time and MCU cycles. For example, advancing a 16 MHz controller by 1 ms executes 16,000 hardware cycles, while a 32 MHz controller executes 32,000 cycles during the same interval.

Advancing cycles does not skip hardware activity. The CPU and other clock-driven devices are advanced during those cycles.

MicroSim currently supports three execution styles:

- `advanceCycles(n)` explicitly executes a known number of MCU cycles.
- `advanceTime(duration)` converts an elapsed duration into MCU cycles using the configured clock frequency.
- Real-time mode uses `std::chrono::steady_clock` to measure real elapsed host time and feeds that duration into the same time-advancement mechanism through `startRealTime()`, `updateRealTime()`, and `stopRealTime()`.

Real-time mode measures elapsed time rather than host processor cycles, so MCU timing is not based on how many CPU cycles the host machine happens to execute. The host application only needs to call `updateRealTime()` from its normal loop; it does not calculate elapsed time itself.

Explicit cycle and duration advancement remain useful for deterministic tests and for external simulators that already manage their own simulation time.

Firmware does not directly read the host clock. It experiences time through simulated MCU execution and peripherals such as timers.

## External Simulation Boundary

A MicroSim controller does not model the physical world around it.

For example, MicroSim does not need to know whether an input voltage represents wheel speed, temperature, pressure, a switch, a sensor, or some other world quantity. The external environment supplies pin voltages, firmware determines how those values are used, and the external environment decides what controller outputs affect.

The current world-facing pin interface is deliberately small:

```cpp
bot.setPinVoltage(pin, voltage);
double voltage = bot.getPinVoltage(pin);
```

For an ordinary real-time host loop, this can be combined with:

```cpp
bot.startRealTime();

while (running)
{
    bot.setPinVoltage(inputPin, inputVoltage);
    bot.updateRealTime();
    double outputVoltage = bot.getPinVoltage(outputPin);
}

bot.stopRealTime();
```

An external simulator with its own time-management system can instead call `advanceTime()`, while deterministic tests can call `advanceCycles()`.

The external environment does not need to know MicroSim's GPIO register map, CPU registers, instruction encoding, or internal peripheral organization merely to exchange physical pin signals with the controller.

An end-to-end switched-light integration test now verifies this boundary: the external side changes a switch pin voltage and observes a light pin voltage, while firmware inside the simulated controller performs the GPIO reads, decision making, and GPIO writes.

## GPIO and Pins

GPIO uses a runtime-sized `std::vector<Pin>`. The number of pins is therefore a board configuration property rather than being limited by a 32-bit register width.

The generic GPIO peripheral currently uses a pin-selected register interface:

| Offset | Register | Behavior |
| --- | --- | --- |
| 0 | `PIN_SELECT` | Selects the pin addressed by subsequent GPIO operations |
| 1 | `DIRECTION` | Reads or sets the selected pin direction |
| 2 | `OUTPUT` | Reads or drives the selected output pin LOW/HIGH |
| 3 | `INPUT` | Reads the selected pin as LOW/HIGH and is CPU read-only |

This allows a generic board to expose 8, 100, or more pins without dividing the user-visible GPIO model into artificial 32-pin banks. CPU word width remains an implementation property and does not define the total number of simulated pins.

Pins carry simulated voltage values. Digital HIGH/LOW interpretation uses the board's configured logic voltage and digital HIGH threshold. Detailed circuit behavior and the physical meaning of those voltages remain outside MicroSim's core scope.

## Configurable Boards

`BoardConfig` currently provides configuration for:

- CPU clock frequency
- RAM size
- GPIO pin count
- timer count
- logic voltage
- digital HIGH threshold
- RAM base address
- GPIO base address
- timer base address

The long-term goal is to let users create generic board configurations approximating the resources and performance characteristics needed for a real embedded project without requiring MicroSim to reproduce a particular commercial MCU exactly.

## Building

MicroSim uses CMake and requires a C++20-compatible compiler.

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

On systems where a specific CMake generator is required, select the appropriate generator when configuring the build.

## Running Tests

After building:

```sh
ctest --test-dir build --output-on-failure
```

The current test suite covers RAM, Bus, GPIO, Timer, CPU, Simulation behavior, real-time mode state, external pin access, and an end-to-end external-world GPIO path.

## Project Structure

```text
MicroSim/
├── docs/       Project design and architecture documentation
├── include/    Public headers
├── src/        Implementation source
├── tests/      Automated tests
├── CMakeLists.txt
└── README.md
```

## Future Direction

Likely future development includes:

- Refinement of the external integration API as additional hardware interfaces are added
- More configurable peripheral construction
- Additional timers and peripherals
- Analog input / ADC support
- PWM and other pin functions
- Interrupt handling
- CAN or similar controller-to-controller communication
- Additional CPU architectures or instruction sets
- External assembler/compiler tooling
- More flexible board profiles and peripheral layouts

These items describe project direction and should not be assumed to be implemented unless documented otherwise.

## Scope

MicroSim focuses on the simulated embedded controller.

Detailed physical-world simulation, vehicle dynamics, robotics physics, circuit simulation, sensor physics, and similar environment-specific behavior are outside the core scope of the project. Those systems can instead interact with MicroSim through its external interfaces.