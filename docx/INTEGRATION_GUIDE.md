# STM32F407 智能云台系统 - 集成指导

## 📦 代码生成完成清单

以下是已经生成的所有驱动模块：

### ✅ 已完成的文件

#### 1. 舵机驱动
- `Core/Inc/servo_driver.h`
- `Core/Src/servo_driver.c`

#### 2. 超声波驱动（非阻塞中断方式）
- `Core/Inc/hcsr04.h`
- `Core/Src/hcsr04.c`

#### 3. OV2640 摄像头驱动
- `Core/Inc/ov2640_regs.h` - 完整寄存器配置数组
- `Core/Inc/ov2640.h`
- `Core/Src/ov2640.c`

#### 4. printf 重定向
- `Core/Src/usart.c` - 已添加 `fputc()` 重定向函数

#### 5. 主程序逻辑
- `Core/Src/main.c` - 完整的应用层代码
- `Core/Src/stm32f4xx_it.c` - 添加了 TIM3 中断回调

---

## 🔧 构建和编译

### 1. 使用 CMake 构建（推荐）

```bash
cd build
cmake ..
make -j4
```

### 2. 可能遇到的编译问题

#### 问题 1: `fputc` 链接器重定义错误

如果遇到 `multiple definition of fputc` 错误，需要修改链接器选项。

**解决方法：**

在 `CMakeLists.txt` 中，找到链接器标志部分，添加以下选项：

```cmake
target_link_libraries(${PROJECT_NAME}.elf
    ...
    -specs=nosys.specs
    -specs=nano.specs
)
```

或者在 `syscalls.c` 中注释掉默认的 `_write` 函数（如果有）。

#### 问题 2: printf 浮点数不显示

STM32 默认不启用浮点数打印。需要在链接器选项中添加：

```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float")
```

或者在 **STM32CubeIDE** 中：
- 右键项目 → Properties → C/C++ Build → Settings
- MCU GCC Linker → Miscellaneous
- 在 "Other flags" 中添加：`-u _printf_float`

---

## 🚀 烧录和测试

### 1. 烧录固件

使用 ST-Link 或 J-Link：

```bash
# 使用 OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/your_project.elf verify reset exit"

# 或使用 st-flash
st-flash write build/your_project.bin 0x08000000
```

### 2. 串口监控

使用串口工具连接到 **USART1**（PA9/PA10），波特率 **115200**。

推荐工具：
- Linux: `minicom`, `screen`, `picocom`
- Windows: PuTTY, Tera Term
- macOS: `screen`, CoolTerm

**示例（Linux）：**

```bash
sudo picocom -b 115200 /dev/ttyUSB0
```

### 3. 预期输出

系统启动后，串口应输出：

```
========================================
 STM32F407 Smart Gimbal System
 Firmware Version: 1.0
========================================

[INIT] Initializing servos...
[OK] Servos initialized
[INIT] Initializing ultrasonic sensor...
[OK] Ultrasonic sensor initialized
[INIT] Initializing OV2640 camera...
[OK] OV2640 initialized (JPEG QQVGA 160x120)

[SYSTEM READY]

Pan: 0.0 deg | Tilt: 90.0 deg | Distance: 125.3 cm
Pan: 30.0 deg | Tilt: 90.0 deg | Distance: 98.7 cm
Pan: 60.0 deg | Tilt: 90.0 deg | Distance: 210.5 cm
...
```

---

## 🛠️ 硬件连接检查清单

在上电前，请确认以下硬件连接：

### ✅ 舵机（TIM4 PWM）
- Pan 舵机 → PD12
- Tilt 舵机 → PD13
- 舵机供电：5V（外部电源，不要用 STM32 的 3.3V！）

### ✅ 超声波（HC-SR04）
- TRIG → PB0
- ECHO → PB1
- VCC → 5V
- GND → GND

### ✅ OV2640 摄像头
- SCCB (I2C2):
  - SCL → PB10
  - SDA → PB11
- DCMI 数据线：
  - D0-D7 → PC6, PC7, PC8, PC9, PC11, PB6, PB8, PB9
  - HREF → PA4
  - VSYNC → PB7
  - PCLK → PA6
- 控制引脚：
  - RESET → PC10
  - PWDN → PC12
- 电源：3.3V

### ✅ 串口（USART1）
- TX → PA9
- RX → PA10

---

## 📝 代码使用说明

### 1. 修改扫描角度范围

在 `main.c` 中修改：

```c
/* 修改角度增量（默认 30 度） */
currentPanAngle += 30.0f;  // 改为你想要的步进角度

/* 修改扫描范围（默认 0-180 度） */
if (currentPanAngle > 180.0f) {
    currentPanAngle = 0.0f;
}
```

### 2. 启用摄像头捕获

在 `main.c` 中，取消注释以下代码块：

```c
/* 去掉注释符号 */
if (camStatus == OV2640_OK) {
    memset(imageBuffer, 0, IMAGE_BUFFER_SIZE);
    if (OV2640_StartCapture(imageBuffer, IMAGE_BUFFER_SIZE) == OV2640_OK) {
        HAL_Delay(100);
        OV2640_StopCapture();
        printf("  [Camera] Image captured\r\n");
    }
}
```

### 3. 调整超声波测距范围

在 `hcsr04.c` 中修改超时时间：

```c
#define HCSR04_TIMEOUT  38000  // 默认 38ms（约 6.5m）
                                // 增大数值以支持更远距离
```

### 4. 更改摄像头分辨率

在 `main.c` 的初始化代码中：

```c
/* 选项 1: QQVGA 160x120（默认，最小）*/
OV2640_Init(OV2640_FORMAT_JPEG_QQVGA);

/* 选项 2: QVGA 320x240（更高分辨率）*/
OV2640_Init(OV2640_FORMAT_JPEG_QVGA);
```

---

## 🐛 调试技巧

### 1. 舵机不转动
- 检查 TIM4 PWM 是否启动（应在 `Servo_Init()` 中自动启动）
- 用示波器测量 PD12/PD13 的 PWM 信号：
  - 频率应为 50Hz（20ms 周期）
  - 脉宽范围：0.5ms - 2.5ms

### 2. 超声波始终返回 0
- 检查 TIM3 输入捕获中断是否启用
- 确认 `HAL_TIM_IC_CaptureCallback` 被调用（可在函数内加 printf）
- 检查 ECHO 引脚接线

### 3. OV2640 初始化失败
- 检查 I2C2 总线（用逻辑分析仪或示波器）
- 确认 RESET 和 PWDN 引脚状态
- 尝试读取 Chip ID：
  ```c
  uint16_t id;
  OV2640_ReadID(&id);
  printf("Chip ID: 0x%04X\r\n", id);  // 应该是 0x2642
  ```

### 4. printf 不输出
- 检查 USART1 TX (PA9) 接线
- 确认波特率：115200
- 确保添加了 `-u _printf_float` 链接器选项（如果打印浮点数）

---

## 📚 API 参考

### 舵机驱动 API

```c
void Servo_Init(void);
void Servo_SetAngle(uint32_t channel, float angle);  // 0-180°
void Servo_SetPulse(uint32_t channel, uint16_t pulse);  // 500-2500

/* 通道定义 */
#define SERVO_PAN_CHANNEL   TIM_CHANNEL_1  // 水平
#define SERVO_TILT_CHANNEL  TIM_CHANNEL_2  // 垂直
```

### 超声波驱动 API

```c
void HCSR04_Init(void);
void HCSR04_Trigger(void);  // 非阻塞触发
float HCSR04_GetDistance(void);  // 返回距离（cm）
HCSR04_Status_t HCSR04_GetStatus(void);  // 获取测量状态

/* 状态枚举 */
typedef enum {
    HCSR04_IDLE = 0,
    HCSR04_MEASURING,
    HCSR04_READY,
    HCSR04_TIMEOUT
} HCSR04_Status_t;
```

### OV2640 驱动 API

```c
OV2640_Status_t OV2640_Init(OV2640_Format_t format);
OV2640_Status_t OV2640_ReadID(uint16_t *id);
OV2640_Status_t OV2640_StartCapture(uint8_t *buffer, uint32_t buffer_size);
OV2640_Status_t OV2640_StopCapture(void);

/* 格式枚举 */
typedef enum {
    OV2640_FORMAT_JPEG_QQVGA = 0,  // 160x120
    OV2640_FORMAT_JPEG_QVGA,       // 320x240
} OV2640_Format_t;
```

---

## ✨ 下一步改进建议

1. **添加上位机通信协议**
   定义一套结构化的数据格式（如 JSON），方便上位机解析。

2. **实现 DCMI JPEG 帧检测**
   添加 DCMI 帧完成中断，自动识别 JPEG 文件结束标志（0xFF 0xD9）。

3. **添加 SD 卡存储**
   将图像保存到 SD 卡（通过 SDIO 或 SPI）。

4. **实现多角度扫描算法**
   同时控制水平和垂直舵机，实现 3D 空间扫描。

5. **添加低功耗模式**
   在无任务时进入睡眠模式，由定时器唤醒。

---

## 📞 支持

如遇到问题，请检查：
1. CubeMX 配置是否与硬件映射文档一致
2. 引脚连接是否正确
3. 供电是否充足（特别是舵机需要外部 5V 电源）

祝调试顺利！🎉
