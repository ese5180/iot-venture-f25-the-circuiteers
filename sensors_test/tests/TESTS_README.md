# Sensor Unit Tests - Test-Driven Development (TDD)

## 📋 概述

本项目包含使用 **Zephyr ztest 框架** 编写的单元测试，遵循 Test-Driven Development (TDD) 原则。

## 📁 测试结构

```
tests/
├── CMakeLists.txt                 # 测试 CMake 构建配置
├── prj.conf                       # 测试 Zephyr 内核配置
├── boards/
│   └── nucleo_wl55jc.overlay      # 测试设备树配置（同主应用）
└── unit/
    ├── test_suite.c               # 测试套件定义
    ├── test_bme280.c              # BME280 单元测试
    └── test_adxl345.c             # ADXL345 单元测试
```

## 🧪 测试套件

### BME280 单元测试 (test_bme280.c)

#### Test 1: Device Detection
**目标**: 验证 BME280 设备在设备树中正确检测
```c
test_bme280_device_detection()
```
- 检查设备指针不为 NULL
- 验证设备在 overlay 中正确定义

#### Test 2: Device Readiness
**目标**: 验证 BME280 设备已就绪
```c
test_bme280_device_ready()
```
- 检查 `device_is_ready()` 返回 true
- 确保驱动已初始化

#### Test 3: Sensor Data Reading
**目标**: 验证 BME280 可以读取传感器数据
```c
test_bme280_sensor_read()
```
- 使用 RTIO 异步 API 调用 `sensor_read()`
- 验证返回值为 0（成功）

#### Test 4: Data Decoder
**目标**: 验证 BME280 解码器 API 可用
```c
test_bme280_decoder()
```
- 调用 `sensor_get_decoder()` 获取解码器
- 验证解码器不为 NULL

#### Test 5: Device Name Verification
**目标**: 验证设备名称有效
```c
test_bme280_device_name()
```
- 检查设备名称不为空
- 验证设备名称包含 "bme280"

#### Test 6: Channel Validation
**目标**: 验证 BME280 传感器通道
```c
test_bme280_channels()
```
- 验证设备支持所需的传感器通道
- 检查温度、气压、湿度通道

### ADXL345 单元测试 (test_adxl345.c)

#### Test 1: Device Detection
**目标**: 验证 ADXL345 设备在设备树中正确检测
```c
test_adxl345_device_detection()
```
- 检查设备指针不为 NULL
- 验证设备在 overlay 中正确定义

#### Test 2: Device Readiness
**目标**: 验证 ADXL345 设备已就绪
```c
test_adxl345_device_ready()
```
- 检查 `device_is_ready()` 返回 true
- 确保驱动已初始化

#### Test 3: Sample Fetch
**目标**: 验证 ADXL345 可以获取样本
```c
test_adxl345_sample_fetch()
```
- 调用 `sensor_sample_fetch()`
- 验证返回值为 0（成功）

#### Test 4: X-Axis Acceleration Reading
**目标**: 验证 ADXL345 可以读取 X 轴加速度
```c
test_adxl345_accel_x()
```
- 获取 X 轴加速度值
- 验证值在合理范围内（-20 到 +20 m/s²）

#### Test 5: Y-Axis Acceleration Reading
**目标**: 验证 ADXL345 可以读取 Y 轴加速度
```c
test_adxl345_accel_y()
```
- 获取 Y 轴加速度值
- 验证值在合理范围内（-20 到 +20 m/s²）

#### Test 6: Z-Axis Acceleration Reading
**目标**: 验证 ADXL345 可以读取 Z 轴加速度（含重力）
```c
test_adxl345_accel_z()
```
- 获取 Z 轴加速度值
- 验证值显示重力加速度（8 到 11 m/s²）

#### Test 7: All Axes Sequential Reading
**目标**: 验证 ADXL345 可以连续读取所有三轴
```c
test_adxl345_all_axes()
```
- 依次读取 X、Y、Z 加速度
- 验证所有值都已填充
- 检查至少一个值非零

#### Test 8: Device Name Verification
**目标**: 验证设备名称有效
```c
test_adxl345_device_name()
```
- 检查设备名称不为空
- 验证设备名称包含 "adxl345"

#### Test 9: Multiple Sequential Readings
**目标**: 验证 ADXL345 多次连续读取的稳定性
```c
test_adxl345_multiple_reads()
```
- 执行 3 次连续的三轴读取
- 验证每次读取都成功
- 测试传感器读取的一致性

## 🏃 运行测试

### 编译测试

```bash
# 设置 SDK 路径
export ZEPHYR_SDK_INSTALL_DIR="/path/to/zephyr-sdk-0.17.4"

# 进入 Zephyr 工作空间
cd /Users/shannyao/zephyr-ws

# 编译单元测试
west build -p always -b nucleo_wl55jc \
  /Users/shannyao/Desktop/Master/ESE_5180/iot-venture-f25-the-circuiteers/sensors_test/tests \
  -- -DKCONFIG_WARN_TO_ERROR=OFF
```

### 烧录和运行测试

```bash
# 烧录到开发板
west flash

# 通过串口监视器观察测试输出（115200 bps）
```

## 📊 测试输出示例

```
Running ZTEST_UNIT_TEST: test_bme280_device_detection
PASS: test_bme280_device_detection

Running ZTEST_UNIT_TEST: test_bme280_device_ready
PASS: test_bme280_device_ready

Running ZTEST_UNIT_TEST: test_bme280_sensor_read
PASS: test_bme280_sensor_read

Running ZTEST_UNIT_TEST: test_adxl345_device_detection
PASS: test_adxl345_device_detection

Running ZTEST_UNIT_TEST: test_adxl345_device_ready
PASS: test_adxl345_device_ready

Running ZTEST_UNIT_TEST: test_adxl345_sample_fetch
PASS: test_adxl345_sample_fetch

Running ZTEST_UNIT_TEST: test_adxl345_accel_x
PASS: test_adxl345_accel_x

...

===== Test Results =====
Passed: 15
Failed: 0
```

## 🔍 测试覆盖范围

| 功能 | BME280 | ADXL345 |
|------|--------|---------|
| 设备检测 | ✅ | ✅ |
| 设备就绪 | ✅ | ✅ |
| 数据读取 | ✅ | ✅ |
| 多轴读取 | ✅ (温/压/湿) | ✅ (X/Y/Z) |
| 连续读取 | ✅ | ✅ |
| 数据验证 | ✅ | ✅ |
| 设备名称 | ✅ | ✅ |

## 🛠️ 测试框架：Ztest

### Ztest 特性
- 轻量级单元测试框架
- 集成到 Zephyr 内核
- 支持设备树集成
- 自动测试报告

### Ztest 宏

```c
/* 定义测试套件 */
ZTEST_SUITE(suite_name, setup, teardown, before, after);

/* 定义单元测试 */
ZTEST(suite_name, test_name) { ... }

/* 断言 */
zassert_true(condition, "message")
zassert_equal(actual, expected, "message")
zassert_not_null(ptr, "message")
```

## 📝 TDD 工作流

### 红-绿-重构循环

1. **红色**: 编写失败的测试
   - 定义预期行为
   - 测试初始状态下失败

2. **绿色**: 编写最少代码通过测试
   - 实现功能使测试通过
   - 关注功能正确性

3. **重构**: 优化代码质量
   - 改进代码结构
   - 保持测试通过

## 📚 相关资源

- [Zephyr ztest 文档](https://docs.zephyrproject.org/latest/develop/test/ztest.html)
- [BME280 数据手册](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280)
- [ADXL345 数据手册](https://www.analog.com/en/products/adxl345.html)

## ✅ 检查清单

- ✅ 两个传感器各有独立的测试文件
- ✅ 设备检测和初始化测试
- ✅ 数据读取功能测试
- ✅ 多轴/多参数读取测试
- ✅ 连续读取稳定性测试
- ✅ 使用 Zephyr ztest 框架
- ✅ 完整的设备树配置
- ✅ 详细的测试文档

## 🎯 下一步

1. **编译单元测试**: 运行 `west build` 在 tests 目录
2. **烧录到硬件**: 使用 `west flash` 烧录测试固件
3. **观察测试输出**: 通过 UART 监视器查看测试结果
4. **扩展测试**: 根据需要添加更多测试用例

---

**测试框架**: Zephyr ztest  
**测试总数**: 15+  
**覆盖传感器**: BME280, ADXL345  
**开发板**: STM32 Nucleo WL55JC1
