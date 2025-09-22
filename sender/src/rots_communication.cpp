// ROTS Communication Module - 通信模块
#include "rots_sender.h"
#include "rots_communication.h"
#include "rots_debug.h"

// 私有变量
static WiFiClient wifi_client;
static PubSubClient mqtt_client(wifi_client);
static bool wifi_connected = false;
static bool mqtt_connected = false;
static uint32_t last_connection_attempt = 0;
static uint32_t last_heartbeat = 0;

// 私有函数声明
static void ROTS_Communication_MQTTCallback(char* topic, byte* payload, unsigned int length);
static ROTS_StatusTypeDef ROTS_Communication_ConnectWiFi(void);
static ROTS_StatusTypeDef ROTS_Communication_ConnectMQTT(void);
static void ROTS_Communication_SendHeartbeat(void);

// 初始化通信模块
ROTS_StatusTypeDef ROTS_Communication_Init(void) {
    DEBUG_INFO("Initializing communication...\r\n");
    
    // 连接WiFi
    ROTS_StatusTypeDef status = ROTS_Communication_ConnectWiFi();
    if (status != ROTS_OK) {
        DEBUG_ERROR("WiFi connection failed\r\n");
        return status;
    }
    
    // 配置MQTT
    mqtt_client.setServer(ROTS_MQTT_BROKER_HOST, ROTS_MQTT_BROKER_PORT);
    mqtt_client.setCallback(ROTS_Communication_MQTTCallback);
    
    // 连接MQTT
    status = ROTS_Communication_ConnectMQTT();
    if (status != ROTS_OK) {
        DEBUG_ERROR("MQTT connection failed\r\n");
        return status;
    }
    
    DEBUG_INFO("Communication initialized\r\n");
    return ROTS_OK;
}

// 连接WiFi
static ROTS_StatusTypeDef ROTS_Communication_ConnectWiFi(void) {
    DEBUG_INFO("Connecting to WiFi: %s\r\n", ROTS_WIFI_SSID);
    
    WiFi.begin(ROTS_WIFI_SSID, ROTS_WIFI_PASSWORD);
    
    uint32_t start_time = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start_time > ROTS_WIFI_TIMEOUT_MS) {
            DEBUG_ERROR("WiFi connection timeout\r\n");
            return ROTS_TIMEOUT;
        }
        delay(500);
        DEBUG_DEBUG("WiFi connecting...\r\n");
    }
    
    wifi_connected = true;
    DEBUG_INFO("WiFi connected: %s\r\n", WiFi.localIP().toString().c_str());
    return ROTS_OK;
}

// 连接MQTT
static ROTS_StatusTypeDef ROTS_Communication_ConnectMQTT(void) {
    DEBUG_INFO("Connecting to MQTT broker...\r\n");
    
    if (!mqtt_client.connect(ROTS_MQTT_CLIENT_ID)) {
        DEBUG_ERROR("MQTT connection failed: %d\r\n", mqtt_client.state());
        return ROTS_COMM_ERROR;
    }
    
    // 订阅状态主题
    if (!mqtt_client.subscribe(ROTS_MQTT_TOPIC_STATUS)) {
        DEBUG_ERROR("Failed to subscribe to status topic\r\n");
        return ROTS_COMM_ERROR;
    }
    
    mqtt_connected = true;
    DEBUG_INFO("MQTT connected\r\n");
    return ROTS_OK;
}

// 发送气味检测结果
ROTS_StatusTypeDef ROTS_Communication_SendOdorDetection(const ROTS_OdorResult_t* result) {
    if (!mqtt_connected || !result) {
        return ROTS_INVALID_PARAM;
    }
    
    // 创建JSON消息
    DynamicJsonDocument doc(512);
    doc["device_id"] = ROTS_MQTT_CLIENT_ID;
    doc["odor_type"] = result->odor_type;
    doc["odor_name"] = result->odor_name;
    doc["confidence"] = result->confidence;
    doc["intensity"] = result->intensity;
    doc["timestamp"] = result->timestamp;
    
    // 序列化JSON
    char json_string[512];
    serializeJson(doc, json_string);
    
    // 发送MQTT消息
    if (!mqtt_client.publish(ROTS_MQTT_TOPIC_DETECTION, json_string)) {
        DEBUG_ERROR("Failed to publish detection result\r\n");
        return ROTS_COMM_ERROR;
    }
    
    DEBUG_INFO("Odor detection sent: %s\r\n", result->odor_name);
    return ROTS_OK;
}

// 发送状态信息
ROTS_StatusTypeDef ROTS_Communication_SendStatus(const ROTS_SenderStatus_t* status) {
    if (!mqtt_connected || !status) {
        return ROTS_INVALID_PARAM;
    }
    
    // 创建JSON消息
    DynamicJsonDocument doc(256);
    doc["device_id"] = ROTS_MQTT_CLIENT_ID;
    doc["state"] = status->state;
    doc["detection_count"] = status->detection_count;
    doc["error_count"] = status->error_count;
    doc["battery_voltage"] = status->battery_voltage;
    doc["timestamp"] = millis();
    
    // 序列化JSON
    char json_string[256];
    serializeJson(doc, json_string);
    
    // 发送MQTT消息
    if (!mqtt_client.publish(ROTS_MQTT_TOPIC_STATUS, json_string)) {
        DEBUG_ERROR("Failed to publish status\r\n");
        return ROTS_COMM_ERROR;
    }
    
    return ROTS_OK;
}

// 发送错误信息
ROTS_StatusTypeDef ROTS_Communication_SendError(ROTS_StatusTypeDef error_code) {
    if (!mqtt_connected) {
        return ROTS_COMM_ERROR;
    }
    
    // 创建JSON消息
    DynamicJsonDocument doc(128);
    doc["device_id"] = ROTS_MQTT_CLIENT_ID;
    doc["error_code"] = error_code;
    doc["timestamp"] = millis();
    
    // 序列化JSON
    char json_string[128];
    serializeJson(doc, json_string);
    
    // 发送MQTT消息
    if (!mqtt_client.publish(ROTS_MQTT_TOPIC_ERROR, json_string)) {
        DEBUG_ERROR("Failed to publish error\r\n");
        return ROTS_COMM_ERROR;
    }
    
    DEBUG_ERROR("Error sent: %d\r\n", error_code);
    return ROTS_OK;
}

// 更新通信状态
ROTS_StatusTypeDef ROTS_Communication_Update(void) {
    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        if (wifi_connected) {
            DEBUG_WARNING("WiFi disconnected\r\n");
            wifi_connected = false;
        }
        
        // 尝试重连
        if (millis() - last_connection_attempt > 5000) {
            ROTS_Communication_ConnectWiFi();
            last_connection_attempt = millis();
        }
    } else if (!wifi_connected) {
        DEBUG_INFO("WiFi reconnected\r\n");
        wifi_connected = true;
    }
    
    // 检查MQTT连接
    if (wifi_connected && !mqtt_client.connected()) {
        if (mqtt_connected) {
            DEBUG_WARNING("MQTT disconnected\r\n");
            mqtt_connected = false;
        }
        
        // 尝试重连
        if (millis() - last_connection_attempt > 5000) {
            ROTS_Communication_ConnectMQTT();
            last_connection_attempt = millis();
        }
    } else if (wifi_connected && !mqtt_connected) {
        DEBUG_INFO("MQTT reconnected\r\n");
        mqtt_connected = true;
    }
    
    // 处理MQTT消息
    if (mqtt_connected) {
        mqtt_client.loop();
    }
    
    // 发送心跳包
    if (millis() - last_heartbeat > 30000) { // 每30秒
        ROTS_Communication_SendHeartbeat();
        last_heartbeat = millis();
    }
    
    return ROTS_OK;
}

// MQTT回调函数
static void ROTS_Communication_MQTTCallback(char* topic, byte* payload, unsigned int length) {
    DEBUG_DEBUG("MQTT message received: %s\r\n", topic);
    
    // 解析JSON消息
    DynamicJsonDocument doc(256);
    deserializeJson(doc, payload, length);
    
    // 处理不同类型的消息
    if (strstr(topic, "status") != NULL) {
        // 处理状态消息
        DEBUG_INFO("Status message received\r\n");
    } else if (strstr(topic, "command") != NULL) {
        // 处理命令消息
        DEBUG_INFO("Command message received\r\n");
    }
}

// 发送心跳包
static void ROTS_Communication_SendHeartbeat(void) {
    if (!mqtt_connected) return;
    
    // 创建心跳消息
    DynamicJsonDocument doc(128);
    doc["device_id"] = ROTS_MQTT_CLIENT_ID;
    doc["type"] = "heartbeat";
    doc["timestamp"] = millis();
    
    // 序列化JSON
    char json_string[128];
    serializeJson(doc, json_string);
    
    // 发送MQTT消息
    mqtt_client.publish("rots/heartbeat/001", json_string);
    
    DEBUG_DEBUG("Heartbeat sent\r\n");
}

// 获取通信状态
ROTS_StatusTypeDef ROTS_Communication_GetStatus(ROTS_CommStatus_t* status) {
    if (!status) {
        return ROTS_INVALID_PARAM;
    }
    
    status->wifi_connected = wifi_connected;
    status->mqtt_connected = mqtt_connected;
    status->wifi_rssi = WiFi.RSSI();
    status->last_heartbeat = last_heartbeat;
    
    return ROTS_OK;
}
