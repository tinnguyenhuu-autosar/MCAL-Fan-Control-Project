# ============================================
#  STM32F103C8T6 + SPL + Renode support
#  
# ============================================

# ===========================
# Project & toolchain
# ===========================
BUILDDIR      := build
TARGET_NAME   := FIRMWARE

TARGET        := $(BUILDDIR)/$(TARGET_NAME)

CROSS         ?= arm-none-eabi-
CC            := $(CROSS)gcc
AS            := $(CROSS)gcc
OBJCOPY       := $(CROSS)objcopy
OBJDUMP       := $(CROSS)objdump
SIZE          := $(CROSS)size

# ===========================
# MCU / CMSIS / SPL
# ===========================
# STM32F103 (Cortex-M3)
CPUFLAGS      := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

# Default Defines
DEFINES_BASE  := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 \
                 -DRTE_DEVICE_STDPERIPH_RCC -DRTE_DEVICE_STDPERIPH_GPIO

# Detect Build Mode (Thêm define RUN_ON_RENODE nếu chạy Renode)
ifeq ($(TARGET_ENV),renode)
  DEFINES_MODE := -DRUN_ON_RENODE
else
  DEFINES_MODE :=
endif

DEFINES       := $(DEFINES_BASE) $(DEFINES_MODE)

# Include Directories
INC_DIRS := \
    . \
    MCAL/Config \
    MCAL/IoHwAb \
    MCAL/Adc \
    MCAL/Dio \
    MCAL/Port \
    MCAL/Pwm \
    MCAL/Types \
    MCAL/platform/include \
    MCAL/platform \
    MCAL/platform/bsp/cmsis \
    MCAL/platform/debug \
    MCAL/platform/spl/inc

INCLUDES := $(addprefix -I, $(INC_DIRS))

# ===========================
# C/ASM/LD flags
# ===========================
CFLAGS_COMMON := -O0 -g3 -Wall -Wextra -Wno-unused-parameter \
                 -ffreestanding -fno-builtin \
                 -ffunction-sections -fdata-sections \
                 -MMD -MP

CFLAGS        := $(CPUFLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS_COMMON)
ASFLAGS       := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp

# Linker Script
LDSCRIPT      := MCAL/platform/bsp/linker/stm32f103.ld

LDFLAGS       := -T$(LDSCRIPT) -nostartfiles -nostdlib -static \
                 -Wl,--gc-sections -Wl,-Map=$(TARGET).map
LDFLAGS      += -specs=nano.specs -specs=nosys.specs

LDLIBS        := -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

# ===========================
# Sources Configuration
# ===========================
SRCS_C := \
  $(wildcard MCAL/Config/*.c) \
  $(wildcard MCAL/IoHwAb/*.c) \
  $(wildcard MCAL/Adc/*.c) \
  $(wildcard MCAL/Dio/*.c) \
  $(wildcard MCAL/Port/*.c) \
  $(wildcard MCAL/Pwm/*.c) \
  $(wildcard MCAL/platform/debug/*.c) \
  $(wildcard MCAL/platform/bsp/cmsis/*.c) \
  $(wildcard MCAL/platform/spl/src/*.c) \
  $(wildcard *.c)

# Startup Code
SRCS_S := \
  MCAL/platform/bsp/startup_stm32f10x_md.s

# ===========================
# Objects / Deps
# ===========================
OBJS_C := $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS_C))
OBJS_S := $(patsubst %.s,$(BUILDDIR)/%.o,$(filter %.s,$(SRCS_S))) \
          $(patsubst %.S,$(BUILDDIR)/%.o,$(filter %.S,$(SRCS_S)))

OBJS   := $(OBJS_C) $(OBJS_S)
DEPS   := $(OBJS_C:.o=.d)

# ===========================
# Default goal
# ===========================
.PHONY: all
all: $(TARGET).bin size

# ===========================
# Compile rules
# ===========================
$(BUILDDIR)/%.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@mkdir -p "$(dir $@)"
	$(AS) $(CPUFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	@mkdir -p "$(dir $@)"
	$(AS) $(ASFLAGS) -c $< -o $@

# ===========================
# Link
# ===========================
$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p "$(BUILDDIR)"
	$(CC) $(CPUFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ===========================
# BIN/HEX/SIZE/Listing
# ===========================
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

.PHONY: size
size: $(TARGET).elf
	$(SIZE) --format=berkeley $<

.PHONY: list
list: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).list

# ===============================
# Nạp firmware (OpencOCD)
# ===============================
.PHONY: flash
flash: $(TARGET).bin
	openocd \
		-f interface/stlink.cfg \
		-f target/stm32f1x.cfg \
		-c "adapter speed 2000" \
		-c "program $(abspath $<) 0x08000000 verify reset exit"

# ===============================
# Clean
# ===============================
.PHONY: clean
clean:
	
	@echo "Cleaning target: $(TARGET_NAME)..."
	@rm -f "$(TARGET).elf"
	@rm -f "$(TARGET).bin"
	@rm -f "$(TARGET).hex"
	@rm -f "$(TARGET).map"
	@rm -f "$(TARGET).list"
	@rm -f $(DEPS)
	@echo "Done."

.PHONY: clean-all
clean-all:
	@echo "Cleaning entire build directory..."
	@rm -rf "$(BUILDDIR)"
	@echo "Done."
