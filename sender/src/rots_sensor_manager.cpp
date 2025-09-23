// ROTS Sensor Manager - 传感器管理模块
#include "rots_sender.h"
#include "rots_sensor_manager.h"
#include "rots_debug.h"

// 私有变量
static ROTS_SensorData_t current_sensor_data;
static ROTS_SensorData_t sensor_history[10];
static uint8_t history_index = 0;
static bool sensor_initialized = false;

// 传感器校准参数
static float sensor_calibration[ROTS_MAX_SENSORS] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

// 温度补偿参数
static float temperature_compensation = 0.02f; // 每度温度补偿系数

// 私有函数声明
static float ROTS_SensorManager_ReadMQSensor(uint8_t pin, uint8_t sensor_id);
static void ROTS_SensorManager_ApplyCalibration(ROTS_SensorData_t* data);
static void ROTS_SensorManager_ApplyTemperatureCompensation(ROTS_SensorData_t* data);
static void ROTS_SensorManager_UpdateHistory(const ROTS_SensorData_t* data);

// 初始化传感器管理器
ROTS_StatusTypeDef ROTS_SensorManager_Init(void) {
    // 配置引脚
    pinMode(ROTS_SENSOR_POWER_PIN, OUTPUT);
    digitalWrite(ROTS_SENSOR_POWER_PIN, HIGH);
    
    // 初始化I2C
    Wire.begin(ROTS_SDA_PIN, ROTS_SCL_PIN);
    
    // 等待传感器预热
    DEBUG_INFO("Warming up sensors...\r\n");
    delay(3000);
    
    // 初始化传感器数据
    memset(&current_sensor_data, 0, sizeof(ROTS_SensorData_t));
    memset(sensor_history, 0, sizeof(sensor_history));
    
    // 执行传感器校准
    ROTS_StatusTypeDef status = ROTS_SensorManager_CalibrateSensors();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Sensor calibration failed\r\n");
        return status;
    }
    
    sensor_initialized = true;
    DEBUG_INFO("Sensor manager initialized\r\n");
    return ROTS_OK;
}

// 读取所有传感器数据
ROTS_StatusTypeDef ROTS_SensorManager_ReadSensors(ROTS_SensorData_t* data) {
    if (!sensor_initialized || !data) {
        return ROTS_INVALID_PARAM;
    }
    
    // 读取MQ传感器
    data->mq2_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ2_PIN, 0);
    data->mq3_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ3_PIN, 1);
    data->mq4_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ4_PIN, 2);
    data->mq5_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ5_PIN, 3);
    data->mq6_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ6_PIN, 4);
    data->mq7_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ7_PIN, 5);
    data->mq8_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ8_PIN, 6);
    data->mq9_value = ROTS_SensorManager_ReadMQSensor(ROTS_MQ9_PIN, 7);
    
    // 读取环境传感器
    data->temperature = ROTS_SensorManager_ReadTemperature();
    data->humidity = ROTS_SensorManager_ReadHumidity();
    data->pressure = ROTS_SensorManager_ReadPressure();
    
    // 设置时间戳
    data->timestamp = millis();
    
    // 应用校准和补偿
    ROTS_SensorManager_ApplyCalibration(data);
    ROTS_SensorManager_ApplyTemperatureCompensation(data);
    
    // 更新历史数据
    ROTS_SensorManager_UpdateHistory(data);
    
    return ROTS_OK;
}

// 更新传感器数据
void ROTS_SensorManager_UpdateData(const ROTS_SensorData_t* data) {
    if (!data) return;
    
    memcpy(&current_sensor_data, data, sizeof(ROTS_SensorData_t));
}

// 获取当前传感器数据
ROTS_StatusTypeDef ROTS_SensorManager_GetCurrentData(ROTS_SensorData_t* data) {
    if (!sensor_initialized || !data) {
        return ROTS_INVALID_PARAM;
    }
    
    memcpy(data, &current_sensor_data, sizeof(ROTS_SensorData_t));
    return ROTS_OK;
}

// 获取传感器历史数据
ROTS_StatusTypeDef ROTS_SensorManager_GetHistoryData(ROTS_SensorData_t* data, uint8_t count) {
    if (!sensor_initialized || !data || count > 10) {
        return ROTS_INVALID_PARAM;
    }
    
    uint8_t start_index = (history_index - count + 10) % 10;
    for (uint8_t i = 0; i < count; i++) {
        memcpy(&data[i], &sensor_history[(start_index + i) % 10], sizeof(ROTS_SensorData_t));
    }
    
    return ROTS_OK;
}

// 校准传感器
ROTS_StatusTypeDef ROTS_SensorManager_CalibrateSensors(void) {
    DEBUG_INFO("Starting sensor calibration...\r\n");
    
    // 在清洁空气中校准
    float calibration_values[ROTS_MAX_SENSORS];
    const int calibration_samples = 100;
    
    for (int sensor = 0; sensor < ROTS_MAX_SENSORS; sensor++) {
        float sum = 0;
        for (int i = 0; i < calibration_samples; i++) {
            sum += analogRead(ROTS_MQ2_PIN + sensor);
            delay(10);
        }
        calibration_values[sensor] = sum / calibration_samples;
        sensor_calibration[sensor] = 4095.0f / calibration_values[sensor];
    }
    
    DEBUG_INFO("Sensor calibration completed\r\n");
    return ROTS_OK;
}

// 读取MQ传感器
static float ROTS_SensorManager_ReadMQSensor(uint8_t pin, uint8_t sensor_id) {
    // 读取模拟值
    int raw_value = analogRead(pin);
    
    // 转换为电压值 (0-3.3V)
    float voltage = (raw_value * 3.3f) / 4095.0f;
    
    // 应用校准
    float calibrated_value = voltage * sensor_calibration[sensor_id];
    
    // 转换为电阻值 (假设RL=10kΩ)
    float resistance = (3.3f - calibrated_value) * 10000.0f / calibrated_value;
    
    // 防止除零错误
    if (resistance < 1.0f) resistance = 1.0f;
    
    // 转换为气体浓度 (简化计算)
    float concentration = pow(10, (log10(resistance) - 2.0f) / 0.8f);
    
    // 限制浓度范围
    if (concentration > 1000.0f) concentration = 1000.0f;
    if (concentration < 0.1f) concentration = 0.1f;
    
    return concentration;
}

// 读取温度
float ROTS_SensorManager_ReadTemperature(void) {
    // 这里应该实现DHT22或BMP280温度读取
    // 暂时返回模拟值
    return 25.0f + random(-5, 5);
}

// 读取湿度
float ROTS_SensorManager_ReadHumidity(void) {
    // 这里应该实现DHT22湿度读取
    // 暂时返回模拟值
    return 50.0f + random(-10, 10);
}

// 读取气压
float ROTS_SensorManager_ReadPressure(void) {
    // 这里应该实现BMP280气压读取
    // 暂时返回模拟值
    return 1013.25f + random(-10, 10);
}

// 应用校准
static void ROTS_SensorManager_ApplyCalibration(ROTS_SensorData_t* data) {
    data->mq2_value *= sensor_calibration[0];
    data->mq3_value *= sensor_calibration[1];
    data->mq4_value *= sensor_calibration[2];
    data->mq5_value *= sensor_calibration[3];
    data->mq6_value *= sensor_calibration[4];
    data->mq7_value *= sensor_calibration[5];
    data->mq8_value *= sensor_calibration[6];
    data->mq9_value *= sensor_calibration[7];
}

// 应用温度补偿
static void ROTS_SensorManager_ApplyTemperatureCompensation(ROTS_SensorData_t* data) {
    float temp_factor = 1.0f + (data->temperature - 25.0f) * temperature_compensation;
    
    data->mq2_value *= temp_factor;
    data->mq3_value *= temp_factor;
    data->mq4_value *= temp_factor;
    data->mq5_value *= temp_factor;
    data->mq6_value *= temp_factor;
    data->mq7_value *= temp_factor;
    data->mq8_value *= temp_factor;
    data->mq9_value *= temp_factor;
}

// 更新历史数据
static void ROTS_SensorManager_UpdateHistory(const ROTS_SensorData_t* data) {
    memcpy(&sensor_history[history_index], data, sizeof(ROTS_SensorData_t));
    history_index = (history_index + 1) % 10;
}

// 获取传感器状态
ROTS_StatusTypeDef ROTS_SensorManager_GetStatus(ROTS_SensorStatus_t* status) {
    if (!sensor_initialized || !status) {
        return ROTS_INVALID_PARAM;
    }
    
    status->initialized = sensor_initialized;
    status->last_read_time = current_sensor_data.timestamp;
    status->temperature = current_sensor_data.temperature;
    status->humidity = current_sensor_data.humidity;
    status->pressure = current_sensor_data.pressure;
    
    // 计算传感器健康状态
    status->sensor_health = 100; // 简化实现
    
    return ROTS_OK;
}
