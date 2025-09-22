// ROTS Communication Module Header
#ifndef ROTS_COMMUNICATION_H
#define ROTS_COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rots_sender.h"

// 通信状态结构
typedef struct {
    bool wifi_connected;
    bool mqtt_connected;
    int32_t wifi_rssi;
    uint32_t last_heartbeat;
} ROTS_CommStatus_t;

// 函数声明
ROTS_StatusTypeDef ROTS_Communication_Init(void);
ROTS_StatusTypeDef ROTS_Communication_SendOdorDetection(const ROTS_OdorResult_t* result);
ROTS_StatusTypeDef ROTS_Communication_SendStatus(const ROTS_SenderStatus_t* status);
ROTS_StatusTypeDef ROTS_Communication_SendError(ROTS_StatusTypeDef error_code);
ROTS_StatusTypeDef ROTS_Communication_Update(void);
ROTS_StatusTypeDef ROTS_Communication_GetStatus(ROTS_CommStatus_t* status);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_COMMUNICATION_H */
