// ROTS Sender Main Header File
#ifndef ROTS_SENDER_H
#define ROTS_SENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>

// 系统状态码
typedef enum {
    ROTS_OK = 0x00,
    ROTS_ERROR = 0x01,
    ROTS_BUSY = 0x02,
    ROTS_TIMEOUT = 0x03,
    ROTS_INVALID_PARAM = 0x04,
    ROTS_COMM_ERROR = 0x05,
    ROTS_SENSOR_ERROR = 0x06,
    ROTS_AI_ERROR = 0x07,
    ROTS_MEMORY_ERROR = 0x08
} ROTS_StatusTypeDef;

// 发送端状态
typedef enum {
    ROTS_SENDER_IDLE = 0x00,
    ROTS_SENDER_DETECTING = 0x01,
    ROTS_SENDER_SENDING = 0x02,
    ROTS_SENDER_ERROR = 0x03
} ROTS_SenderState_t;

// 调试级别
typedef enum {
    ROTS_DEBUG_ERROR = 0,
    ROTS_DEBUG_WARNING = 1,
    ROTS_DEBUG_INFO = 2,
    ROTS_DEBUG_DEBUG = 3
} ROTS_DebugLevel_t;

// 气味类型
typedef enum {
    ROTS_ODOR_COFFEE = 0x01,
    ROTS_ODOR_ALCOHOL = 0x02,
    ROTS_ODOR_LEMON = 0x03,
    ROTS_ODOR_MINT = 0x04,
    ROTS_ODOR_LAVENDER = 0x05,
    ROTS_ODOR_UNKNOWN = 0x00
} ROTS_OdorType_t;

// 传感器数据结构
typedef struct {
    float mq2_value;      // MQ-2 可燃气体
    float mq3_value;      // MQ-3 酒精
    float mq4_value;      // MQ-4 甲烷
    float mq5_value;      // MQ-5 液化气
    float mq6_value;      // MQ-6 液化气
    float mq7_value;      // MQ-7 一氧化碳
    float mq8_value;      // MQ-8 氢气
    float mq9_value;      // MQ-9 一氧化碳
    float temperature;    // 温度
    float humidity;       // 湿度
    float pressure;       // 气压
    uint32_t timestamp;   // 时间戳
} ROTS_SensorData_t;

// AI推理结果
typedef struct {
    ROTS_OdorType_t odor_type;
    char odor_name[16];
    float confidence;
    float intensity;
    uint32_t timestamp;
} ROTS_OdorResult_t;

// 发送端状态
typedef struct {
    ROTS_SenderState_t state;
    uint32_t last_detection_time;
    uint32_t detection_count;
    uint32_t error_count;
    bool wifi_connected;
    bool mqtt_connected;
    float battery_voltage;
} ROTS_SenderStatus_t;

// 硬件引脚定义
#define ROTS_ERROR_LED_PIN        2
#define ROTS_STATUS_LED_PIN       4
#define ROTS_SENSOR_POWER_PIN     5

// MQ传感器引脚
#define ROTS_MQ2_PIN              A0
#define ROTS_MQ3_PIN              A1
#define ROTS_MQ4_PIN              A2
#define ROTS_MQ5_PIN              A3
#define ROTS_MQ6_PIN              A4
#define ROTS_MQ7_PIN              A5
#define ROTS_MQ8_PIN              A6
#define ROTS_MQ9_PIN              A7

// I2C引脚 (DHT22, BMP280)
#define ROTS_SDA_PIN              21
#define ROTS_SCL_PIN              22

// 系统配置
#define ROTS_MAX_SENSORS          8
#define ROTS_AI_CONFIDENCE_THRESHOLD  0.7f
#define ROTS_SENSOR_READ_INTERVAL     100    // ms
#define ROTS_AI_INFERENCE_INTERVAL    500    // ms
#define ROTS_STATUS_UPDATE_INTERVAL   1000   // ms
#define ROTS_DEBUG_OUTPUT_INTERVAL    10000  // ms

// WiFi配置
#define ROTS_WIFI_SSID            "ROTS_Network"
#define ROTS_WIFI_PASSWORD        "rots_password_2024"
#define ROTS_WIFI_TIMEOUT_MS      10000

// MQTT配置
#define ROTS_MQTT_BROKER_HOST     "mqtt.rots-system.com"
#define ROTS_MQTT_BROKER_PORT     1883
#define ROTS_MQTT_CLIENT_ID       "ROTS_SENDER_001"
#define ROTS_MQTT_TOPIC_DETECTION "rots/detection/001"
#define ROTS_MQTT_TOPIC_STATUS    "rots/status/001"
#define ROTS_MQTT_TOPIC_ERROR     "rots/error/001"

// 函数声明
ROTS_StatusTypeDef ROTS_Sender_Init(void);
void ROTS_Sender_MainLoop(void);
void ROTS_Sender_ErrorHandler(ROTS_StatusTypeDef error_code);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_SENDER_H */
