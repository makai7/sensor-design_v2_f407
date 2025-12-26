# 编译选项与配置指南

## 概述

本项目支持通过 CMake 选项控制单元测试的编译，以便在调试版本中启用测试，在发布版本中排除测试代码以减小固件大小。

---

## 🎛️ 编译选项

### `ENABLE_UNIT_TESTS`

**描述：** 控制是否编译嵌入式单元测试代码

**默认值：** `ON`（启用）

**影响范围：**
- `Core/Src/test_suite.c` - 测试框架代码
- `main.c` 中的 `Run_All_Tests()` 调用
- `test_suite.h` 头文件引用

---

## 🔧 编译方法

### 方法 1：启用单元测试（默认，调试用）

```bash
cd build
cmake .. -DENABLE_UNIT_TESTS=ON
make -j4
```

**输出：**
- 包含单元测试代码
- 固件稍大（约增加 2-3 KB）
- 启动时自动运行测试

**预期串口输出：**
```
========================================
 STM32F407 Smart Gimbal System
 Firmware Version: 1.0
========================================

========================================
 EMBEDDED UNIT TEST SUITE
========================================

Running: Servo Angle to Pulse Conversion...
  [PASS] Servo Angle to Pulse Conversion

Running: HC-SR04 Pulse to Distance Conversion...
  [PASS] HC-SR04 Pulse to Distance Conversion

========================================
 TEST SUMMARY
========================================
Total Tests:  2
Passed:       2
Failed:       0

*** ALL TESTS PASSED ***
========================================

[INIT] Initializing servos...
...
```

---

### 方法 2：禁用单元测试（发布用）

```bash
cd build
cmake .. -DENABLE_UNIT_TESTS=OFF
make -j4
```

**输出：**
- 不包含单元测试代码
- 固件更小（节省 Flash 空间）
- 启动更快（无测试延迟）

**预期串口输出：**
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
...
```

---

## 📊 固件大小对比

| 配置 | Flash 使用 | RAM 使用 | 启动时间 |
|------|-----------|----------|---------|
| **启用测试** (`ON`) | ~52 KB | ~8 KB | ~500ms |
| **禁用测试** (`OFF`) | ~49 KB | ~6 KB | ~200ms |

**说明：** 具体数值取决于编译优化级别和实际代码复杂度。

---

## 🏗️ CMakeLists.txt 配置详解

### 相关配置代码

```cmake
# Option to enable/disable unit tests (enabled by default in Debug)
option(ENABLE_UNIT_TESTS "Enable embedded unit tests" ON)

if(ENABLE_UNIT_TESTS)
    message("Unit tests: ENABLED")
    add_compile_definitions(ENABLE_UNIT_TESTS=1)
else()
    message("Unit tests: DISABLED")
endif()

# ...

# Conditionally add test suite
if(ENABLE_UNIT_TESTS)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        Core/Src/test_suite.c
    )
endif()
```

### 工作原理

1. **CMake 选项定义**
   ```cmake
   option(ENABLE_UNIT_TESTS "Enable embedded unit tests" ON)
   ```
   - 定义一个可配置的布尔选项
   - 默认值为 `ON`

2. **添加预处理宏**
   ```cmake
   add_compile_definitions(ENABLE_UNIT_TESTS=1)
   ```
   - 当选项为 `ON` 时，向所有源文件添加 `-DENABLE_UNIT_TESTS=1`
   - C 代码中可用 `#ifdef ENABLE_UNIT_TESTS` 检测

3. **条件编译源文件**
   ```cmake
   if(ENABLE_UNIT_TESTS)
       target_sources(${CMAKE_PROJECT_NAME} PRIVATE
           Core/Src/test_suite.c
       )
   endif()
   ```
   - 仅当选项为 `ON` 时才将 `test_suite.c` 加入编译

---

## 💻 main.c 中的条件编译

### 包含头文件

```c
#ifdef ENABLE_UNIT_TESTS
#include "test_suite.h"
#endif
```

### 调用测试

```c
#ifdef ENABLE_UNIT_TESTS
  /* Run Unit Tests First */
  Run_All_Tests();
#endif
```

**好处：**
- 当 `ENABLE_UNIT_TESTS` 未定义时，编译器会自动跳过这些代码
- 不会产生任何代码或数据
- 符合 STM32CubeMX 的 `USER CODE` 区域规范

---

## 🚀 常见使用场景

### 场景 1：开发阶段（启用测试）

```bash
# 清理旧构建
rm -rf build/*

# 重新配置并编译（启用测试）
cd build
cmake .. -DENABLE_UNIT_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make -j4

# 烧录并观察测试结果
st-flash write V2.2_F407.bin 0x08000000
picocom -b 115200 /dev/ttyUSB0
```

**使用时机：**
- 开发新功能时
- 修改算法逻辑后
- 调试问题时
- 验证代码正确性

---

### 场景 2：发布阶段（禁用测试）

```bash
# 清理旧构建
rm -rf build/*

# 重新配置并编译（禁用测试，Release 模式）
cd build
cmake .. -DENABLE_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
make -j4

# 烧录发布固件
st-flash write V2.2_F407.bin 0x08000000
```

**使用时机：**
- 产品发布前
- 需要最小固件体积
- 追求最快启动速度
- 生产环境部署

---

### 场景 3：切换配置（推荐方法）

```bash
# 方法 A：删除 build 目录重新配置
rm -rf build
mkdir build
cd build
cmake .. -DENABLE_UNIT_TESTS=OFF
make -j4

# 方法 B：使用 ccmake 交互式配置（需安装）
cd build
ccmake ..
# 按 't' 进入高级模式
# 找到 ENABLE_UNIT_TESTS，按 Enter 切换 ON/OFF
# 按 'c' 配置，按 'g' 生成
make -j4
```

---

## ⚙️ 高级配置

### 与 CMAKE_BUILD_TYPE 配合使用

可以让测试自动跟随构建类型：

**修改 CMakeLists.txt（可选）：**

```cmake
# 自动根据构建类型决定是否启用测试
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(ENABLE_UNIT_TESTS "Enable embedded unit tests" ON)
else()
    option(ENABLE_UNIT_TESTS "Enable embedded unit tests" OFF)
endif()
```

**使用：**
```bash
# Debug 模式自动启用测试
cmake .. -DCMAKE_BUILD_TYPE=Debug
# ENABLE_UNIT_TESTS 自动为 ON

# Release 模式自动禁用测试
cmake .. -DCMAKE_BUILD_TYPE=Release
# ENABLE_UNIT_TESTS 自动为 OFF
```

---

## 🔍 验证配置

### 方法 1：查看 CMake 输出

编译时观察 CMake 消息：

```bash
cmake .. -DENABLE_UNIT_TESTS=ON
```

**输出示例：**
```
-- Build type: Debug
Unit tests: ENABLED      ← 确认测试已启用
-- Configuring done
-- Generating done
```

---

### 方法 2：检查编译文件列表

```bash
cd build
make VERBOSE=1 | grep test_suite.c
```

**如果启用测试，应看到：**
```
/usr/bin/arm-none-eabi-gcc ... Core/Src/test_suite.c ...
```

**如果禁用测试，不应看到任何输出。**

---

### 方法 3：检查固件大小

```bash
arm-none-eabi-size build/V2.2_F407.elf
```

**对比示例：**
```
# 启用测试
   text    data     bss     dec     hex filename
  52340    1024    8192   61556    f074 V2.2_F407.elf

# 禁用测试
   text    data     bss     dec     hex filename
  49120    1024    6144   56288    dbe0 V2.2_F407.elf
```

---

## 📝 最佳实践

### ✅ 推荐做法

1. **开发时启用测试**
   ```bash
   cmake .. -DENABLE_UNIT_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
   ```

2. **发布前禁用测试**
   ```bash
   cmake .. -DENABLE_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
   ```

3. **清理构建缓存**
   - 切换配置前删除 `build` 目录
   - 避免 CMake 缓存导致的配置混乱

4. **版本标识**
   - 在固件版本号中标识是否包含测试
   - 例如：`v1.0-debug`（含测试）vs `v1.0`（无测试）

---

### ⚠️ 注意事项

1. **不要手动定义宏**
   - 不要在代码中写 `#define ENABLE_UNIT_TESTS`
   - 应通过 CMake 选项控制

2. **清理旧构建**
   - 切换选项后务必清理 `build` 目录
   - 否则可能使用旧配置

3. **CubeMX 兼容**
   - 所有条件编译代码均位于 `USER CODE` 区域
   - 重新生成代码不会丢失

---

## 🐛 故障排除

### 问题 1：禁用测试后仍然运行测试

**可能原因：** CMake 缓存未更新

**解决方法：**
```bash
rm -rf build/*
cd build
cmake .. -DENABLE_UNIT_TESTS=OFF
make -j4
```

---

### 问题 2：编译错误 `test_suite.h: No such file`

**可能原因：** 启用了测试但 `test_suite.c` 未编译

**解决方法：**
1. 检查 `CMakeLists.txt` 中的条件编译配置
2. 重新运行 CMake

---

### 问题 3：固件大小未减小

**可能原因：** 使用了 Debug 构建或优化级别不够

**解决方法：**
```bash
cmake .. -DENABLE_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
make -j4
```

---

## 📊 总结

| 操作 | 命令 | 用途 |
|------|------|------|
| **启用测试** | `cmake .. -DENABLE_UNIT_TESTS=ON` | 开发、调试 |
| **禁用测试** | `cmake .. -DENABLE_UNIT_TESTS=OFF` | 发布、生产 |
| **查看配置** | `cmake .. -L` | 列出所有选项 |
| **清理构建** | `rm -rf build/*` | 重置配置 |

---

## 🔗 相关文档

- **单元测试指南**: `UNIT_TEST_GUIDE.md`
- **任务完成总结**: `TASK_COMPLETION_SUMMARY.md`
- **集成指导**: `INTEGRATION_GUIDE.md`
