# 硬件资源映射表

## 1. 舵机 (Servo / Gimbal)
- **控制器**: SG90 舵机 x 2
- **定时器**: TIM4 (APB1 84MHz)
- **PWM 频率**: 50Hz (Prescaler=83, Period=19999)
- **引脚**:
  - Pan (水平): PD12 (TIM4_CH1)
  - Tilt (垂直): PD13 (TIM4_CH2)

## 2. 超声波传感器 (Ultrasonic)
- **型号**: HC-SR04
- **定时器**: TIM3 (APB1 84MHz, 1us 精度)
- **引脚**:
  - TRIG (触发): PB0 (GPIO Output, 默认 Low)
  - ECHO (回响): PB1 (TIM3_CH4 Input Capture)

## 3. 摄像头 (Camera)
- **型号**: OV2640
- **接口**: DCMI (8-bit 并行) + DMA2_Stream1
- **控制总线 (SCCB)**: I2C2 (PB10/PB11)
- **控制引脚**:
  - RESET: PC10 (Active Low, 已配置上拉 GPIO_PIN_SET)
  - PWDN: PC12 (Active High, 已配置下拉 GPIO_PIN_RESET)
- **数据引脚**: PC6-PC9, PC11, PB6, PB8, PB9, PA4(HREF), PA6(PCLK), PB7(VSYNC)

## 4. 通信 (Communication)
- **接口**: USART1 (115200 bps)
- **引脚**: PA9 (TX), PA10 (RX)