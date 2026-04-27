# ================================================================
# Makefile for DA14531 Blinky Project
#
# Copyright (c) 2025 gilroy619
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# ================================================================
#
# Environment:
#   - Tested with MSYS2 MINGW64
#   - GCC: arm-none-eabi-gcc 10.3.1 (20210824)
#   - Renesas DA145xx SDK: v6.0.24.1464
#
# Usage:
#   make            # Build ELF, HEX, BIN, IMG
#   make clean      # Remove build artifacts
#
# Configuration:
#   - Set SDK_ROOT before building:
#       export SDK_ROOT=/path/to/DA145xx_SDK/6.0.24.1464   (Linux/MSYS)
#   - Or edit the SDK_ROOT variable directly in this Makefile
#
# Notes:
#   - Requires ARM GCC toolchain in PATH
#   - Generates .elf, .hex, .bin, .img, and .map files
# ================================================================

PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SDK_ROOT ?= /path/to/DA145xx_SDK/6.0.24.1464
BUILD_DIR := build

# Toolchain
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

# Project name 
PROJECT := blinky

# Targets
TARGET  := $(BUILD_DIR)/$(PROJECT).elf
HEX     := $(BUILD_DIR)/$(PROJECT).hex
BIN     := $(BUILD_DIR)/$(PROJECT).bin
IMG     := $(BUILD_DIR)/$(PROJECT).img
MAP     := $(BUILD_DIR)/$(PROJECT).map

# Include platform and common related files for compiler
INCLUDE_DIRS := $(SDK_ROOT)/sdk/platform/arch/main \
				$(SDK_ROOT)/sdk/platform/include \
				$(SDK_ROOT)/sdk/platform/include/CMSIS/5.9.0/CMSIS/Core/Include \
				$(SDK_ROOT)/sdk/platform/arch \
				$(SDK_ROOT)/sdk/platform/arch/boot \
				$(SDK_ROOT)/sdk/platform/arch/compiler \
				$(SDK_ROOT)/sdk/platform/arch/ll \
				$(SDK_ROOT)/sdk/platform/system_library/include \
				$(SDK_ROOT)/sdk/platform/core_modules/nvds/api \
				$(SDK_ROOT)/sdk/platform/core_modules/rf/api \
				$(SDK_ROOT)/sdk/platform/driver/gpio \
				$(SDK_ROOT)/sdk/platform/driver/uart \
				$(SDK_ROOT)/sdk/platform/driver/dma \
				$(SDK_ROOT)/sdk/platform/driver/syscntl \
				$(SDK_ROOT)/sdk/platform/driver/hw_otpc \
				$(SDK_ROOT)/sdk/platform/driver/adc \
				$(SDK_ROOT)/sdk/platform/driver/trng \
				$(SDK_ROOT)/sdk/platform/utilities/otp_cs \
				$(SDK_ROOT)/sdk/platform/utilities/otp_hdr \
				$(SDK_ROOT)/sdk/common_project_files \
				$(SDK_ROOT)/third_party/rand \
				$(SDK_ROOT)/third_party/irng
					
# Library directories for linker
LIB_DIRS := $(SDK_ROOT)/sdk/common_project_files/misc \
			$(SDK_ROOT)/sdk/platform/system_library/output/IAR


# ----------------------------------------------------------------
# Compiler / Linker flags
# ----------------------------------------------------------------
DEFS = -D__DA14531__ -D__NON_BLE_EXAMPLE__ -D__EXCLUDE_ROM_CHACHA20__

CFLAGS = -mcpu=cortex-m0plus -march=armv6-m -mthumb \
	-O0 -fmessage-length=0 -fsigned-char \
	-ffunction-sections -fdata-sections -Wall \
	-Isrc/config \
	$(addprefix -I,$(INCLUDE_DIRS)) \
	-include "src/config/da1458x_config_basic.h" \
	-include "src/config/da1458x_config_advanced.h" \
	$(DEFS) 
	
# LD Script src and final location
LDSCRIPT_SRC := $(SDK_ROOT)/sdk/common_project_files/ldscripts/ldscript_DA14531_e2s.lds.S
LDSCRIPT     := $(PROJECT_ROOT)ldscript_DA14531.lds

# LD Script pre-processing defines and directories
LDSCRIPT_DEFS := -D__DA14531__ -D__NON_BLE_EXAMPLE__ 

LDSCRIPT_INCS := -I$(PROJECT_ROOT)/src/config \
    			 -I$(SDK_ROOT)/sdk/common_project_files \
    			 -I$(SDK_ROOT)/sdk/common_project_files/misc

# LD script post process flags/ .map file generation
LDFLAGS = -mcpu=cortex-m0plus -march=armv6-m -mthumb -O0 \
          -fsigned-char -ffunction-sections -fdata-sections -Wall \
		  -T $(LDSCRIPT) -Xlinker --gc-sections \
		  $(addprefix -L,$(LIB_DIRS)) \
		  -Wl,-Map,$(MAP) --specs=nosys.specs -l:da14531.a

# ----------------------------------------------------------------
# Source files: app + required SDK platform files
# ----------------------------------------------------------------
SRC_C := src/main.c \
		 src/platform/user_periph_setup.c \
		 $(SDK_ROOT)/sdk/platform/driver/dma/dma.c \
		 $(SDK_ROOT)/sdk/platform/driver/gpio/gpio.c \
		 $(SDK_ROOT)/sdk/platform/driver/hw_otpc/hw_otpc_531.c \
		 $(SDK_ROOT)/sdk/platform/driver/syscntl/syscntl.c \
		 $(SDK_ROOT)/sdk/platform/driver/uart/uart.c \
		 $(SDK_ROOT)/sdk/platform/driver/uart/uart_utils.c \
		 $(SDK_ROOT)/sdk/platform/arch/main/hardfault_handler.c \
		 $(SDK_ROOT)/sdk/platform/arch/main/nmi_handler.c \
		 $(SDK_ROOT)/sdk/platform/arch/boot/startup_DA14531.c \
		 $(SDK_ROOT)/sdk/platform/arch/boot/system_DA14531.c \
		 $(SDK_ROOT)/sdk/platform/arch/main/arch_system.c \
		 $(SDK_ROOT)/sdk/platform/utilities/otp_cs/otp_cs.c  

# Object file
OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_C))

# MKIMG tool
MKIMAGE := $(SDK_ROOT)/binaries/host/windows/mkimage/mkimage.exe

# ----------------------------------------------------------------
# Rules
# ----------------------------------------------------------------

# Default build target
all: $(TARGET) $(HEX) $(BIN)

# ELF build rule
$(TARGET): $(OBJ) $(LDSCRIPT)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
	
# HEX from ELF
$(HEX): $(TARGET)
	$(OBJCOPY) -O ihex --gap-fill 0xff $< $@

# BIN from ELF
$(BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@

# IMG from BIN
$(IMG): $(BIN)
	$(MKIMAGE) single $< $(SDK_ROOT)/sdk/platform/include/sdk_version.h $@

# Compile C into BUILD_DIR
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	
# Preprocess linker script
$(LDSCRIPT): $(LDSCRIPT_SRC)
	$(CC) -E -P -c $(LDSCRIPT_INCS) $(LDSCRIPT_DEFS) $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(MAP) $(BIN) $(IMG) $(HEX) $(LDSCRIPT)

.PHONY: all clean
