# ROTS Sender - ESP32发送端

基于ESP32的远程气味识别发送端，使用边缘AI进行实时气味检测。

## 硬件要求

- ESP32开发板 (ESP32-WROOM-32E推荐)
- 8x MQ系列气体传感器 (MQ-2到MQ-9)
- DHT22温湿度传感器
- BMP280气压传感器
- ADS1115 ADC转换器
- 电源管理模块

## 项目结构

```
sender/
├── src/                    # 源代码
│   ├── main.cpp           # 主程序
│   ├── rots_sender.h      # 主头文件
│   ├── rots_sensor_manager.cpp/h    # 传感器管理
│   ├── rots_ai_engine.cpp/h         # AI推理引擎
│   ├── rots_communication.cpp/h     # 通信模块
│   ├── rots_debug.cpp/h             # 调试模块
│   └── rots_system_monitor.cpp/h    # 系统监控
├── lib/                   # 库文件
├── models/                # AI模型文件
├── platformio.ini         # PlatformIO配置
└── README.md              # 说明文档
```

## 功能特性

- **多传感器融合**: 8路MQ传感器 + 环境传感器
- **边缘AI推理**: 实时气味识别和分类
- **WiFi通信**: 连接云服务器
- **MQTT协议**: 实时数据传输
- **调试支持**: 串口调试和LED指示
- **系统监控**: 内存、网络、错误监控

## 快速开始

### 1. 安装PlatformIO

```bash
# 安装PlatformIO Core
pip install platformio

# 或者使用VS Code扩展
# 搜索并安装PlatformIO IDE
```

### 2. 克隆项目

```bash
git clone https://github.com/yourusername/ROTS.git
cd ROTS/sender
```

### 3. 配置网络

编辑 `src/rots_sender.h` 中的网络配置：

```cpp
#define ROTS_WIFI_SSID            "your_wifi_ssid"
#define ROTS_WIFI_PASSWORD        "your_wifi_password"
#define ROTS_MQTT_BROKER_HOST     "your_mqtt_broker"
```

### 4. 编译和上传

```bash
# 编译项目
pio run

# 上传到ESP32
pio run --target upload

# 监控串口输出
pio device monitor
```

## 硬件连接

### ESP32引脚分配

#### 模拟输入 (MQ传感器)
- MQ2: GPIO36 (ADC1_CH0)
- MQ3: GPIO39 (ADC1_CH3)
- MQ4: GPIO34 (ADC1_CH6)
- MQ5: GPIO35 (ADC1_CH7)
- MQ6: GPIO32 (ADC1_CH4)
- MQ7: GPIO33 (ADC1_CH5)
- MQ8: GPIO25 (ADC1_CH8)
- MQ9: GPIO26 (ADC1_CH9)

#### I2C接口 (环境传感器)
- SDA: GPIO21
- SCL: GPIO22

#### 状态指示
- Error LED: GPIO2
- Status LED: GPIO4
- Sensor Power: GPIO5

### 传感器连接

#### MQ传感器
```
VCC → 5V
GND → GND
A0 → ESP32 GPIO36-39, 32-35, 25-26
```

#### DHT22
```
VCC → 3.3V
GND → GND
DATA → GPIO21 (SDA)
```

#### BMP280
```
VCC → 3.3V
GND → GND
SDA → GPIO21
SCL → GPIO22
```

## 配置说明

### 1. 传感器校准

系统启动时会自动进行传感器校准：

```cpp
// 在清洁空气中校准
ROTS_SensorManager_CalibrateSensors();
```

### 2. AI模型配置

```cpp
// 设置置信度阈值
#define ROTS_AI_CONFIDENCE_THRESHOLD  0.7f

// 设置推理间隔
#define ROTS_AI_INFERENCE_INTERVAL    500    // ms
```

### 3. 通信配置

```cpp
// WiFi配置
#define ROTS_WIFI_SSID            "ROTS_Network"
#define ROTS_WIFI_PASSWORD        "rots_password_2024"

// MQTT配置
#define ROTS_MQTT_BROKER_HOST     "mqtt.rots-system.com"
#define ROTS_MQTT_BROKER_PORT     1883
```

## 调试指南

### 1. 串口调试

- 波特率: 115200
- 数据位: 8, 停止位: 1, 校验: 无

### 2. 调试级别

```cpp
ROTS_DEBUG_ERROR   = 0  // 错误信息
ROTS_DEBUG_WARNING = 1  // 警告信息
ROTS_DEBUG_INFO    = 2  // 一般信息
ROTS_DEBUG_DEBUG   = 3  // 调试信息
```

### 3. 调试命令

```cpp
// 设置调试级别
ROTS_Debug_SetLevel(ROTS_DEBUG_INFO);

// 打印调试信息
DEBUG_INFO("System started\r\n");
DEBUG_ERROR("Error occurred: %d\r\n", error_code);
```

### 4. LED指示

- **Error LED**: 错误指示，闪烁表示错误
- **Status LED**: 状态指示，常亮表示WiFi连接

## 性能优化

### 1. 内存优化

```cpp
// 检查内存使用
DEBUG_INFO("Free Heap: %lu bytes\r\n", ESP.getFreeHeap());
DEBUG_INFO("Free PSRAM: %lu bytes\r\n", ESP.getFreePsram());
```

### 2. 功耗优化

```cpp
// 设置CPU频率
setCpuFrequencyMhz(80);  // 降低功耗

// 启用深度睡眠
esp_deep_sleep_start();
```

### 3. 网络优化

```cpp
// 设置WiFi功率
WiFi.setTxPower(WIFI_POWER_11dBm);

// 设置MQTT保持连接
mqtt_client.setKeepAlive(60);
```

## 故障排除

### 1. 编译错误

- 检查PlatformIO配置
- 检查库依赖
- 检查Arduino框架版本

### 2. 上传失败

- 检查USB连接
- 检查端口设置
- 检查ESP32进入下载模式

### 3. 运行错误

- 检查串口输出
- 检查硬件连接
- 检查网络配置

### 4. 传感器问题

- 检查电源供应
- 检查模拟输入
- 检查校准参数

## API参考

### 传感器管理

```cpp
// 读取传感器数据
ROTS_StatusTypeDef ROTS_SensorManager_ReadSensors(ROTS_SensorData_t* data);

// 获取传感器状态
ROTS_StatusTypeDef ROTS_SensorManager_GetStatus(ROTS_SensorStatus_t* status);
```

### AI引擎

```cpp
// 处理气味检测
ROTS_StatusTypeDef ROTS_AIEngine_ProcessOdor(ROTS_OdorResult_t* result);

// 获取AI状态
ROTS_StatusTypeDef ROTS_AIEngine_GetStatus(ROTS_AIStatus_t* status);
```

### 通信模块

```cpp
// 发送检测结果
ROTS_StatusTypeDef ROTS_Communication_SendOdorDetection(const ROTS_OdorResult_t* result);

// 发送状态信息
ROTS_StatusTypeDef ROTS_Communication_SendStatus(const ROTS_SenderStatus_t* status);
```

## 许可证

本项目采用MIT许可证 - 详见主项目README。
