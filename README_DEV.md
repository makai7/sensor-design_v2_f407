# STM32F407 智能云台系统 - 开发指南

## 项目概述

这是一个基于 STM32F407VET6 的智能云台系统，具有以下功能：
- **舵机控制**：双轴云台（水平/垂直）
- **超声波测距**：HC-SR04 非阻塞式测量
- **图像采集**：OV2640 摄像头 JPEG 输出
- **数据传输**：UART 115200 波特率传输遥测和图像数据

## 开发环境（Arch Linux）

### 必需软件包

```bash
# 安装 ARM 工具链
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib arm-none-eabi-gdb

# 安装构建工具
sudo pacman -S cmake make

# 安装调试工具（可选）
yay -S jlink-software-and-documentation  # JLink
# 或
sudo pacman -S openocd  # OpenOCD

# 安装 Python 串口库（用于图像接收）
sudo pacman -S python-pyserial
```

### VSCode 扩展

推荐安装以下扩展：
- **Cortex-Debug**：ARM Cortex 调试支持
- **C/C++**：代码智能提示
- **CMake Tools**：CMake 支持

## 构建固件

### 方法 1：使用构建脚本（推荐）

```bash
./build.sh
```

### 方法 2：手动构建

```bash
rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      ..
make -j$(nproc)
```

### 启用单元测试

```bash
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_UNIT_TESTS=ON \
      ..
make -j$(nproc)
```

## 烧录固件

### 使用 JLink

```bash
# 自动烧录
JLinkExe -device STM32F407VE -if SWD -speed 4000 -autoconnect 1 \
         -CommanderScript .vscode/flash.jlink

# 或在 VSCode 中运行任务：Flash with JLink
```

### 使用 OpenOCD

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
        -c "program build/V2.2_F407.bin 0x08000000 verify reset exit"
```

## 调试

### VSCode 调试（JLink SWD）

1. 连接 JLink 调试器到 STM32 的 SWD 接口（SWDIO, SWCLK, GND, VCC）
2. 在 VSCode 中按 F5 或选择 "Run and Debug" → "STM32 JLink Debug"
3. 支持断点、单步调试、变量查看等功能

### 串口调试

```bash
# 使用 Python 接收脚本（推荐）
python3 serial_receiver.py /dev/ttyUSB0 115200

# 或使用 minicom
minicom -D /dev/ttyUSB0 -b 115200

# 或使用 screen
screen /dev/ttyUSB0 115200
```

## 接收图像数据

### 使用提供的 Python 脚本

```bash
python3 serial_receiver.py /dev/ttyUSB0 115200
```

接收到的图像将保存到 `captured_images/` 目录，文件名格式：`img_YYYYMMDD_HHMMSS_NNNN.jpg`

### 输出格式

```
[HH:MM:SS.mmm] Pan: 30.0 deg | Tilt: 90.0 deg | Distance: 45.2 cm
[HH:MM:SS.mmm]   [Camera] Image captured

[IMAGE] Receiving image #1...
[IMAGE] IMG_END (size: 5432 bytes)
[OK] Saved: captured_images/img_20250127_143052_0000.jpg (5432 bytes)
```

## 硬件连接

### 舵机（TIM4 PWM）
- Pan（水平）：PD12 → TIM4_CH1
- Tilt（垂直）：PD13 → TIM4_CH2
- 电源：5V（外部供电，共地）

### HC-SR04 超声波（TIM3 Input Capture）
- TRIG：PB0（GPIO 输出）
- ECHO：PB1（TIM3_CH4 输入捕获）
- VCC：5V
- GND：GND

### OV2640 摄像头（I2C2 + DCMI）
- SCCB（I2C2）：PB10（SCL），PB11（SDA）
- DCMI：8 位并行数据 + HSYNC/VSYNC/PCLK
- RESET：PC10（低电平有效）
- PWDN：PC12（高电平有效）

### UART1（调试串口）
- TX：PA9
- RX：PA10
- 波特率：115200，8N1

### JLink SWD 调试
- SWDIO：PA13
- SWCLK：PA14
- GND：GND
- VCC：3.3V

## 故障排除

### 编译错误

**问题**：`Error: no such instruction: 'wfe'`

**解决**：确保使用了正确的工具链文件：
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake ..
```

### printf 无输出

**解决方案**：已修复，确保使用最新代码（usart.c 实现了 `__io_putchar()`）

### 图像数据损坏

**原因**：UART 缓冲区溢出或波特率不匹配

**解决方案**：
1. 确认波特率设置正确（115200）
2. 使用流控制或增加 UART 超时
3. 检查 JPEG 数据完整性（查看 IMG_END 的 size）

### 超声波测量超时

**检查**：
1. 接线是否正确（TRIG → PB0，ECHO → PB1）
2. 5V 电源是否稳定
3. 测量距离是否在有效范围内（2cm - 400cm）

## 性能指标

### 固件大小
```
   text    data     bss     dec     hex filename
  45976     472   31576   78024   130c8 V2.2_F407.elf
```

- **Flash 使用率**：8.86% (46 KB / 512 KB)
- **RAM 使用率**：24.45% (32 KB / 128 KB)

### 系统参数
- **系统时钟**：168 MHz
- **APB1 时钟**：42 MHz（定时器 × 2 = 84 MHz）
- **APB2 时钟**：84 MHz
- **舵机 PWM 频率**：50 Hz（20ms 周期）
- **超声波定时器分辨率**：1 μs

## 代码结构

```
sensor-design_v2_f407-1/
├── Core/
│   ├── Src/
│   │   ├── main.c              # 主程序
│   │   ├── servo_driver.c      # 舵机驱动
│   │   ├── hcsr04.c            # HC-SR04 驱动
│   │   ├── ov2640.c            # OV2640 驱动
│   │   ├── test_suite.c        # 单元测试
│   │   ├── usart.c             # UART 配置（printf 重定向）
│   │   └── ...                 # 其他外设初始化
│   └── Inc/                    # 头文件
├── Drivers/                    # STM32 HAL 库
├── cmake/
│   └── gcc-arm-none-eabi.cmake # ARM 工具链配置
├── .vscode/
│   ├── launch.json             # 调试配置
│   ├── tasks.json              # 构建任务
│   └── flash.jlink             # JLink 烧录脚本
├── build.sh                    # 构建脚本
├── serial_receiver.py          # 串口图像接收脚本
├── CMakeLists.txt              # CMake 配置
└── README_DEV.md               # 本文档
```

## 版本历史

### v1.3（当前版本）
- ✅ 修复 printf/scanf 重定向（支持 GCC 和其他编译器）
- ✅ 添加图像 UART 传输功能（JPEG 检测和分块发送）
- ✅ 优化超声波轮询逻辑（减少 CPU 占用）
- ✅ 修复编译配置（使用 ARM 工具链）
- ✅ 添加 VSCode 调试配置（JLink SWD）
- ✅ 添加 Python 串口接收脚本

### v1.2
- 添加单元测试框架
- 实现所有核心驱动

### v1.1
- 基础功能实现

## 许可证

STMicroelectronics 提供的代码遵循其软件许可协议。
用户自定义代码（驱动程序、应用逻辑）可自行选择许可证。

## 技术支持

如有问题，请检查：
1. 硬件连接是否正确
2. 固件版本是否最新
3. 工具链是否正确安装
4. 串口权限（添加用户到 `uucp` 或 `dialout` 组）

```bash
# Arch Linux 串口权限
sudo usermod -aG uucp $USER
# 重新登录后生效
```

---

**开发环境**：Arch Linux + VSCode + arm-none-eabi-gcc
**调试方式**：JLink SWD + UART 串口
**最后更新**：2025-01-27
