/**
 * @file rots_display.c
 * @brief ROTS Display Module
 * @author ROTS Team
 * @date 2024
 * 
 * Handles OLED display output for system status
 */

#include "rots_receiver.h"
#include "rots_display.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
static I2C_HandleTypeDef hi2c_display;
static bool display_initialized = false;
static char display_buffer[128];
static uint32_t last_update_time = 0;

/* Private function prototypes */
static ROTS_StatusTypeDef ROTS_Display_InitI2C(void);
static void ROTS_Display_ClearScreen(void);
static void ROTS_Display_WriteString(uint8_t x, uint8_t y, const char* str);
static void ROTS_Display_UpdateScreen(void);

/**
 * @brief Initialize display module
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_Display_Init(void)
{
    ROTS_StatusTypeDef status = ROTS_OK;
    
    // Initialize I2C for display
    status = ROTS_Display_InitI2C();
    if (status != ROTS_OK) return status;
    
    // Initialize SSD1306 display
    ROTS_Display_ClearScreen();
    ROTS_Display_WriteString(0, 0, "ROTS Receiver");
    ROTS_Display_WriteString(0, 1, "Initializing...");
    ROTS_Display_UpdateScreen();
    
    display_initialized = true;
    return ROTS_OK;
}

/**
 * @brief Show message on display
 * @param line1 First line text
 * @param line2 Second line text
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_Display_ShowMessage(const char* line1, const char* line2)
{
    if (!display_initialized) {
        return ROTS_DISPLAY_ERROR;
    }
    
    ROTS_Display_ClearScreen();
    ROTS_Display_WriteString(0, 0, line1);
    ROTS_Display_WriteString(0, 1, line2);
    ROTS_Display_UpdateScreen();
    
    return ROTS_OK;
}

/**
 * @brief Show error message on display
 * @param error_code Error code
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_Display_ShowError(ROTS_StatusTypeDef error_code)
{
    if (!display_initialized) {
        return ROTS_DISPLAY_ERROR;
    }
    
    ROTS_Display_ClearScreen();
    ROTS_Display_WriteString(0, 0, "ERROR");
    
    // Format error code
    snprintf(display_buffer, sizeof(display_buffer), "Code: %d", error_code);
    ROTS_Display_WriteString(0, 1, display_buffer);
    ROTS_Display_UpdateScreen();
    
    return ROTS_OK;
}

/**
 * @brief Update display with system status
 * @param status System status structure
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_Display_Update(void)
{
    if (!display_initialized) {
        return ROTS_DISPLAY_ERROR;
    }
    
    // Update every 500ms
    if ((HAL_GetTick() - last_update_time) < 500) {
        return ROTS_OK;
    }
    
    last_update_time = HAL_GetTick();
    
    // Show system status
    ROTS_Display_ClearScreen();
    ROTS_Display_WriteString(0, 0, "ROTS Receiver");
    
    // Show uptime
    uint32_t uptime = HAL_GetTick() / 1000;
    snprintf(display_buffer, sizeof(display_buffer), "Uptime: %lu s", uptime);
    ROTS_Display_WriteString(0, 1, display_buffer);
    
    ROTS_Display_UpdateScreen();
    
    return ROTS_OK;
}

/**
 * @brief Initialize I2C for display
 * @return ROTS_OK if successful, error code otherwise
 */
static ROTS_StatusTypeDef ROTS_Display_InitI2C(void)
{
    // Configure I2C1 for SSD1306 display
    hi2c_display.Instance = I2C1;
    hi2c_display.Init.ClockSpeed = 400000;
    hi2c_display.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c_display.Init.OwnAddress1 = 0;
    hi2c_display.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c_display.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c_display.Init.OwnAddress2 = 0;
    hi2c_display.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c_display.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c_display) != HAL_OK) {
        return ROTS_DISPLAY_ERROR;
    }
    
    return ROTS_OK;
}

/**
 * @brief Clear display screen
 */
static void ROTS_Display_ClearScreen(void)
{
    // Send clear command to SSD1306
    uint8_t clear_cmd[] = {0x00, 0x21, 0x00, 0x7F, 0x22, 0x00, 0x07};
    HAL_I2C_Master_Transmit(&hi2c_display, 0x3C << 1, clear_cmd, sizeof(clear_cmd), 100);
}

/**
 * @brief Write string to display
 * @param x X position
 * @param y Y position
 * @param str String to write
 */
static void ROTS_Display_WriteString(uint8_t x, uint8_t y, const char* str)
{
    // Set cursor position
    uint8_t pos_cmd[] = {0x00, 0x21, x, 0x7F, 0x22, y, y + 1};
    HAL_I2C_Master_Transmit(&hi2c_display, 0x3C << 1, pos_cmd, sizeof(pos_cmd), 100);
    
    // Send string data
    uint8_t data_cmd[64];
    data_cmd[0] = 0x40;  // Data mode
    strncpy((char*)&data_cmd[1], str, 63);
    data_cmd[63] = '\0';
    
    HAL_I2C_Master_Transmit(&hi2c_display, 0x3C << 1, data_cmd, strlen(str) + 1, 100);
}

/**
 * @brief Update display screen
 */
static void ROTS_Display_UpdateScreen(void)
{
    // Send display on command
    uint8_t display_on[] = {0x00, 0xAF};
    HAL_I2C_Master_Transmit(&hi2c_display, 0x3C << 1, display_on, sizeof(display_on), 100);
}
