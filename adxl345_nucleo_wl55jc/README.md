# ADXL345 on Nucleo WL55JC1 (Zephyr)

这是一个基于 Zephyr 的示例应用，展示如何在 STM32 Nucleo WL55JC1 开发板上通过 I2C3 读取 ADXL345 三轴加速度传感器并将数据通过串口打印。

## 目录结构

- `CMakeLists.txt` - Zephyr app 列表
- `prj.conf` - Kconfig 配置，启用 I2C / sensor / ADXL345 等
- `boards/nucleo_wl55jc.overlay` - 设备树覆盖，将 ADXL345 绑定到 I2C3 总线
- `src/main.c` - 主程序：读取并打印加速度数据

## 硬件接线

本应用使用 **I2C3** 接口，与 BME280 的 I2C2 接口完全独立，可以同时在一块板上工作。

### ADXL345 I2C 接线（Nucleo WL55JC1）

根据 Nucleo WL55JC1 数据表，I2C3 使用以下引脚：

| ADXL345 引脚 | Nucleo WL55JC 引脚 | CN8 连接器位置 | STM32 引脚 | 说明 |
|---|---|---|---|---|
| VCC | 3.3V | - | - | 电源（不要用 5V） |
| GND | GND | - | - | 地 |
| SDA | A4 | Pin 5 | PB14 | I2C3_SDA |
| SCL | A5 | Pin 6 | PB13 | I2C3_SCL |
| ALT_ADDR | GND | - | - | 地址选择（GND=0x53，VCC=0x1D） |
| INT1/INT2 | 可选 | - | - | 中断引脚（暂不使用） |

### 接线建议

1. 确保 ADXL345 的电源为 **3.3V**（不要接 5V，会损坏芯片）
2. 所有 GND 引脚都要接板子的 GND
3. I2C 总线的 SDA/SCL 接到 CN8 连接器的 Pin 5/6（对应 PB14/PB13）
4. ALT_ADDR 接 GND，地址为 0x53；若要地址 0x1D，接 VCC
5. 一般来说，I2C 总线需要 4.7kΩ 的上拉电阻连接到 VCC（通常在 breakout 板上已有）

## 构建与烧录

### 1. 构建

在项目根目录或 zephyr-ws 中运行：

```bash
west build -b nucleo_wl55jc /path/to/adxl345_nucleo_wl55jc
```

或者如果你有清空旧编译缓存的需要：

```bash
west build -p always -b nucleo_wl55jc /path/to/adxl345_nucleo_wl55jc
```

### 2. 烧录

```bash
west flash -d build
```

### 3. 查看串口输出

使用串口终端工具连接板子（波特率通常为 115200，取决于板配置）：

```bash
# 使用 minicom/picocom/screen 等
minicom -D /dev/ttyUSB0 -b 115200
# 或
picocom /dev/ttyUSB0 -b 115200
```

预期输出示例（每秒更新一次）：

```
[00:00:01.234,000] <inf> adxl345_nucleo: Found ADXL345 device 'ADXL345', reading acceleration data...
[00:00:02.235,000] <inf> adxl345_nucleo: Accel X: 0.245632 m/s^2, Y: -0.098765 m/s^2, Z: 9.810234 m/s^2
[00:00:03.235,000] <inf> adxl345_nucleo: Accel X: 0.251234 m/s^2, Y: -0.087654 m/s^2, Z: 9.812345 m/s^2
...
```

## 已知假设与调整建议

### ADXL345 驱动配置

- 本示例使用 Zephyr 内置的 ADXL345 I2C 驱动（`CONFIG_ADXL345=y`）
- 使用的是标准的 Zephyr sensor API（`sensor_sample_fetch` 和 `sensor_channel_get`）
- 数据单位：m/s²（米每平方秒）
- I2C 地址：0x53（当 ALT_ADDR=GND）或 0x1D（当 ALT_ADDR=VCC），默认配置为 0x53

### I2C 总线配置

- I2C3 在 Nucleo WL55JC 上需要通过 overlay 启用
- 如需修改 I2C 地址或频率，编辑 `boards/nucleo_wl55jc.overlay`
- 若要启用 I2C3 的引脚控制（pinctrl），可能需要在 overlay 中添加 pinctrl 配置（见下面的可选改进）

### 与 BME280 共存

本应用使用 **I2C3**，而 BME280 应用使用 **I2C2**，两者互不干扰。若要在同一块板上同时运行两个传感器，需要：

1. 一个整合的应用，同时初始化 I2C2 (BME280) 和 I2C3 (ADXL345)
2. 或者分别编译两个应用，轮流烧录测试

### 可能的问题与解决

| 问题 | 原因 | 解决方案 |
|---|---|---|
| "no ADXL345 device found" | overlay 中的设备节点未被识别或驱动未启用 | 检查 `prj.conf` 中 `CONFIG_ADXL345=y`，确保设备树编译无误 |
| "Device not ready" | ADXL345 硬件未正确连接或通信故障 | 检查硬件接线，特别是 VCC/GND/SDA/SCL 引脚 |
| I2C 通信错误 | 接线错误或 I2C3 未启用 | 检查 I2C3 的 pinctrl 配置，确保 SDA/SCL 正确连接 |
| 数据异常（全 0 或固定值） | ADXL345 未正确初始化或寄存器读取失败 | 检查 I2C 地址是否正确（0x53 或 0x1D），检查总线上拉电阻 |

## 下一步（可选改进）

- 添加中断处理（INT1 或 INT2 引脚），实现基于阈值的事件检测
- 支持多种工作模式（低功耗、高分辨率等），通过 sensor API 的 attribute 控制
- 增加数据缓冲和时间戳记录
- 与 BME280 集成到一个应用中，同时读取两个传感器数据并输出到 SD 卡或网络

## 参考资源

- [ADXL345 数据手册](https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf)
- [Zephyr Sensor API 文档](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html)
- [Nucleo WL55JC1 原理图与开发资料](https://www.st.com/en/evaluation-tools/nucleo-wl55jc.html)
