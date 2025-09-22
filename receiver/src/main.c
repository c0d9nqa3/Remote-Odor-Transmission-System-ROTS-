// ROTS Receiver Main Application

#include "rots_receiver.h"
#include "rots_communication.h"
#include "rots_actuator_control.h"
#include "rots_recipe_manager.h"
#include "rots_display.h"
#include "rots_system_monitor.h"
#include "rots_debug.h"
#include "rots_hardware.h"

static ROTS_StatusTypeDef ROTS_SystemInit(void);
static void ROTS_MainLoop(void);
static void ROTS_ErrorHandler(ROTS_StatusTypeDef error_code);

int main(void)
{
    ROTS_StatusTypeDef status;
    
    // Initialize system
    status = ROTS_SystemInit();
    if (status != ROTS_OK) {
        ROTS_ErrorHandler(status);
    }
    
    // Display startup message
    ROTS_Display_ShowMessage("ROTS Receiver", "Initializing...");
    
    // Main application loop
    ROTS_MainLoop();
    
    return 0;
}

static ROTS_StatusTypeDef ROTS_SystemInit(void)
{
    ROTS_StatusTypeDef status = ROTS_OK;
    
    // Initialize HAL
    HAL_Init();
    
    // Initialize system clock
    status = ROTS_SystemClock_Init();
    if (status != ROTS_OK) return status;
    
    // Initialize hardware
    status = ROTS_GPIO_Init();
    if (status != ROTS_OK) return status;
    
    status = ROTS_PWM_Init();
    if (status != ROTS_OK) return status;
    
    status = ROTS_UART_Init();
    if (status != ROTS_OK) return status;
    
    status = ROTS_I2C_Init();
    if (status != ROTS_OK) return status;
    
    // Initialize debug system
    status = ROTS_Debug_Init();
    if (status != ROTS_OK) return status;
    
    DEBUG_INFO("ROTS Receiver Starting...\r\n");
    
    // Hardware self-test
    status = ROTS_Hardware_SelfTest();
    if (status != ROTS_OK) return status;
    
    // Initialize communication module
    status = ROTS_Communication_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Communication init failed\r\n");
        return status;
    }
    
    // Initialize actuator control
    status = ROTS_ActuatorControl_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Actuator control init failed\r\n");
        return status;
    }
    
    // Initialize recipe manager
    status = ROTS_RecipeManager_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Recipe manager init failed\r\n");
        return status;
    }
    
    // Initialize display
    status = ROTS_Display_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("Display init failed\r\n");
        return status;
    }
    
    // Initialize system monitor
    status = ROTS_SystemMonitor_Init();
    if (status != ROTS_OK) {
        DEBUG_ERROR("System monitor init failed\r\n");
        return status;
    }
    
    DEBUG_INFO("System initialization completed\r\n");
    return ROTS_OK;
}

static void ROTS_MainLoop(void)
{
    ROTS_MessageTypeDef received_msg;
    ROTS_StatusTypeDef status;
    uint32_t last_debug_time = 0;
    uint32_t last_status_time = 0;
    
    DEBUG_INFO("Entering main loop\r\n");
    
    while (1) {
        // Check for incoming messages
        status = ROTS_Communication_ReceiveMessage(&received_msg);
        if (status == ROTS_OK) {
            DEBUG_INFO("Received message\r\n");
            ROTS_Debug_PrintMessage(&received_msg);
            
            // Process received odor command
            status = ROTS_ActuatorControl_ProcessOdorCommand(&received_msg);
            if (status != ROTS_OK) {
                DEBUG_ERROR("Failed to process command: %d\r\n", status);
            }
        } else if (status == ROTS_COMM_ERROR) {
            DEBUG_ERROR("Communication error\r\n");
        }
        
        // Update system status every 1 second
        if ((HAL_GetTick() - last_status_time) >= 1000) {
            ROTS_SystemMonitor_Update();
            last_status_time = HAL_GetTick();
        }
        
        // Update display every 500ms
        ROTS_Display_Update();
        
        // Print debug info every 10 seconds
        if ((HAL_GetTick() - last_debug_time) >= 10000) {
            ROTS_Debug_PrintSystemStatus();
            ROTS_Debug_PrintWiFiStatus();
            ROTS_Debug_PrintMQTTStatus();
            ROTS_Debug_PrintMemoryUsage();
            last_debug_time = HAL_GetTick();
        }
        
        // Small delay to prevent CPU overload
        HAL_Delay(10);
    }
}

static void ROTS_ErrorHandler(ROTS_StatusTypeDef error_code)
{
    // Display error on screen
    ROTS_Display_ShowError(error_code);
    
    // Log error
    ROTS_SystemMonitor_LogError(error_code);
    
    // Enter error state
    while (1) {
        // Blink error LED
        HAL_GPIO_TogglePin(ERROR_LED_PORT, ERROR_LED_PIN);
        HAL_Delay(500);
    }
}
