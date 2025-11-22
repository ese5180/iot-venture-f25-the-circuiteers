# fuota_with_sensor 程序修改说明

## 修改概述
已成功将 `fuota_with_sensor` 程序升级，集成了 BME280 和 ADXL345 两个传感器的功能，使其能够定期读取传感器数据并通过 LoRa 发送打包后的数据。

## 修改的文件

### 1. `src/main.c` - 核心主程序文件
主要增加了以下内容：

#### 新增头文件
```c
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>
```

#### 新增传感器配置
- BME280 通过 RTIO API 异步读取温度、湿度、气压
- ADXL345 通过标准传感器 API 读取 X、Y、Z 三轴加速度

#### 新增函数

**1. 初始化函数**
- `init_bme280()` - 初始化 BME280 传感器
- `init_adxl345()` - 初始化 ADXL345 传感器

**2. 数据读取函数**
- `read_bme280_data()` - 读取温度、湿度、气压（返回 Q31 格式数据）
- `read_adxl345_data()` - 读取三轴加速度数据

**3. 数据打包函数**
- `pack_sensor_payload()` - 将传感器数据转换为二进制负载并打包成 12 字节

#### 数据打包格式（12 字节）

| 字节 | 数据 | 范围/说明 |
|------|------|---------|
| 0-1  | 温度 × 100 (int16_t) | -327.68 ~ +327.67 °C |
| 2-3  | 湿度 × 100 (uint16_t) | 0 ~ 655.35 % |
| 4-5  | 气压 × 10 (uint16_t) | 例如 1013.2 hPa → 10132 |
| 6-7  | X 加速度 × 1000 (int16_t) | m/s² |
| 8-9  | Y 加速度 × 1000 (int16_t) | m/s² |
| 10-11 | Z 加速度 × 1000 (int16_t) | m/s² |

#### main 函数的改动
- 在 FUOTA 设置之前添加传感器初始化
- 在原有的 20 秒上行循环中添加传感器数据读取和打包
- 当传感器正常工作时，使用打包后的传感器数据替代原始的 "updated data" 字符串发送
- 如果传感器初始化失败，程序仍然会发送原始的 "updated data" 字符串（降级处理）
- 保持了原有的 LoRa 发送方式（FPort=2，UNCONFIRMED 模式）

### 2. `prj.conf` - 项目配置文件
增加了传感器相关的配置选项：
```conf
# Sensor Configuration
CONFIG_GPIO=y
CONFIG_I2C=y
CONFIG_SENSOR=y
CONFIG_SENSOR_ASYNC_API=y
CONFIG_BME280=y
CONFIG_ADXL345=y
```

### 3. `boards/nucleo_wl55jc.overlay` - 板级设备树配置
新创建该文件，配置了两个传感器的 I2C 接口：
- BME280 连接到 I2C2 总线（地址 0x77）
- ADXL345 连接到 I2C3 总线（地址 0x53），并配置了相应的 GPIO 引脚（PB13/PB14）

## 功能说明

### 工作流程
1. 程序启动时初始化两个传感器
2. FUOTA 框架正常加载和初始化
3. 每隔 20 秒（DELAY = K_SECONDS(20)）：
   - 读取 BME280 温度、湿度、气压数据
   - 读取 ADXL345 X、Y、Z 加速度数据
   - 将数据转换为浮点数，然后转换为整数固定点数
   - 打包成 12 字节的二进制负载
   - 通过 LoRa 发送（FPort=2）

### 错误处理
- 如果传感器初始化失败，程序会记录错误日志但继续运行，发送原始的 "updated data" 字符串
- 如果在运行时读取传感器数据失败，会记录具体的传感器错误代码

### 向后兼容性
- 完全保留了原有的 FUOTA 功能
- 保持了原有的 LoRa 发送方式和周期（20 秒）
- 如果传感器无法使用，程序仍然可以正常运行并发送原始数据

## 与 sensors_test 的区别

`fuota_with_sensor` 相比 `sensors_test` 程序的主要区别：
- 集成了 LoRaWAN 和 FUOTA 框架
- 定时发送打包后的传感器数据而不是打印输出
- 提供了降级处理机制（传感器失败时仍可发送原始数据）
- 将 6 个传感器数据值合并打包为单一的 12 字节负载，便于通过 LoRa 发送

## 编译和构建

确保在构建时包含了传感器相关的驱动程序：
```bash
west build -b nucleo_wl55jc
```

如有需要，可以使用菜单配置工具进行更详细的配置：
```bash
west build -t menuconfig
```

## 测试建议

1. 使用 LoRaWAN 网络服务器或模拟器验证接收到的打包数据
2. 监控日志输出，查看传感器初始化和数据读取的状态
3. 使用示波器或逻辑分析仪验证 I2C 通信是否正常

