# ROTS 接收端调试指南

## 调试工具

### 1. 串口调试
- **调试UART**: USART1 (PA9/PA10)
- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: 无

### 2. 调试级别
```c
ROTS_DEBUG_ERROR   = 0  // 错误信息
ROTS_DEBUG_WARNING = 1  // 警告信息
ROTS_DEBUG_INFO    = 2  // 一般信息
ROTS_DEBUG_DEBUG   = 3  // 调试信息
```

### 3. 调试命令
```c
// 设置调试级别
ROTS_Debug_SetLevel(ROTS_DEBUG_INFO);

// 打印调试信息
DEBUG_INFO("System started\r\n");
DEBUG_ERROR("Error occurred: %d\r\n", error_code);

// 打印十六进制数据
uint8_t data[] = {0x01, 0x02, 0x03};
ROTS_Debug_PrintHex(ROTS_DEBUG_DEBUG, "Data", data, 3);
```

## 硬件连接

### 1. STM32F407VET6 引脚分配

#### 调试UART (USART1)
- TX: PA9
- RX: PA10

#### ESP8266通信 (USART2)
- TX: PA2
- RX: PA3
- Reset: PA4

#### OLED显示 (I2C1)
- SDA: PB7
- SCL: PB6

#### 泵控制 (PWM)
- Pump1: PA8 (TIM2_CH1)
- Pump2: PA9 (TIM2_CH2)
- Pump3: PA10 (TIM2_CH3)
- Pump4: PB0 (TIM2_CH4)
- Pump5: PB1 (TIM3_CH4)

#### 阀控制 (GPIO)
- Valve1: PC0
- Valve2: PC1
- Valve3: PC2
- Valve4: PC3
- Valve5: PC4

#### 风扇控制 (GPIO)
- Fan1: PC5
- Fan2: PC6

#### 状态LED
- Error LED: PA5
- Status LED: PA6

### 2. 电源连接
- VCC: 3.3V
- GND: 地
- 泵电源: 12V (通过继电器控制)

## 调试步骤

### 1. 基础硬件测试
```c
// 在main.c中添加
ROTS_Hardware_SelfTest();
```

### 2. 串口调试测试
1. 连接USB转TTL模块到PA9/PA10
2. 打开串口调试工具 (115200波特率)
3. 观察启动信息

### 3. WiFi连接测试
```c
// 检查WiFi连接状态
ROTS_Debug_PrintWiFiStatus();
```

### 4. MQTT连接测试
```c
// 检查MQTT连接状态
ROTS_Debug_PrintMQTTStatus();
```

### 5. 执行器测试
```c
// 测试泵控制
ROTS_Hardware_SetPumpSpeed(0, 50); // 50%速度

// 测试阀控制
ROTS_Hardware_SetValveState(0, ROTS_ACTUATOR_ON);

// 测试风扇控制
ROTS_Hardware_SetFanSpeed(0, 80); // 80%速度
```

## 常见问题

### 1. 编译错误
- 检查HAL库是否正确包含
- 检查头文件路径设置
- 检查宏定义是否正确

### 2. 串口无输出
- 检查UART配置
- 检查引脚连接
- 检查波特率设置

### 3. WiFi连接失败
- 检查ESP8266模块
- 检查AT命令响应
- 检查网络配置

### 4. MQTT连接失败
- 检查MQTT服务器地址
- 检查网络连接
- 检查MQTT协议实现

### 5. 执行器不工作
- 检查GPIO配置
- 检查PWM配置
- 检查电源连接

## 调试技巧

### 1. 使用LED指示
```c
// 错误指示
HAL_GPIO_TogglePin(ERROR_LED_PORT, ERROR_LED_PIN);

// 状态指示
HAL_GPIO_TogglePin(STATUS_LED_PORT, STATUS_LED_PIN);
```

### 2. 使用断点调试
- 在STM32CubeIDE中设置断点
- 使用ST-Link调试器
- 观察变量值

### 3. 使用逻辑分析仪
- 观察PWM信号
- 检查时序
- 验证通信协议

### 4. 使用示波器
- 检查电源质量
- 观察信号完整性
- 测量时序参数

## 性能监控

### 1. 内存使用
```c
ROTS_Debug_PrintMemoryUsage();
```

### 2. 系统状态
```c
ROTS_Debug_PrintSystemStatus();
```

### 3. 通信状态
```c
ROTS_Debug_PrintWiFiStatus();
ROTS_Debug_PrintMQTTStatus();
```

## 测试用例

### 1. 基础功能测试
```c
void test_basic_functions(void)
{
    // 测试GPIO
    HAL_GPIO_TogglePin(STATUS_LED_PORT, STATUS_LED_PIN);
    
    // 测试PWM
    ROTS_Hardware_SetPumpSpeed(0, 50);
    
    // 测试UART
    DEBUG_INFO("Basic test passed\r\n");
}
```

### 2. 通信测试
```c
void test_communication(void)
{
    // 测试WiFi连接
    if (ROTS_Communication_ConnectWiFi() == ROTS_OK) {
        DEBUG_INFO("WiFi connected\r\n");
    }
    
    // 测试MQTT连接
    if (ROTS_Communication_ConnectMQTT() == ROTS_OK) {
        DEBUG_INFO("MQTT connected\r\n");
    }
}
```

### 3. 执行器测试
```c
void test_actuators(void)
{
    // 测试所有泵
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        ROTS_Hardware_SetPumpSpeed(i, 30);
        HAL_Delay(1000);
        ROTS_Hardware_SetPumpSpeed(i, 0);
    }
    
    // 测试所有阀
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        ROTS_Hardware_SetValveState(i, ROTS_ACTUATOR_ON);
        HAL_Delay(500);
        ROTS_Hardware_SetValveState(i, ROTS_ACTUATOR_OFF);
    }
}
```

## 部署检查清单

- [ ] 硬件连接正确
- [ ] 电源供应稳定
- [ ] 串口调试正常
- [ ] WiFi连接成功
- [ ] MQTT连接成功
- [ ] 执行器响应正常
- [ ] 显示功能正常
- [ ] 错误处理正常
- [ ] 内存使用合理
- [ ] 性能满足要求
