
And I'd replace the existing architecture file with this.

### `docs/Architecture.md`

# MicroSim Architecture

## 1. Purpose

MicroSim is a modular embedded-controller simulation framework.

Its primary responsibility is to simulate the internal behavior and external
hardware interface of configurable microcontrollers.

A MicroSim controller should behave as though the environment connected to it is
the real world. The controller itself should not require knowledge of the
simulation environment that provides its inputs or consumes its outputs.

This allows the same simulated controller and firmware to potentially operate in
many different host environments.

---

## 2. Design Principles

MicroSim follows several architectural principles.

### Hardware and software separation

The simulated CPU executes machine instructions.

Assembly-language syntax, parsers, assemblers, compilers, and other software
toolchain components should remain separate from the simulated hardware.

### Environment independence

The simulated controller does not contain application-specific world state.

It should not know whether an input represents vehicle speed, temperature,
position, pressure, or another external quantity.

The host environment is responsible for translating its world state into signals
that the controller can receive.

### Modular peripherals

Peripherals communicate through defined interfaces and should not depend directly
on CPU implementation details.

### Bus-based communication

The CPU communicates with RAM and memory-mapped peripherals through the system
bus rather than directly accessing individual device implementations.

### Configurable hardware

Board configuration should determine characteristics such as memory capacity,
clock frequency, available peripherals, pin count, and electrical parameters.

### Deterministic execution

Given the same initial state and external inputs, simulation behavior should be
repeatable.

### Testable subsystems

Major components should be testable independently before being integrated into a
complete simulated controller.

---

## 3. High-Level Architecture

Conceptually, a simulated controller is organized as:

```text
                    External Environment
                           │
                    input/output signals
                           │
                           ▼
                    ┌──────────────┐
                    │     Pins     │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │ Peripherals  │
                    │ GPIO / Timer │
                    │    / etc.    │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │     Bus      │
                    └───┬──────┬───┘
                        │      │
                  ┌─────▼─┐  ┌─▼──────┐
                  │  CPU  │  │  RAM   │
                  └───────┘  └────────┘
```

The Simulator class currently assembles these components into a functioning
controller.

##4. Board Configuration

BoardConfig describes characteristics of a simulated board.

Configuration currently contains values for:

clock frequency
RAM size
GPIO pin count
timer count
logic voltage
digital HIGH threshold

Not every configuration field is fully implemented yet.

The architectural intention is that configuration describes the hardware being
created rather than requiring changes to the simulator source code for each board.

For example, future board configurations might represent:

Board A
    Clock:      16 MHz
    RAM:        1K words
    GPIO:       8 pins
    Logic:      5 V


Board B
    Clock:      32 MHz
    RAM:        4K words
    GPIO:       20 pins
    Logic:      3.3 V

These are generic simulated controllers and are not required to reproduce a
specific commercial MCU exactly.

##5. CPU and Instruction Set

The CPU is responsible for executing machine instructions and interacting with the
system bus.

Instruction encoding and decoding are defined separately from the CPU execution
logic.

The current development CPU uses SimpleISA.

Conceptually:

Assembler / Compiler
        │
        ▼
   Machine Code
        │
        ▼
      CPU
        │
        ▼
       Bus

The CPU does not need to understand assembly-language text.

This separation is intentional. A future assembler may define its own syntax while
still producing machine instructions compatible with a particular CPU/ISA.

Likewise, future CPU architectures may introduce different instruction sets
without requiring unrelated peripherals to be rewritten.

##6. Bus

The system bus provides memory-mapped communication between the CPU and devices.

Devices implement a common bus-device interface and are attached to address
ranges.

Conceptually:

CPU
 │
 ▼
Bus
 ├── RAM
 ├── GPIO
 ├── Timer
 └── future peripherals

The bus is responsible for routing reads and writes to the device mapped to the
requested address.

Devices should not require direct references to the CPU.

The current memory map contains fixed peripheral locations. Making the memory map
configurable is a future architectural improvement.

##7. Memory

RAM is implemented as a bus device.

Its size is determined by board configuration when the simulator is constructed.

RAM is accessed through the system bus rather than directly by the CPU.

The current implementation stores 32-bit values indexed by bus address.

##8. GPIO and Pins

GPIO provides the translation between CPU-visible digital registers and simulated
pin state.

A pin currently contains:

a voltage
a runtime direction

Pin direction may be:

Input
Output

Pins default to input.

Firmware changes pin direction through GPIO register operations.

Input

An external environment may provide a voltage to an input pin.

GPIO compares that voltage against the board's configured digital HIGH threshold
when producing the CPU-visible digital input register.

Conceptually:

External environment
        │
        │ voltage
        ▼
       Pin
        │
        ▼
      GPIO
        │ threshold comparison
        ▼
     0 or 1
        │
        ▼
       CPU
Output

Firmware writes the GPIO output register.

For an output pin:

Digital LOW  -> 0 V
Digital HIGH -> configured logic voltage

The external environment can then observe that voltage.

Scope of pin modeling

A pin voltage represents voltage relative to the simulated board's reference.

MicroSim does not attempt to determine what that voltage means to the outside
world.

Detailed electrical circuit simulation is outside the current scope of the core
project.

##9. Timer

The timer is a memory-mapped peripheral and also participates in clock-driven
simulation.

It maintains internal state including:

counter
period
enabled state
expiration state

The timer advances as the simulator clock advances.

Additional timers and more advanced timer behavior may be added later.

##10. Clock and Time

Simulated hardware should receive time from the MicroSim execution model rather
than from wall-clock time.

This preserves deterministic execution and prevents the controller from depending
on the speed of the host computer.

Clock-driven devices implement the clockable interface and receive simulation
ticks.

A future external simulation environment should advance MicroSim according to the
integration model rather than MicroSim reading world time directly from that
environment.

##11. Simulator

Simulator represents a complete simulated controller instance.

It owns and connects the controller's major hardware components, including the:

bus
RAM
GPIO
timer
CPU
simulation clock

The simulator is responsible for constructing the board according to its
configuration and coordinating execution.

It should not contain environment-specific application logic.

##12. Multiple Controllers

A future embedded system may contain multiple independent MicroSim controller
instances.

For example:

               ┌───────────┐
               │   Bot A   │
               │ MicroSim  │
               └─────┬─────┘
                     │
                     │ CAN
                     │
               ┌─────┴─────┐
               │           │
        ┌──────▼─────┐ ┌───▼────────┐
        │   Bot B    │ │   Bot C    │
        │  MicroSim  │ │  MicroSim  │
        └────────────┘ └────────────┘

Each controller should remain an independent embedded system with its own CPU,
memory, peripherals, firmware, and clock state.

Communication mechanisms such as CAN should be modeled separately from the
external physical world.

This is similar to real systems in which multiple electronic control units manage
different subsystems and communicate over a shared network.

The existing multi-node Simulation infrastructure is an early step toward
supporting multiple controller instances. Its final role and API are not yet
settled.

##13. External Environment Boundary

MicroSim deliberately stops at the controller boundary.

Consider a simulated vehicle.

The vehicle simulator might determine:

Wheel speed = 40 mph

A sensor model outside MicroSim could translate that into:

Sensor output = 2.5 V

MicroSim receives only:

Input pin = 2.5 V

Firmware then interprets that input according to the program running on the
controller.

The reverse applies to outputs.

MicroSim might produce:

Output pin = 5 V

The external environment determines whether that voltage activates a relay,
changes motor behavior, turns on a lamp, or performs some other action.

This boundary is important because it allows MicroSim to be embedded in unrelated
simulation environments without embedding those environments into MicroSim.

##14. Testing

MicroSim uses subsystem tests to verify components independently.

Current test targets include:

RAMTests
BusTests
GPIOTests
TimerTests
CPUTests
SimulationTests

Tests should continue to be added whenever new hardware behavior is introduced or
a bug is discovered.

Integration tests should verify complete paths such as:

Firmware -> CPU -> Bus -> GPIO -> Pin

and:

External Input -> Pin -> GPIO -> Bus -> CPU
##15. Current Limitations

MicroSim is early in development.

Known architectural limitations currently include:

GPIO pin count is not yet fully dynamic.
Timer count is not yet fully configurable.
Clock-frequency configuration is not yet fully reflected throughout execution.
Peripheral addresses are currently fixed.
Large RAM configurations can conflict with fixed peripheral address ranges.
The GPIO electrical model is intentionally simplified.
External simulation APIs are not yet finalized.
Multi-controller communication such as CAN is not yet implemented.
Interrupt handling is not yet implemented.

These limitations should be addressed incrementally rather than by prematurely
adding complexity to the core architecture.

##16. Out of Scope

The core MicroSim project is not intended to become:

a vehicle physics simulator
a robotics physics engine
a circuit/SPICE simulator
a mechanical simulator
a sensor-physics simulator
a specific commercial MCU emulator

External systems may implement those capabilities and communicate with MicroSim
through defined interfaces.

##17. Future Architecture

Likely future areas of development include:

dynamic GPIO counts
configurable memory maps
ADC support
PWM
interrupts
additional peripherals
multiple timer instances
CAN communication
additional CPUs and instruction sets
assembler/compiler tooling
external integration APIs
configurable board profiles

These are architectural directions rather than promises of currently implemented
functionality.