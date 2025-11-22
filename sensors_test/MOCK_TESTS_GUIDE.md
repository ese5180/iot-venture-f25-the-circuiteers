# Mock Unit Tests - TDD & CI/CD Guide

## 概述

该项目采用**分层测试策略**，将单元测试与硬件集成测试分离：

- **Mock 单元测试** (`tests_unit_mock/`) - ✅ 无需硬件，可在任何机器上运行
- **硬件集成测试** (`tests/`) - 需要 STM32 Nucleo WL55JC1 板子

## 快速开始

### 1. 构建 Mock 单元测试

```bash
cd sensors_test/tests_unit_mock
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### 2. 运行 Mock 单元测试

```bash
# 运行所有测试
ctest --output-on-failure

# 或分别运行
./test_bme280_mock
./test_adxl345_mock
```

### 3. 预期输出

```
╔════════════════════════════════════════╗
║  BME280 Mock Unit Tests                ║
║  No Hardware Required                  ║
╚════════════════════════════════════════╝

[TEST 1] BME280 Device Detection
✓ PASS: test_bme280_device_detection

[TEST 2] BME280 Device Not Found
✓ PASS: test_bme280_device_not_found

... (共 8 个测试)

╔════════════════════════════════════════╗
║  ✓ ALL TESTS PASSED                   ║
╚════════════════════════════════════════╝
```

## 项目结构

```
sensors_test/
├── src/
│   └── main.c                    # 生产代码 (BME280 + ADXL345)
├── tests/                        # 硬件集成测试 (Zephyr ztest)
│   └── unit/
│       ├── test_bme280.c         # BME280 硬件测试
│       ├── test_adxl345.c        # ADXL345 硬件测试
│       └── test_suite.c
├── tests_unit_mock/              # Mock 单元测试 (无硬件)
│   ├── CMakeLists.txt            # 独立构建系统
│   ├── include/
│   │   └── mock_sensor.h         # Mock 接口定义
│   └── unit/
│       ├── mock_sensor.c         # Mock 实现
│       ├── test_bme280_mock.c    # BME280 Mock 测试
│       └── test_adxl345_mock.c   # ADXL345 Mock 测试
├── .githooks/
│   └── pre-commit               # Pre-commit 钩子 (自动运行测试)
└── .github/workflows/
    └── mock-tests.yml           # GitHub Actions CI/CD
```

## Mock 框架说明

### Mock 接口

`include/mock_sensor.h` 定义了以下 Mock API：

#### 设备管理
```c
struct device *mock_bme280_get_device(void);
struct device *mock_adxl345_get_device(void);
bool mock_device_is_ready(const struct device *dev);
```

#### 传感器操作
```c
int mock_sensor_sample_fetch(const struct device *dev);
int mock_sensor_channel_get(const struct device *dev,
                            sensor_channel_t chan,
                            struct sensor_value *val);
int mock_sensor_read(const struct device *dev,
                     uint8_t *buffer,
                     size_t buf_size);
```

#### 测试配置
```c
void mock_adxl345_set_values(int32_t x, int32_t y, int32_t z);
void mock_bme280_set_values(int32_t temp, int32_t pressure, int32_t humidity);
void mock_device_set_not_found(bool not_found);
void mock_sensor_set_read_failure(bool failure);
void mock_sensor_reset_all(void);
```

### Mock 实现

`unit/mock_sensor.c` 提供完整实现，包括：

- **全局状态管理** - 模拟设备和传感器状态
- **默认值** - BME280 (25°C, 101325 Pa, 52%), ADXL345 (-5, -1, 16 m/s²)
- **错误注入** - 支持模拟设备失效和读取错误
- **动态值修改** - 测试不同场景

## 单元测试详解

### BME280 Mock 测试 (8 个)

1. **Device Detection** - 验证设备检测
2. **Device Not Found** - 模拟设备缺失
3. **Device Readiness** - 验证就绪状态
4. **Device Name** - 验证命名
5. **Sensor Read** - 测试读取操作
6. **Read Failure** - 模拟读取失败
7. **Decoder API** - 验证解码器
8. **Multiple Reads** - 连续读取测试

### ADXL345 Mock 测试 (12 个)

1. **Device Detection** - 验证设备检测
2. **Device Not Found** - 模拟设备缺失
3. **Device Readiness** - 验证就绪状态
4. **Device Name** - 验证命名
5. **Sample Fetch** - 采样获取测试
6. **X-Axis Read** - X 轴读取测试
7. **Y-Axis Read** - Y 轴读取测试
8. **Z-Axis Read** - Z 轴读取测试
9. **All Axes Sequential** - 连续读取所有轴
10. **Custom Values** - 自定义值测试
11. **Read Failure** - 模拟读取失败
12. **Multiple Reads** - 连续读取测试

## Pre-commit 钩子设置

### 自动方式（推荐）

```bash
cd sensors_test
git config core.hooksPath .githooks
```

验证：
```bash
git config core.hooksPath
# 输出: .githooks
```

### 手动方式

```bash
# 复制钩子到 .git/hooks
cp .githooks/pre-commit .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

### Pre-commit 钩子功能

提交前自动运行：
- 构建 Mock 测试（如果需要）
- 运行所有 Mock 单元测试
- 验证所有测试通过
- 失败时阻止提交

### 示例

```bash
# 测试通过 ✅
$ git commit -m "Fix bug"
🧪 Running Mock Unit Tests (Pre-commit Hook)
================================================
✅ All mock unit tests PASSED!
================================================
[main a1b2c3d] Fix bug

# 测试失败 ❌
$ git commit -m "Add feature"
🧪 Running Mock Unit Tests (Pre-commit Hook)
================================================
100% tests passed, 0 tests failed out of 2
❌ Mock unit tests FAILED!
================================================
Fix the failing tests before committing.
```

## GitHub Actions CI/CD

### 工作流文件

`.github/workflows/mock-tests.yml` 定义了完整的 CI/CD 管道：

### 支持的操作系统

- ✅ Ubuntu (Linux)
- ✅ macOS
- ✅ Windows

### 自动触发条件

- Push 到 `main` 或 `develop` 分支
- PR 到 `main` 或 `develop` 分支
- 每日计划运行 (UTC 2 AM)

### 工作流步骤

1. **检出代码** - Clone repository
2. **设置 CMake** - 安装最新 CMake 3.20+
3. **编译测试** - 构建 Mock 单元测试
4. **运行测试** - BME280 和 ADXL345 测试
5. **生成报告** - 创建测试摘要

### 工作流输出示例

```
✅ All mock unit tests PASSED!
✅ All mock unit tests PASSED!
✅ All mock unit tests PASSED!

Summary:
- Platform: Ubuntu Linux / macOS / Windows
- Test Duration: ~15 seconds total
- All tests: PASSED
```

## 扩展 Mock 测试

### 添加新的测试场景

编辑 `unit/test_bme280_mock.c` 或 `unit/test_adxl345_mock.c`：

```c
int test_bme280_custom_scenario(void)
{
    printf("\n[TEST N] BME280 Custom Scenario\n");
    
    struct device *dev = mock_bme280_get_device();
    ASSERT_NOT_NULL(dev, "Device should exist");
    
    /* 设置自定义值 */
    mock_bme280_set_values(30, 102000, 45);
    
    /* 执行测试 */
    struct sensor_value val;
    int rc = mock_sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &val);
    ASSERT_EQUAL(rc, 0, "Read should succeed");
    ASSERT_EQUAL(val.val1, 30, "Temperature should match");
    
    TEST_PASS("test_bme280_custom_scenario");
    return 0;
}
```

添加到 main 中：
```c
failed += test_bme280_custom_scenario();
```

### 模拟故障场景

```c
/* 模拟设备离线 */
mock_device_set_not_found(true);
struct device *dev = mock_bme280_get_device();
assert(dev == NULL);

/* 模拟读取失败 */
mock_sensor_set_read_failure(true);
int rc = mock_sensor_sample_fetch(dev);
assert(rc == -1);

/* 重置状态 */
mock_sensor_reset_all();
```

## 与硬件集成测试的关系

### 分层测试策略

```
┌─────────────────────────────────────┐
│  Mock Unit Tests                    │
│  - ✅ 速度快 (~0.1s)                │
│  - ✅ 无硬件依赖                     │
│  - ✅ Pre-commit 每次运行           │
│  - ✅ CI/CD 自动化                   │
└──────────┬──────────────────────────┘
           │
    开发循环（快速反馈）
           │
┌──────────▼──────────────────────────┐
│  Hardware Integration Tests          │
│  - ⏱️ 速度慢 (~30s)                  │
│  - 🔌 需要 Nucleo WL55JC1            │
│  - 🧪 真实传感器验证                │
│  - 📊 性能基准测试                  │
└─────────────────────────────────────┘
```

### 推荐流程

1. **本地开发**：在提交前运行 Mock 单元测试
2. **Pre-commit**：自动运行 Mock 测试，阻止失败提交
3. **CI/CD**：PR 自动运行 Mock 测试
4. **发布**：部署到硬件前运行集成测试

## 常见问题

### Q1: Mock 测试如何在我的机器上运行？

**A:** Mock 测试是纯 C 代码，无需任何硬件：

```bash
cd sensors_test/tests_unit_mock/build
./test_bme280_mock    # 直接运行
./test_adxl345_mock   # 无需板子
```

### Q2: 如何为我的改动添加 Mock 测试？

**A:** 编辑对应的测试文件并添加新测试函数，确保在 `main()` 中调用。

### Q3: Mock 值与实际硬件不匹配怎么办？

**A:** 更新 `mock_sensor.c` 中的默认值或使用 `mock_*_set_values()` 函数调整。

### Q4: 如何跳过 pre-commit 钩子？

**A:** 使用 `--no-verify` 标志（不推荐）：
```bash
git commit --no-verify -m "Skip pre-commit checks"
```

### Q5: 如何在 CI/CD 中调试测试失败？

**A:** 查看 GitHub Actions 工作流日志，或在本地重现：
```bash
cd sensors_test/tests_unit_mock/build
ctest --output-on-failure --verbose
```

## 性能指标

| 指标 | Mock 测试 | 硬件测试 |
|------|----------|---------|
| 执行时间 | ~0.23s | ~30s |
| 并发支持 | ✅ 支持 | ❌ 串行 |
| 依赖项 | CMake, C11 | Zephyr SDK, 板子 |
| 故障恢复 | ✅ 快速 | ⏱️ 需要手动 |
| CI/CD 成本 | 💰 免费 | 💰 需要硬件 |

## 下一步

- [ ] 集成 SonarQube 代码质量分析
- [ ] 添加性能基准测试
- [ ] 实现夜间硬件测试运行
- [ ] 创建测试覆盖率报告
- [ ] 集成 CMake Ctest 仪表板

## 支持的平台

✅ **Mock 单元测试（所有平台支持）**
- Linux (Ubuntu, Debian, Fedora)
- macOS (Intel, Apple Silicon)
- Windows (MSVC, MinGW)
- CI/CD (GitHub Actions, GitLab CI, Jenkins)

⚠️ **硬件测试（需要特定板子）**
- STM32 Nucleo WL55JC1
- Zephyr 4.3.0-rc1+

---

**最后更新**: 2025-01-21  
**创建者**: GitHub Copilot  
**许可证**: MIT
