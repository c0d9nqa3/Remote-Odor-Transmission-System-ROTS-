// ROTS Sender Main Application - ESP32
#include "rots_sender.h"
#include "rots_sensor_manager.h"
#include "rots_ai_engine.h"
#include "rots_communication.h"
#include "rots_system_monitor.h"
#include "rots_debug.h"

// 全局变量
ROTS_SenderStatus_t sender_status;
bool system_initialized = false;

// 函数声明
void setup();
void loop();
ROTS_StatusTypeDef ROTS_Sender_Init(void);
void ROTS_Sender_MainLoop(void);
void ROTS_Sender_ErrorHandler(ROTS_StatusTypeDef error_code);

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    DEBUG_INFO("ROTS Sender Starting...\r\n");
    
    // 初始化系统
    ROTS_StatusTypeDef status = ROTS_Sender_Init();
    if (status != ROTS_OK) {
        ROTS_Sender_ErrorHandler(status);
        return;
    }
    
    system_initialized = true;
    DEBUG_INFO("System initialization completed\r\n");
}

void loop() {
    if (!system_initialized) {
        delay(1000);
        return;
    }
    
    ROTS_Sender_MainLoop();
    delay(10);
}

ROTS_StatusTypeDef ROTS_Sender_Init(void) {
    ROTS_StatusTypeDef status = ROTS_OK;
    
    // 初始化调试系统
    status = ROTS_Debug_Init();
    if (status != ROTS_OK) return status;
    
    // 初始化传感器管理器
    status = ROTS_SensorManager_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Sensor manager init failed\r\n");
        return status;
    }
    
    // 初始化AI引擎
    status = ROTS_AIEngine_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("AI engine init failed\r\n");
        return status;
    }
    
    // 初始化通信模块
    status = ROTS_Communication_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Communication init failed\r\n");
        return status;
    }
    
    // 初始化系统监控
    status = ROTS_SystemMonitor_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("System monitor init failed\r\n");
        return status;
    }
    
    // 初始化发送端状态
    sender_status.state = ROTS_SENDER_IDLE;
    sender_status.last_detection_time = 0;
    sender_status.detection_count = 0;
    sender_status.error_count = 0;
    
    return ROTS_OK;
}

void ROTS_Sender_MainLoop(void) {
    static uint32_t last_sensor_read = 0;
    static uint32_t last_ai_inference = 0;
    static uint32_t last_status_update = 0;
    static uint32_t last_debug_output = 0;
    
    uint32_t current_time = millis();
    
    // 读取传感器数据 (每100ms)
    if (current_time - last_sensor_read >= 100) {
        ROTS_SensorData_t sensor_data;
        ROTS_StatusTypeDef status = ROTS_SensorManager_ReadSensors(&sensor_data);
        
        if (status == ROTS_OK) {
            // 更新传感器数据
            ROTS_SensorManager_UpdateData(&sensor_data);
            last_sensor_read = current_time;
        } else {
            DEBUG_ERROR("Sensor read failed: %d\r\n", status);
        }
    }
    
    // AI推理 (每500ms)
    if (current_time - last_ai_inference >= 500) {
        ROTS_OdorResult_t ai_result;
        ROTS_StatusTypeDef status = ROTS_AIEngine_ProcessOdor(&ai_result);
        
        if (status == ROTS_OK && ai_result.confidence > ROTS_AI_CONFIDENCE_THRESHOLD) {
            DEBUG_INFO("Odor detected: %s (confidence: %.2f)\r\n", 
                      ai_result.odor_name, ai_result.confidence);
            
            // 发送检测结果
            ROTS_Communication_SendOdorDetection(&ai_result);
            
            // 更新状态
            sender_status.state = ROTS_SENDER_DETECTING;
            sender_status.last_detection_time = current_time;
            sender_status.detection_count++;
        } else if (current_time - sender_status.last_detection_time > 5000) {
            // 5秒内无检测，回到空闲状态
            sender_status.state = ROTS_SENDER_IDLE;
        }
        
        last_ai_inference = current_time;
    }
    
    // 更新系统状态 (每1秒)
    if (current_time - last_status_update >= 1000) {
        ROTS_SystemMonitor_Update();
        last_status_update = current_time;
    }
    
    // 调试输出 (每10秒)
    if (current_time - last_debug_output >= 10000) {
        ROTS_Debug_PrintSystemStatus();
        ROTS_Debug_PrintSensorStatus();
        ROTS_Debug_PrintAIStatus();
        ROTS_Debug_PrintMemoryUsage();
        last_debug_output = current_time;
    }
}

void ROTS_Sender_ErrorHandler(ROTS_StatusTypeDef error_code) {
    DEBUG_ERROR("System error: %d\r\n", error_code);
    
    // 显示错误LED
    digitalWrite(ROTS_ERROR_LED_PIN, HIGH);
    
    // 记录错误
    sender_status.error_count++;
    ROTS_SystemMonitor_LogError(error_code);
    
    // 尝试恢复
    delay(1000);
    digitalWrite(ROTS_ERROR_LED_PIN, LOW);
}
