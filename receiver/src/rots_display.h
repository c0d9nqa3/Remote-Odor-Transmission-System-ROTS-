/**
 * @file rots_display.h
 * @brief ROTS Display Module Header
 * @author ROTS Team
 * @date 2024
 * 
 * Header file for OLED display control
 */

#ifndef ROTS_DISPLAY_H
#define ROTS_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "rots_receiver.h"

/* Function Prototypes */
ROTS_StatusTypeDef ROTS_Display_Init(void);
ROTS_StatusTypeDef ROTS_Display_ShowMessage(const char* line1, const char* line2);
ROTS_StatusTypeDef ROTS_Display_ShowError(ROTS_StatusTypeDef error_code);
ROTS_StatusTypeDef ROTS_Display_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_DISPLAY_H */
