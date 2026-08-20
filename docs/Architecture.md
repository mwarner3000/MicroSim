# MicroSim Architecture

## 1. Purpose

MicroSim is a modular embedded-controller simulation framework. Its primary responsibility is to simulate the internal behavior and external hardware interface of configurable microcontrollers.

A MicroSim controller should behave as though the environment connected to it is the real world. The controller itself should not require knowledge of the simulation environment that provides its inputs or consumes its outputs. This allows the same simulated controller and firmware to operate in different host environments.

## 2. Design Principles

### Hardware and software separation

The simulated CPU executes machine code. Assembly syntax, parsers, assemblers, compilers, and other toolchain components remain separate from simulated hardware.

### Environment independence

The controller contains no application-specific world state. It does not know whether a voltage represents vehicle speed, temperature, pressure, position, or another external quantity.

### Real elapsed time is independent of host CPU cycles

Real-time execution is based on elapsed time measured by a monotonic host clock, not on host processor cycle counts. MCU hardware timing is still determined by the configured MCU clock frequency.

### Explicit timing remains available

Tests and external simulators may advance a controller explicitly by hardware cycles or elapsed duration rather than using real-time mode.

### Modular peripherals

Peripherals communicate through defined interfaces and should not depend directly on CPU implementation details.

### Multiple hardware communication paths

Memory-mapped reads and writes are the normal CPU/device configuration and data path, but they are not the only hardware interaction. Interrupt requests travel independently through the interrupt controller. Future DMA, bus-master, and inter-controller mechanisms may likewise require paths that are not ordinary CPU reads and writes.

### Configurable hardware

Board configuration describes hardware characteristics such as memory capacity, clock frequency, pin count, electrical parameters, and memory-map locations.

### Testable subsystems

Major components should be independently testable before integration into a complete controller.

## 3. High-Level Architecture

```text
                         External Environment
                                |
                        pin voltages / time
                                |
                                v
                         +-------------+
                         |  Simulator  |
                         | public API  |
                         +------+------+ 
                                |
                         +------v------+
                         |    Pins     |
                         +------+------+ 
                                |
                         +------v------+
                         | Peripherals |
                         | GPIO/Timer  |
                         +--+-------+--+
                            |       |
              registers/data|       |IRQ
                            v       v
                         +-----+  +-------------------+
                         | Bus |  |InterruptController|
                         +-+-+-+  +---------+---------+
                           | |              |
                     +-----+ +-----+        |
                     v             v        v
                   +-----+       +-----+  +-----+
                   | RAM |       | CPU |<-| IRQ |
                   +-----+       +-----+  +-----+
```

The diagram intentionally distinguishes memory-mapped peripheral traffic from interrupt signaling. A Timer is configured through the Bus but can asynchronously request CPU attention through `InterruptController`.

`Simulator` assembles these components into a functioning controller and provides the external pin/time boundary.

## 4. Board Configuration

`BoardConfig` currently contains:

- `clockHz`
- `ramWords`
- `gpioPins`
- `logicVoltage`
- `digitalHighThreshold`
- `ramBase`
- `gpioBase`
- `timerBase`

Configuration describes hardware being constructed rather than requiring source changes for each board. For example, two generic boards may differ in clock rate, RAM, GPIO count, voltage, and address layout without representing exact commercial MCU part numbers.

Peripheral multiplicity is not yet generalized through `BoardConfig`; the current `Simulator` constructs one Timer.

## 5. CPU and Instruction Set

The CPU executes machine instructions and interacts with memory and memory-mapped peripherals through the system bus. Assembly-language text is not interpreted by the CPU.

```text
Assembler / Compiler
        |
        v
   Machine Code
        |
        v
      CPU
        |
        v
       Bus
```

The current development CPU uses SimpleISA. SimpleISA is a development ISA and does not define the only language or CPU architecture MicroSim may support.

A future assembler may use different syntax while producing compatible machine code. Future CPUs may use different instruction sets without requiring unrelated peripherals to be rewritten.

SimpleCPU also supports the current interrupt mechanism described below, including interrupt entry and `RETI`.

## 6. Bus and Memory Map

The system bus routes memory-mapped reads and writes to attached devices implementing the common bus-device interface.

```text
CPU
 |
 v
Bus
 +-- RAM
 +-- GPIO
 +-- Timer
 +-- future peripherals
```

`BoardConfig` supplies base addresses for RAM, GPIO, and Timer. The simulator uses these configured locations when attaching the devices.

Devices should not require direct references to the CPU. A peripheral that needs to signal asynchronous work can instead request an IRQ through the interrupt controller.

## 7. Memory

RAM is a bus device. Its size is determined by board configuration when the simulator is constructed, and the CPU accesses it through the system bus.

The current RAM model stores 32-bit values indexed by bus address.

RAM also holds ordinary firmware and, for SimpleCPU, the memory-based interrupt vector table and interrupt handlers.

## 8. GPIO and Pins

### Physical pin model

`GPIO` owns a runtime-sized `std::vector<Pin>`. `BoardConfig::gpioPins` determines the number of pins created.

The physical/logical pin count is therefore not limited by the width of a CPU register. A generic GPIO device may contain 8, 100, or more pins without requiring the user to divide those pins into artificial 32-pin banks.

Each `Pin` currently stores simulated voltage and runtime direction. Pins default to input.

### Generic CPU-facing GPIO interface

The current generic GPIO peripheral uses four memory-mapped offsets:

| Offset | Register | Description |
| --- | --- | --- |
| 0 | `PIN_SELECT` | Select the pin used by subsequent GPIO operations |
| 1 | `DIRECTION` | Read/set selected pin direction (`0` input, `1` output) |
| 2 | `OUTPUT` | Read/drive selected output pin (`0` LOW, `1` HIGH) |
| 3 | `INPUT` | Read selected pin (`0` LOW, `1` HIGH); CPU read-only |

For example, to drive pin 73 HIGH, firmware conceptually performs:

```text
PIN_SELECT = 73
DIRECTION  = 1
OUTPUT     = 1
```

Internally this operates on `pins[73]`. There is no requirement for firmware to address a synthetic 32-pin bank merely because the CPU uses 32-bit values.

### External pin access

`Simulator` exposes:

```cpp
void setPinVoltage(std::size_t pin, double voltage);
double getPinVoltage(std::size_t pin) const;
```

These methods form the current physical-signal boundary between a MicroSim controller and its host environment. The host does not need to know which memory-mapped registers firmware uses to interact with the same pins.

### Input and output behavior

The external environment may provide a voltage to an input pin. GPIO compares the selected pin voltage against `digitalHighThreshold` when producing a digital input value.

For a configured output pin, the current generic GPIO maps digital LOW to 0 V and digital HIGH to the configured logic voltage.

The external environment may observe that voltage through `getPinVoltage()` and determine what it means in the simulated world.

### Electrical scope

Pin voltage is modeled relative to the simulated board reference. MicroSim does not determine whether a voltage represents an open circuit, sensor value, relay command, motor command, or other world behavior. Detailed circuit simulation is outside the current core scope.

## 9. Timer

The Timer is memory mapped and clock driven. It maintains a counter, period, enabled state, expiration state, and interrupt-enable state.

Its current register interface is:

| Offset | Register | Description |
| --- | --- | --- |
| 0 | `COUNTER` | Current counter value |
| 1 | `PERIOD` | Expiration period |
| 2 | `ENABLE` | Enables/disables the timer |
| 3 | `EXPIRED` | Expiration flag; writing 1 clears it |
| 4 | `INTERRUPT_ENABLE` | Enables/disables IRQ generation on expiration |

When the Timer reaches its period, it resets its counter and sets `EXPIRED`. If interrupt generation is enabled, it also calls `InterruptController::request()` with the interrupt number assigned when the Timer was constructed.

The Timer therefore knows its IRQ number but does not know what the CPU will do with it or what firmware handler it maps to.

The timer continues to participate in MCU clock advancement independently of whether the CPU is executing useful instructions.

## 10. Interrupt Architecture

### Interrupt controller

`InterruptController` stores pending interrupt state by interrupt number. Peripherals request or clear numbered IRQs without requiring direct access to the CPU.

When more than one interrupt is pending, the current controller returns the lowest numbered pending IRQ first. This is the current simple selection rule, not a complete configurable priority system.

### CPU interrupt entry

At the beginning of an instruction tick, SimpleCPU checks for a pending interrupt when it is not already servicing one. If an IRQ is pending, the CPU:

1. records the current program counter as its interrupt return address;
2. calculates the vector-table entry from `InterruptVectorBase + interruptNumber`;
3. reads the handler address through the Bus;
4. clears the selected pending interrupt;
5. marks itself as servicing an interrupt;
6. fetches and executes the first handler instruction.

Conceptually:

```text
Peripheral
    |
    | request IRQ N
    v
InterruptController
    |
    | pending IRQ N
    v
SimpleCPU
    |
    | read vector[N]
    v
Bus / RAM
    |
    | handler address
    v
Interrupt handler
```

The CPU does not know whether IRQ N came from a Timer, GPIO device, CAN controller, UART, or another future peripheral.

### Vector table

SimpleCPU currently uses a memory-based vector table beginning at `SimpleCPU::InterruptVectorBase` (`0x0100`). Each vector-table entry contains the firmware address of the corresponding handler.

```text
InterruptVectorBase + 0 -> IRQ 0 handler address
InterruptVectorBase + 1 -> IRQ 1 handler address
InterruptVectorBase + 2 -> IRQ 2 handler address
...
```

The current Simulator assigns IRQ 0 to the Timer.

### Returning from an interrupt

SimpleISA provides `RETI`. When executed during an interrupt handler, `RETI` restores the saved return program counter and clears the CPU's servicing-interrupt state so normal firmware execution can resume.

Executing `RETI` outside an interrupt handler is an error.

### Current interrupt limitations

The current interrupt implementation deliberately establishes the architecture without trying to model every MCU interrupt feature:

- interrupts do not nest;
- the lowest numbered pending IRQ is selected first rather than using configurable priorities;
- only the return program counter is automatically preserved;
- general registers and flags are not automatically saved/restored;
- interrupt entry does not currently consume a separately modeled latency cycle;
- SimpleCPU checks `halted` before checking pending interrupts, so `HALT` cannot currently be awakened by an IRQ;
- global interrupt masking and per-source CPU masks are not implemented.

These are SimpleCPU limitations, not requirements imposed on every future CPU model.

## 11. Clock and Time

### Fundamental cycle definition

One MicroSim cycle represents one hardware clock cycle of the configured MCU.

`BoardConfig::clockHz` determines the relationship between elapsed time and hardware cycles:

```text
cycles = elapsed seconds x clockHz
```

For example, a 16 MHz controller advances 16,000 cycles in 1 ms, while a 32 MHz controller advances 32,000 cycles in 1 ms.

### Advancing cycles

`Simulator::advanceCycles(N)` executes `N` simulated hardware cycles. It is not a time skip. Clock-driven hardware receives those cycles and may change state during them.

### Advancing time

`Simulator::advanceTime(std::chrono::nanoseconds)` converts requested elapsed time into hardware cycles using the configured clock frequency and then executes those cycles. Fractional-cycle timing is accumulated between calls.

### Real-time mode

For ordinary applications that should follow real elapsed time, `Simulator` provides:

```cpp
startRealTime();
updateRealTime();
stopRealTime();
isRealTimeRunning();
```

`updateRealTime()` measures elapsed monotonic host time with `std::chrono::steady_clock` and feeds that duration into the same advancement mechanism. The caller does not calculate elapsed time itself.

Real-time execution is not based on host processor cycles. The current interface is cooperative rather than background-threaded: the host calls `updateRealTime()` from its own loop.

### Firmware-visible time

Firmware does not directly read the host's `steady_clock`. The host clock determines how much MCU execution occurs in real-time mode; firmware experiences that passage of time through MCU cycles and simulated peripherals such as timers.

### Halted CPU versus MCU clock

A halted CPU does not imply that the MCU clock ceases to exist. Clock-driven peripherals may continue advancing while the CPU performs no further instruction fetches. In the current SimpleCPU implementation, however, a pending interrupt does not wake a halted CPU.

## 12. Simulator

`Simulator` represents one complete simulated controller instance. It owns and connects the board's current major components:

- Bus
- RAM
- GPIO
- InterruptController
- Timer
- SimpleCPU
- Clock

Construction order matters where components hold references to one another: the interrupt controller must exist before the Timer and CPU that reference it.

`Simulator` constructs components according to `BoardConfig`, attaches bus devices to configured address ranges, coordinates clock-driven execution, exposes explicit cycle/time advancement, provides cooperative real-time synchronization, and exposes external pin-voltage accessors.

`Simulator` should not contain environment-specific application logic.

## 13. External Environment Boundary

MicroSim stops at the controller boundary.

A vehicle/sensor model outside MicroSim might determine that a wheel-speed sensor produces 2.5 V. MicroSim receives only that electrical input:

```cpp
bot.setPinVoltage(sensorPin, 2.5);
```

Firmware interprets that signal according to the program running on the controller. The reverse applies to outputs through `getPinVoltage()`.

The current host-facing boundary therefore consists primarily of pin signals plus a choice of execution/time control:

```text
External Environment
       |
       +-- setPinVoltage()
       +-- getPinVoltage()
       |
       +-- execution control
             +-- updateRealTime()
             +-- advanceTime()
             +-- advanceCycles()
```

The external environment does not need to know the GPIO register map, CPU register layout, interrupt-vector layout, or instruction encoding merely to exchange physical signals with the controller.

### End-to-end switched-light test

`SimulationTests` contains an end-to-end integration case representing a switched light. The external side changes a switch voltage and observes a light voltage, while firmware configures GPIO, reads the switch, makes the decision, and drives the output.

This verifies the intended external boundary in both directions.

## 14. Multiple Controllers

A future system may contain multiple independent MicroSim instances connected by simulated communication hardware such as CAN.

Each controller remains an independent embedded system with its own CPU, memory, peripherals, firmware, configuration, clock state, and interrupt state.

CAN or similar controller-to-controller communication belongs to the simulated embedded system and is distinct from the external world/environment interface.

The existing multi-node `Simulation` infrastructure is an early step toward this capability; its final role and API are not yet settled.

## 15. Testing

Current test targets include:

- `RAMTests`
- `BusTests`
- `GPIOTests`
- `TimerTests`
- `CPUTests`
- `SimulationTests`
- `InterruptControllerTests`

Subsystem tests verify components independently. Integration tests verify important complete paths, including:

```text
External Input -> Pin -> GPIO -> Bus -> CPU -> Firmware
Firmware -> CPU -> Bus -> GPIO -> Pin -> External Output
```

and the asynchronous interrupt path:

```text
Timer
  |
  v
InterruptController
  |
  v
SimpleCPU
  |
  v
Vector Table
  |
  v
Firmware Handler
  |
  v
RETI -> interrupted firmware
```

Timing tests cover explicit time advancement and real-time mode state. Real-time tests should avoid relying on exact host sleep/scheduling intervals where deterministic tests can verify the same underlying behavior more reliably.

## 16. Current Limitations

MicroSim is early in development. Current limitations include:

- Only one Timer object is currently constructed.
- Interrupt behavior is intentionally basic as described in the interrupt section.
- The GPIO electrical model is intentionally simplified.
- The generic GPIO interface does not yet model alternate functions, analog conversion, pull-ups, output-driver characteristics, or pin multiplexing.
- Real-time execution currently requires cooperative `updateRealTime()` calls from the host loop; there is no background execution thread.
- CAN and other multi-controller communication are not yet implemented.
- DMA and multiple bus masters are not yet implemented.

These limitations should be addressed incrementally rather than by introducing assumptions that unnecessarily constrain future board models.

## 17. Out of Scope

The core MicroSim project is not intended to become:

- a vehicle physics simulator
- a robotics physics engine
- a circuit/SPICE simulator
- a mechanical simulator
- a sensor-physics simulator
- an exact emulator of a specific commercial MCU

External systems may implement those capabilities and communicate with MicroSim through defined interfaces.

## 18. Future Architecture

Likely future areas include:

- refinement of the external integration API as additional hardware interfaces are added;
- more configurable peripheral construction and layouts;
- additional timers and peripherals;
- analog input / ADC support;
- PWM and alternate pin functions;
- more advanced interrupt masking, priorities, context handling, or wake behavior where required;
- CAN or similar controller-to-controller communication;
- additional CPU architectures and instruction sets;
- assembler/compiler tooling;
- DMA and multiple bus masters where a concrete use case requires them.

Future features should preserve the central separation between simulated controller hardware, firmware/toolchains, and the external simulation environment.