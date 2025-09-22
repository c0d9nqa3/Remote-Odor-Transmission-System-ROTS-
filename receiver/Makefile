# ROTS Receiver Makefile for STM32F407VET6
# Author: ROTS Team
# Date: 2024

# Project name
PROJECT = rots_receiver

# Target MCU
MCU = STM32F407VET6

# Compiler and tools
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# Directories
SRC_DIR = src
INC_DIR = src
CONFIG_DIR = config
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Include directories
INCLUDES = -I$(INC_DIR) -I$(CONFIG_DIR) -I$(STM32_CUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc \
           -I$(STM32_CUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
           -I$(STM32_CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
           -I$(STM32_CUBE_DIR)/Drivers/CMSIS/Include

# Compiler flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -DSTM32F407xx -DUSE_HAL_DRIVER
CFLAGS += -Wall -Wextra -Wpedantic -Werror
CFLAGS += -O2 -g3 -ggdb
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(INCLUDES)

# Linker flags
LDFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDFLAGS += -specs=nano.specs -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage

# Linker script
LINKER_SCRIPT = STM32F407VETx_FLASH.ld

# Default target
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

# Create build directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files
$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

# Generate hex file
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@

# Generate binary file
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary -S $< $@

# Generate disassembly
$(BUILD_DIR)/$(PROJECT).lst: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJDUMP) -h -S $< > $@

# Flash to MCU
flash: $(BUILD_DIR)/$(PROJECT).hex
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $< verify reset exit"

# Debug with GDB
debug: $(BUILD_DIR)/$(PROJECT).elf
	arm-none-eabi-gdb $< -ex "target remote localhost:3333" -ex "monitor reset halt" -ex "load" -ex "monitor reset halt"

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build project (default)"
	@echo "  flash    - Flash to MCU"
	@echo "  debug    - Start GDB debug session"
	@echo "  clean    - Clean build files"
	@echo "  help     - Show this help"

# Dependencies
-include $(OBJECTS:.o=.d)

.PHONY: all flash debug clean help
