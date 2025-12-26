# 代码逻辑需求

请按模块化方式编写代码，不要全部塞在 main.c 中。

## 模块 1: 舵机驱动 (servo_driver.c/.h)
- 封装 `Set_Servo_Angle(uint8_t channel, float angle)` 函数。
- 逻辑: 将 0-180 度转换为 TIM4 的 CCR 值 (500-2500)。
- 注意: 启动时需调用 `HAL_TIM_PWM_Start`。

## 模块 2: 超声波驱动 (hcsr04.c/.h)
- 封装 `HCSR04_Read_Distance(void)` 函数。
- 逻辑:
  1. 拉高 PB0 10us 以上触发。
  2. 使用 TIM3 输入捕获测量 PB1 高电平时间。
  3. 转换公式: Distance (cm) = Time (us) * 0.017。
  4. 需处理超时情况，防止程序卡死。

## 模块 3: OV2640 驱动 (ov2640.c/.h) - **难点**
- **注意**: CubeMX 仅生成了 I2C 和 DCMI 初始化，**你需要提供 OV2640 的 SCCB 寄存器配置列表**。
- 逻辑:
  1. 硬件复位: 拉低 PC10 再拉高。
  2. SCCB 初始化: 通过 I2C2 读取 Sensor ID (应为 0x2642) 验证连接。
  3. 写寄存器: 配置为 JPEG 模式或 RGB565 模式 (根据需求，先选 JPEG 方便传输)。
  4. 启动 DCMI DMA 传输到内存缓冲区。

## 主程序逻辑 (main.c)
1. 初始化所有外设。
2. 打印 "System Ready"。
3. While(1) 循环:
   - 控制舵机转到一个新角度。
   - 延时稳定。
   - 触发超声波测距。
   - (可选) 触发摄像头拍照 (DCMI Start)。
   - 通过串口打印: "Angle: 90, Dist: 125cm"。
   - 延时。