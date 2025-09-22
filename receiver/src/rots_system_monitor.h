/**
 * @file rots_system_monitor.h
 * @brief ROTS System Monitor Module Header
 * @author ROTS Team
 * @date 2024
 * 
 * Header file for system monitoring and logging
 */

#ifndef ROTS_SYSTEM_MONITOR_H
#define ROTS_SYSTEM_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "rots_receiver.h"

/* Function Prototypes */
ROTS_StatusTypeDef ROTS_SystemMonitor_Init(void);
ROTS_StatusTypeDef ROTS_SystemMonitor_Update(void);
ROTS_StatusTypeDef ROTS_SystemMonitor_LogError(ROTS_StatusTypeDef error_code);
ROTS_StatusTypeDef ROTS_SystemMonitor_GetStatus(ROTS_SystemStatus_t* status);
ROTS_StatusTypeDef ROTS_SystemMonitor_GetErrorLog(uint32_t* log, uint8_t max_count, uint8_t* actual_count);
ROTS_StatusTypeDef ROTS_SystemMonitor_ClearErrorLog(void);
ROTS_StatusTypeDef ROTS_SystemMonitor_SetState(ROTS_SystemState_t state);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_SYSTEM_MONITOR_H */
