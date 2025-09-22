// ROTS Receiver Main Application

#include "rots_receiver.h"
#include "rots_communication.h"
#include "rots_actuator_control.h"
#include "rots_recipe_manager.h"
#include "rots_display.h"
#include "rots_system_monitor.h"

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
    
    // Initialize system clock
    ROTS_SystemClock_Init();
    
    // Initialize communication module
    status = ROTS_Communication_Init();
    if (status != ROTS_OK) return status;
    
    // Initialize actuator control
    status = ROTS_ActuatorControl_Init();
    if (status != ROTS_OK) return status;
    
    // Initialize recipe manager
    status = ROTS_RecipeManager_Init();
    if (status != ROTS_OK) return status;
    
    // Initialize display
    status = ROTS_Display_Init();
    if (status != ROTS_OK) return status;
    
    // Initialize system monitor
    status = ROTS_SystemMonitor_Init();
    if (status != ROTS_OK) return status;
    
    return ROTS_OK;
}

static void ROTS_MainLoop(void)
{
    ROTS_MessageTypeDef received_msg;
    ROTS_StatusTypeDef status;
    
    while (1) {
        // Check for incoming messages
        status = ROTS_Communication_ReceiveMessage(&received_msg);
        if (status == ROTS_OK) {
            // Process received odor command
            ROTS_ProcessOdorCommand(&received_msg);
        }
        
        // Update system status
        ROTS_SystemMonitor_Update();
        
        // Update display
        ROTS_Display_Update();
        
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
