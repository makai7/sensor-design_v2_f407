#!/bin/bash
# STM32F407 Build Script for Arch Linux
# Requires: arm-none-eabi-gcc, cmake, make

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}STM32F407 Smart Gimbal System - Build${NC}"
echo -e "${GREEN}========================================${NC}\n"

# Check if arm-none-eabi-gcc is installed
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo -e "${RED}[ERROR] arm-none-eabi-gcc not found!${NC}"
    echo -e "${YELLOW}Install it with: sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib${NC}"
    exit 1
fi

echo -e "${GREEN}[INFO] Toolchain: $(arm-none-eabi-gcc --version | head -1)${NC}\n"

# Clean build directory
if [ -d "build" ]; then
    echo -e "${YELLOW}[CLEAN] Removing old build directory...${NC}"
    rm -rf build
fi

mkdir -p build
cd build

# Configure with CMake using ARM toolchain
echo -e "${GREEN}[CMAKE] Configuring project...${NC}"
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_UNIT_TESTS=OFF \
      ..

# Build
echo -e "\n${GREEN}[BUILD] Compiling firmware...${NC}"
make -j$(nproc)

# Check if build succeeded
if [ -f "V2.2_F407.elf" ] && [ -f "V2.2_F407.bin" ]; then
    echo -e "\n${GREEN}========================================${NC}"
    echo -e "${GREEN}Build SUCCESS!${NC}"
    echo -e "${GREEN}========================================${NC}"

    # Show binary size
    echo -e "\n${YELLOW}[INFO] Firmware size:${NC}"
    arm-none-eabi-size V2.2_F407.elf

    echo -e "\n${YELLOW}[INFO] Output files:${NC}"
    echo -e "  - ELF: build/V2.2_F407.elf"
    echo -e "  - BIN: build/V2.2_F407.bin"
    echo -e "  - MAP: build/V2.2_F407.map"
else
    echo -e "\n${RED}========================================${NC}"
    echo -e "${RED}Build FAILED!${NC}"
    echo -e "${RED}========================================${NC}"
    exit 1
fi
