// ROTS System Monitor Header
#ifndef ROTS_SYSTEM_MONITOR_H
#define ROTS_SYSTEM_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rots_sender.h"

// 系统状态结构
typedef struct {
    uint32_t uptime;
    uint32_t free_heap;
    uint32_t free_psram;
    uint8_t error_count;
    bool wifi_connected;
    int32_t wifi_rssi;
    float battery_voltage;
} ROTS_SystemStatus_t;

// 系统信息结构
typedef struct {
    char chip_model[32];
    uint8_t chip_revision;
    uint32_t cpu_freq;
    uint32_t flash_size;
    uint32_t free_heap;
    uint32_t free_psram;
    uint32_t uptime;
} ROTS_SystemInfo_t;

// 函数声明
ROTS_StatusTypeDef ROTS_SystemMonitor_Init(void);
ROTS_StatusTypeDef ROTS_SystemMonitor_Update(void);
ROTS_StatusTypeDef ROTS_SystemMonitor_LogError(ROTS_StatusTypeDef error_code);
ROTS_StatusTypeDef ROTS_SystemMonitor_GetStatus(ROTS_SystemStatus_t* status);
ROTS_StatusTypeDef ROTS_SystemMonitor_GetErrorLog(uint32_t* log, uint8_t max_count, uint8_t* actual_count);
ROTS_StatusTypeDef ROTS_SystemMonitor_ClearErrorLog(void);
ROTS_StatusTypeDef ROTS_SystemMonitor_GetSystemInfo(ROTS_SystemInfo_t* info);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_SYSTEM_MONITOR_H */
