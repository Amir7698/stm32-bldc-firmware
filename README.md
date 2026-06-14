# STM32F407 Bare-Metal BLDC Motor Controller

Register-level firmware for a three-phase BLDC motor controller, written in C with no HAL, no CubeMX generated code, and no RTOS. Every peripheral is configured by directly writing hardware registers with comments citing the relevant section of the STM32F407 Reference Manual (RM0090).

## Motivation

Most embedded firmware tutorials rely on STM32CubeMX and HAL to abstract away the hardware. This project deliberately strips those layers away. The goal is to demonstrate complete control over the MCU: from the PLL configuration that sets the core to 168 MHz, through the dead-time register that prevents gate shoot-through, to the CRC field that guards UART frame integrity. This is the kind of code that runs on real motor drives in industrial and aerospace applications.

## Hardware Target

| Item | Value |
|------|-------|
| MCU | STM32F407VGT6 (Cortex-M4, 168 MHz, 1 MB Flash, 192 KB SRAM) |
| Board | ST Nucleo-F407ZG or any STM32F407 breakout |
| Debug probe | ST-Link V2 (on-board on Nucleo) or J-Link |
| Motor voltage | 12–48 V (external gate driver required) |

## Pin Assignment

| Pin | Function | Notes |
|-----|----------|-------|
| PA8 | TIM1_CH1 | Phase U high-side |
| PB13 | TIM1_CH1N | Phase U low-side |
| PA9 | TIM1_CH2 | Phase V high-side |
| PB14 | TIM1_CH2N | Phase V low-side |
| PA10 | TIM1_CH3 | Phase W high-side |
| PB15 | TIM1_CH3N | Phase W low-side |
| PA2 | USART2_TX | Debug output at 115200 |
| PA3 | USART2_RX | Debug input |
| PA0 | EXTI0 | Direction input (rising + falling) |
| PA1 | EXTI1 | Enable input (rising + falling) |
| PC13 | GPIO_OUT | Status LED (1 Hz heartbeat) |

## Architecture

```
main.c
  └── SystemInit()        system_clock.c   168 MHz PLL, SysTick 1 ms
  └── gpio_init()         gpio.c           AF config, EXTI0/1 setup
  └── uart_init()         uart.c           USART2 115200 8N1
  └── pwm_init()          pwm.c            TIM1 3-phase center-aligned 20 kHz
  └── motor_init()        motor.c          State machine init
  └── loop: motor_run()   motor.c          State transitions + UART report
        EXTI0_IRQHandler                   Direction pin edge
        EXTI1_IRQHandler                   Enable pin edge
        TIM1_UP_TIM10_IRQHandler           PWM period heartbeat
        SysTick_Handler   system_clock.c   1 ms tick counter
```

## State Machine

```
          enable asserted
  IDLE ─────────────────► RUNNING
   ▲                          │
   │  fault timeout +         │  enable deasserted
   │  enable deasserted        │  OR fault condition
   │                          ▼
   └──────────────────────  FAULT  (2 s lockout)
```

- **IDLE**: PWM outputs disabled, counter running but MOE cleared.
- **RUNNING**: MOE set, 50% duty cycle startup, RPM estimated from duty.
- **FAULT**: MOE cleared immediately, 2 s lockout prevents auto-restart. Cleared only when enable is deasserted after lockout expires.

## Register-Level Decisions

### Clock Tree (system_clock.c)

The HSE (8 MHz external crystal on Nucleo) feeds the PLL. HSI (internal RC) is deliberately avoided because its ±1% tolerance would produce UART baud rate errors above the 3% margin.

```
HSE 8 MHz → PLLM /8 → 1 MHz VCO input
→ PLLN ×336 → 336 MHz VCO output
→ PLLP /2  → 168 MHz SYSCLK
→ PLLQ /7  → 48 MHz USB clock
APB1 /4    → 42 MHz (USART2 source)
APB2 /2    → 84 MHz (TIM1 source × 2 = 168 MHz)
```

Flash wait states (5 WS) are set **before** the clock switch — writing them after would cause a transient read at wrong wait states.

### TIM1 PWM (pwm.c)

TIM1 is the only advanced-control timer on STM32F4 and the only one that supports complementary outputs with hardware dead-time. Key choices:

- **Center-aligned mode** (`CMS = 01`): the counter counts up then down, producing a triangle carrier. All three channel PWMs are symmetric around the peak, which halves the harmonic content compared to edge-aligned.
- **ARR = 4200**: `168 MHz / (2 × 20 kHz) = 4200`. The `/2` factor comes from center-aligned counting both ways.
- **Dead-time = 84 cycles ≈ 500 ns**: set in `BDTR[DTG]`. Calculated for typical gate charge of 20–50 nC and gate driver source/sink current of 2 A. The 500 ns margin covers worst-case turn-off delay of common N-channel MOSFETs.
- **MOE in BDTR**: the main output enable is set only in `pwm_enable()`, not in `pwm_init()`. This prevents gate outputs from activating before the state machine reaches RUNNING.
- **ARPE + OC1PE**: shadow registers prevent a duty-cycle update mid-cycle from producing a truncated pulse, which would appear as a voltage spike on motor windings.

### USART2 (uart.c)

BRR = `f_PCLK / (16 × baud)`. APB1 is 42 MHz, giving `42000000 / (16 × 115200) = 22.786`. Mantissa 22, fraction `round(0.786 × 16) = 13`. Encoded as `BRR = (22 << 4) | 13 = 0x16D`.

### UART Frame + CRC-8

Each report frame is 6 bytes:

```
[0xAA][RPM_H][RPM_L][DUTY][STATE][CRC8]
```

CRC-8/MAXIM (polynomial 0x31, reflected) is computed over bytes 0–4. A host parser can detect single-byte corruption with 100% probability and most multi-byte errors. The 0xAA start marker is chosen because it has alternating bits, useful for baud-rate autosync on a logic analyser.

### EXTI (gpio.c)

Both rising and falling edges are captured on PA0/PA1. The ISR reads the actual `IDR` pin level rather than inferring it from which edge was detected — this avoids a race condition where a very fast pulse would cause the edge flag to disagree with the current level.

EXTI pending bits must be cleared by **writing 1** to `EXTI->PR` (not clearing to 0). This is a common bug source — the manual states "cleared by writing a 1" in §10.3.6.

## UART Frame Protocol

A Python host parser example:

```python
import serial, struct

ser = serial.Serial('/dev/ttyUSB0', 115200)

def crc8(data):
    crc = 0
    for b in data:
        crc = TABLE[crc ^ b]  # use CRC-8/MAXIM table
    return crc

while True:
    raw = ser.read(6)
    if raw[0] != 0xAA:
        continue
    rpm, duty, state, crc = struct.unpack('>HBBx', raw[1:])
    # verify: crc8(raw[:5]) == raw[5]
```

## Build Instructions

### Prerequisites

1. **ARM GNU Toolchain** — download from [developer.arm.com](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) or:
   ```
   winget install Arm.GnuArmEmbeddedToolchain
   ```
   Ensure `arm-none-eabi-gcc` is on your `PATH`.

2. **Make** — install via [MSYS2](https://www.msys2.org/) or use the bundled MinGW:
   ```
   winget install MSYS2.MSYS2
   ```

3. **OpenOCD** (for flashing) — download from [openocd.org](https://openocd.org/) or:
   ```
   winget install OpenOCD.OpenOCD
   ```

### Build

```bash
make          # builds build/bldc_firmware.elf, .bin, .hex
make size     # prints section sizes
make dump     # disassembly → build/bldc_firmware.lst
make clean    # removes build/
```

### Flash

```bash
make flash    # uses OpenOCD with ST-Link
```

### Debug (VS Code)

1. Install recommended extensions (VS Code will prompt on first open).
2. Press `F5` — selects "Debug (ST-Link via OpenOCD)" configuration.
3. Set breakpoints in any `.c` file; peripheral registers visible in the XPERIPHERALS panel via the SVD file.

> **SVD file**: Download `STM32F407.svd` from [ST's CMSIS pack](https://www.st.com/en/microcontrollers-microprocessors/stm32f407vg.html) and place it at `.vscode/STM32F407.svd` for full register visibility in Cortex-Debug.

## Project Structure

```
stm32-bldc-firmware/
├── cmsis/
│   └── stm32f4xx.h          Hand-written CMSIS register definitions
├── include/
│   ├── system_clock.h
│   ├── gpio.h
│   ├── pwm.h
│   ├── uart.h
│   └── motor.h
├── src/
│   ├── main.c               Entry point, init sequence
│   ├── system_clock.c       PLL → 168 MHz, SysTick
│   ├── gpio.c               AF config, EXTI
│   ├── pwm.c                TIM1 complementary PWM + dead-time
│   ├── uart.c               USART2 + CRC-8 framing
│   └── motor.c              State machine + ISRs
├── startup/
│   └── startup_stm32f407.s  Vector table, .data copy, .bss zero
├── linker/
│   └── stm32f407.ld         Flash/SRAM memory map
├── .vscode/
│   ├── c_cpp_properties.json IntelliSense for ARM cross-compiler
│   ├── tasks.json           Build / flash / size tasks
│   ├── launch.json          Cortex-Debug ST-Link + J-Link configs
│   └── extensions.json      Recommended extensions
├── Makefile
└── README.md
```

## Author

Amirhossein Mohtashami — M.Sc. Electrical Engineering, STM32/FreeRTOS background.
