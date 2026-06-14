# ─────────────────────────────────────────────────────────────────────────────
# Makefile — STM32F407 BLDC Firmware
#
# Requires: arm-none-eabi-gcc (GNU Arm Embedded Toolchain)
# Install:  https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
#           or: winget install Arm.GnuArmEmbeddedToolchain
# ─────────────────────────────────────────────────────────────────────────────

# ── Toolchain ────────────────────────────────────────────────────────────────
PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
AS       := $(PREFIX)gcc -x assembler-with-cpp
OBJCOPY  := $(PREFIX)objcopy
OBJDUMP  := $(PREFIX)objdump
SIZE     := $(PREFIX)size

# ── Target ───────────────────────────────────────────────────────────────────
TARGET   := bldc_firmware
BUILD    := build

# ── CPU flags ────────────────────────────────────────────────────────────────
# -mcpu=cortex-m4     : Cortex-M4 instruction set
# -mthumb             : Generate Thumb-2 code (compact encoding for flash)
# -mfpu=fpv4-sp-d16   : Hardware FPU present on STM32F4
# -mfloat-abi=hard    : Use FPU registers for float arguments (ABI)
CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# ── Source files ─────────────────────────────────────────────────────────────
C_SRCS := \
    src/main.c          \
    src/system_clock.c  \
    src/gpio.c          \
    src/pwm.c           \
    src/uart.c          \
    src/motor.c

ASM_SRCS := \
    startup/startup_stm32f407.s

OBJECTS := $(addprefix $(BUILD)/, $(C_SRCS:.c=.o))
OBJECTS += $(addprefix $(BUILD)/, $(ASM_SRCS:.s=.o))

# ── Include paths ─────────────────────────────────────────────────────────────
INCLUDES := -Icmsis -Iinclude

# ── Compiler flags ────────────────────────────────────────────────────────────
# -Wall -Wextra        : Enable broad warnings
# -Werror              : Treat warnings as errors (zero-warning policy)
# -Os                  : Optimise for size (common for embedded firmware)
# -ffunction-sections  : Place each function in its own section (enables LTO GC)
# -fdata-sections      : Same for data
# -std=c11             : Use C11; allows _Static_assert, anonymous structs, etc.
CFLAGS := $(CPU_FLAGS) \
          -Wall -Wextra -Werror \
          -Os \
          -ffunction-sections \
          -fdata-sections \
          -std=c11 \
          -ffreestanding \
          -fno-common \
          $(INCLUDES)

# ── Linker flags ──────────────────────────────────────────────────────────────
# -T              : Linker script
# -Wl,--gc-sections : Remove unused sections (requires -ffunction/data-sections)
# -Wl,-Map        : Emit map file for size analysis
# -nostdlib       : Do not link against standard C runtime (bare-metal)
# -specs=nano.specs + -specs=nosys.specs : Use newlib-nano, stub syscalls
LDFLAGS := $(CPU_FLAGS) \
           -T linker/stm32f407.ld \
           -Wl,--gc-sections \
           -Wl,-Map=$(BUILD)/$(TARGET).map \
           -nostdlib \
           -specs=nano.specs \
           -specs=nosys.specs

# ── Default target ────────────────────────────────────────────────────────────
.PHONY: all clean flash size dump

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).hex
	@$(SIZE) $(BUILD)/$(TARGET).elf

# ── Link ──────────────────────────────────────────────────────────────────────
$(BUILD)/$(TARGET).elf: $(OBJECTS)
	@echo "[LD]  $@"
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# ── Binary and Intel HEX output ───────────────────────────────────────────────
$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	@echo "[BIN] $@"
	@$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	@echo "[HEX] $@"
	@$(OBJCOPY) -O ihex $< $@

# ── Compile C sources ────────────────────────────────────────────────────────
$(BUILD)/src/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "[CC]  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ── Assemble startup ──────────────────────────────────────────────────────────
$(BUILD)/startup/%.o: startup/%.s
	@mkdir -p $(dir $@)
	@echo "[AS]  $<"
	@$(AS) $(CPU_FLAGS) -c $< -o $@

# ── Utility targets ───────────────────────────────────────────────────────────
size: $(BUILD)/$(TARGET).elf
	$(SIZE) -A $<

dump: $(BUILD)/$(TARGET).elf
	$(OBJDUMP) -d -S $< > $(BUILD)/$(TARGET).lst

# Flash via OpenOCD (ST-Link V2 — adjust interface if using a different probe)
flash: $(BUILD)/$(TARGET).bin
	openocd \
	  -f interface/stlink.cfg \
	  -f target/stm32f4x.cfg \
	  -c "program $(BUILD)/$(TARGET).elf verify reset exit"

clean:
	@rm -rf $(BUILD)
	@echo "Cleaned."
