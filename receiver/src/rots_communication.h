/**
 * @file rots_communication.h
 * @brief ROTS Communication Module Header
 * @author ROTS Team
 * @date 2024
 * 
 * Header file for communication between ESP32 and STM32
 */

#ifndef ROTS_COMMUNICATION_H
#define ROTS_COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "rots_receiver.h"

/* Function Prototypes */
ROTS_StatusTypeDef ROTS_Communication_Init(void);
ROTS_StatusTypeDef ROTS_Communication_ReceiveMessage(ROTS_MessageTypeDef* message);
ROTS_StatusTypeDef ROTS_Communication_SendStatus(ROTS_SystemStatus_t* status);
ROTS_StatusTypeDef ROTS_Communication_SendError(ROTS_StatusTypeDef error_code);

/* UART Callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_COMMUNICATION_H */
