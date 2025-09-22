// ROTS System Monitor - 系统监控模块
#include "rots_sender.h"
#include "rots_system_monitor.h"
#include "rots_debug.h"

// 私有变量
static bool monitor_initialized = false;
static uint32_t error_log[32];
static uint8_t error_count = 0;
static uint32_t system_start_time = 0;

// 初始化系统监控
ROTS_StatusTypeDef ROTS_SystemMonitor_Init(void) {
    // 初始化错误日志
    memset(error_log, 0, sizeof(error_log));
    error_count = 0;
    
    // 记录系统启动时间
    system_start_time = millis();
    
    monitor_initialized = true;
    DEBUG_INFO("System monitor initialized\r\n");
    return ROTS_OK;
}

// 更新系统监控
ROTS_StatusTypeDef ROTS_SystemMonitor_Update(void) {
    if (!monitor_initialized) {
        return ROTS_ERROR;
    }
    
    // 检查内存使用
    if (ESP.getFreeHeap() < 10000) { // 小于10KB
        ROTS_SystemMonitor_LogError(ROTS_MEMORY_ERROR);
    }
    
    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        ROTS_SystemMonitor_LogError(ROTS_COMM_ERROR);
    }
    
    // 更新状态LED
    ROTS_Debug_StatusLED(WiFi.status() == WL_CONNECTED);
    
    return ROTS_OK;
}

// 记录错误
ROTS_StatusTypeDef ROTS_SystemMonitor_LogError(ROTS_StatusTypeDef error_code) {
    if (!monitor_initialized) {
        return ROTS_ERROR;
    }
    
    // 添加错误到日志
    if (error_count < 32) {
        error_log[error_count] = (uint32_t)error_code;
        error_count++;
    } else {
        // 循环覆盖
        for (int i = 0; i < 31; i++) {
            error_log[i] = error_log[i + 1];
        }
        error_log[31] = (uint32_t)error_code;
    }
    
    // 错误LED指示
    ROTS_Debug_ErrorLED(true);
    delay(100);
    ROTS_Debug_ErrorLED(false);
    
    DEBUG_ERROR("Error logged: %d\r\n", error_code);
    return ROTS_OK;
}

// 获取系统状态
ROTS_StatusTypeDef ROTS_SystemMonitor_GetStatus(ROTS_SystemStatus_t* status) {
    if (!monitor_initialized || !status) {
        return ROTS_INVALID_PARAM;
    }
    
    status->uptime = (millis() - system_start_time) / 1000;
    status->free_heap = ESP.getFreeHeap();
    status->free_psram = ESP.getFreePsram();
    status->error_count = error_count;
    status->wifi_connected = (WiFi.status() == WL_CONNECTED);
    status->wifi_rssi = WiFi.RSSI();
    status->battery_voltage = 3.7f; // 模拟值
    
    return ROTS_OK;
}

// 获取错误日志
ROTS_StatusTypeDef ROTS_SystemMonitor_GetErrorLog(uint32_t* log, uint8_t max_count, uint8_t* actual_count) {
    if (!monitor_initialized || !log || !actual_count) {
        return ROTS_INVALID_PARAM;
    }
    
    uint8_t count = (error_count < max_count) ? error_count : max_count;
    
    // 复制错误日志
    for (int i = 0; i < count; i++) {
        log[i] = error_log[i];
    }
    
    *actual_count = count;
    return ROTS_OK;
}

// 清除错误日志
ROTS_StatusTypeDef ROTS_SystemMonitor_ClearErrorLog(void) {
    if (!monitor_initialized) {
        return ROTS_ERROR;
    }
    
    memset(error_log, 0, sizeof(error_log));
    error_count = 0;
    
    DEBUG_INFO("Error log cleared\r\n");
    return ROTS_OK;
}

// 获取系统信息
ROTS_StatusTypeDef ROTS_SystemMonitor_GetSystemInfo(ROTS_SystemInfo_t* info) {
    if (!monitor_initialized || !info) {
        return ROTS_INVALID_PARAM;
    }
    
    strcpy(info->chip_model, ESP.getChipModel());
    info->chip_revision = ESP.getChipRevision();
    info->cpu_freq = ESP.getCpuFreqMHz();
    info->flash_size = ESP.getFlashChipSize();
    info->free_heap = ESP.getFreeHeap();
    info->free_psram = ESP.getFreePsram();
    info->uptime = (millis() - system_start_time) / 1000;
    
    return ROTS_OK;
}
