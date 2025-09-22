/**
 * @file rots_actuator_control.c
 * @brief ROTS Actuator Control Module
 * @author ROTS Team
 * @date 2024
 * 
 * Controls pumps, valves, and fans for odor generation
 */

#include "rots_receiver.h"
#include "rots_actuator_control.h"
#include <math.h>

/* Private variables */
static TIM_HandleTypeDef htim_pwm;
static ROTS_ActuatorState_t pump_states[ROTS_MAX_PUMPS];
static ROTS_ActuatorState_t valve_states[ROTS_MAX_VALVES];
static ROTS_ActuatorState_t fan_states[ROTS_MAX_FANS];
static uint8_t pump_speeds[ROTS_MAX_PUMPS];
static bool system_initialized = false;

/* Private function prototypes */
static ROTS_StatusTypeDef ROTS_ActuatorControl_InitPWM(void);
static ROTS_StatusTypeDef ROTS_ActuatorControl_InitGPIO(void);
static void ROTS_ActuatorControl_SetPumpSpeed(uint8_t pump_id, uint8_t speed);
static void ROTS_ActuatorControl_SetValveState(uint8_t valve_id, ROTS_ActuatorState_t state);
static void ROTS_ActuatorControl_SetFanSpeed(uint8_t fan_id, uint8_t speed);

/**
 * @brief Initialize actuator control system
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_Init(void)
{
    ROTS_StatusTypeDef status = ROTS_OK;
    
    // Initialize PWM for pumps
    status = ROTS_ActuatorControl_InitPWM();
    if (status != ROTS_OK) return status;
    
    // Initialize GPIO for valves and fans
    status = ROTS_ActuatorControl_InitGPIO();
    if (status != ROTS_OK) return status;
    
    // Initialize all actuators to OFF state
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        pump_states[i] = ROTS_ACTUATOR_OFF;
        pump_speeds[i] = 0;
    }
    
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        valve_states[i] = ROTS_ACTUATOR_OFF;
    }
    
    for (int i = 0; i < ROTS_MAX_FANS; i++) {
        fan_states[i] = ROTS_ACTUATOR_OFF;
    }
    
    system_initialized = true;
    return ROTS_OK;
}

/**
 * @brief Process odor command and control actuators
 * @param message Odor command message
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_ProcessOdorCommand(ROTS_MessageTypeDef* message)
{
    if (!system_initialized) {
        return ROTS_ERROR;
    }
    
    ROTS_StatusTypeDef status = ROTS_OK;
    
    // Emergency stop check
    if (message->message_type == ROTS_MSG_EMERGENCY_STOP) {
        ROTS_ActuatorControl_EmergencyStop();
        return ROTS_OK;
    }
    
    // Process odor command
    if (message->message_type == ROTS_MSG_ODOR_COMMAND) {
        // Configure pumps based on odor type and intensity
        status = ROTS_ActuatorControl_ConfigurePumps(message);
        if (status != ROTS_OK) return status;
        
        // Configure valves
        status = ROTS_ActuatorControl_ConfigureValves(message);
        if (status != ROTS_OK) return status;
        
        // Configure fans
        status = ROTS_ActuatorControl_ConfigureFans(message);
        if (status != ROTS_OK) return status;
        
        // Start odor generation
        status = ROTS_ActuatorControl_StartOdorGeneration(message->duration);
    }
    
    return status;
}

/**
 * @brief Configure pumps for odor generation
 * @param message Odor command message
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_ConfigurePumps(ROTS_MessageTypeDef* message)
{
    // Get recipe for the specified odor type
    ROTS_Recipe_t recipe;
    ROTS_StatusTypeDef status = ROTS_RecipeManager_GetRecipe(message->odor_type, &recipe);
    if (status != ROTS_OK) return status;
    
    // Configure each pump based on recipe and intensity
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        uint8_t pump_speed = (recipe.pump_ratios[i] * message->intensity) / 100;
        ROTS_ActuatorControl_SetPumpSpeed(i, pump_speed);
    }
    
    return ROTS_OK;
}

/**
 * @brief Configure valves for odor generation
 * @param message Odor command message
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_ConfigureValves(ROTS_MessageTypeDef* message)
{
    // Get recipe for the specified odor type
    ROTS_Recipe_t recipe;
    ROTS_StatusTypeDef status = ROTS_RecipeManager_GetRecipe(message->odor_type, &recipe);
    if (status != ROTS_OK) return status;
    
    // Configure valves based on recipe
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        ROTS_ActuatorState_t valve_state = recipe.valve_states[i] ? ROTS_ACTUATOR_ON : ROTS_ACTUATOR_OFF;
        ROTS_ActuatorControl_SetValveState(i, valve_state);
    }
    
    return ROTS_OK;
}

/**
 * @brief Configure fans for odor generation
 * @param message Odor command message
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_ConfigureFans(ROTS_MessageTypeDef* message)
{
    // Configure fans based on intensity
    uint8_t fan_speed = (message->intensity * 255) / 100;
    
    for (int i = 0; i < ROTS_MAX_FANS; i++) {
        ROTS_ActuatorControl_SetFanSpeed(i, fan_speed);
    }
    
    return ROTS_OK;
}

/**
 * @brief Start odor generation process
 * @param duration Duration in seconds
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_StartOdorGeneration(uint16_t duration)
{
    // Start all configured pumps
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        if (pump_speeds[i] > 0) {
            pump_states[i] = ROTS_ACTUATOR_ON;
        }
    }
    
    // Set generation timer
    // This would typically use a timer interrupt
    // For now, we'll use a simple delay approach
    
    return ROTS_OK;
}

/**
 * @brief Stop odor generation
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_StopOdorGeneration(void)
{
    // Stop all pumps
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        pump_states[i] = ROTS_ACTUATOR_OFF;
        pump_speeds[i] = 0;
    }
    
    // Close all valves
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        valve_states[i] = ROTS_ACTUATOR_OFF;
    }
    
    // Stop fans
    for (int i = 0; i < ROTS_MAX_FANS; i++) {
        fan_states[i] = ROTS_ACTUATOR_OFF;
    }
    
    return ROTS_OK;
}

/**
 * @brief Emergency stop all actuators
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_EmergencyStop(void)
{
    // Immediately stop all actuators
    ROTS_ActuatorControl_StopOdorGeneration();
    
    // Log emergency stop event
    ROTS_SystemMonitor_LogError(ROTS_ERROR);
    
    return ROTS_OK;
}

/**
 * @brief Get actuator status
 * @param pump_status Array to store pump status
 * @param valve_status Array to store valve status
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_ActuatorControl_GetStatus(uint8_t* pump_status, uint8_t* valve_status)
{
    if (!pump_status || !valve_status) {
        return ROTS_INVALID_PARAM;
    }
    
    // Copy pump status
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        pump_status[i] = pump_states[i];
    }
    
    // Copy valve status
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        valve_status[i] = valve_states[i];
    }
    
    return ROTS_OK;
}

/**
 * @brief Initialize PWM for pump control
 * @return ROTS_OK if successful, error code otherwise
 */
static ROTS_StatusTypeDef ROTS_ActuatorControl_InitPWM(void)
{
    // Configure TIM2 for PWM generation
    htim_pwm.Instance = TIM2;
    htim_pwm.Init.Prescaler = 84 - 1;  // 1MHz
    htim_pwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_pwm.Init.Period = 1000 - 1;   // 1kHz PWM frequency
    htim_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    if (HAL_TIM_PWM_Init(&htim_pwm) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    // Configure PWM channels
    TIM_OC_InitTypeDef sConfigOC;
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    // Configure each pump PWM channel
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_4);
    
    // Start PWM
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_4);
    
    return ROTS_OK;
}

/**
 * @brief Initialize GPIO for valves and fans
 * @return ROTS_OK if successful, error code otherwise
 */
static ROTS_StatusTypeDef ROTS_ActuatorControl_InitGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure valve pins
    GPIO_InitStruct.Pin = VALVE1_PIN | VALVE2_PIN | VALVE3_PIN | VALVE4_PIN | VALVE5_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(VALVE1_PORT, &GPIO_InitStruct);
    
    // Configure fan pins
    GPIO_InitStruct.Pin = FAN1_PIN | FAN2_PIN;
    HAL_GPIO_Init(FAN1_PORT, &GPIO_InitStruct);
    
    // Configure pump enable pins
    GPIO_InitStruct.Pin = PUMP1_EN_PIN | PUMP2_EN_PIN | PUMP3_EN_PIN | PUMP4_EN_PIN | PUMP5_EN_PIN;
    HAL_GPIO_Init(PUMP1_EN_PORT, &GPIO_InitStruct);
    
    return ROTS_OK;
}

/**
 * @brief Set pump speed
 * @param pump_id Pump ID (0-4)
 * @param speed Speed (0-100%)
 */
static void ROTS_ActuatorControl_SetPumpSpeed(uint8_t pump_id, uint8_t speed)
{
    if (pump_id >= ROTS_MAX_PUMPS) return;
    
    pump_speeds[pump_id] = speed;
    
    // Convert percentage to PWM value
    uint32_t pwm_value = (speed * 1000) / 100;
    
    // Set PWM duty cycle based on pump ID
    switch (pump_id) {
        case 0:
            __HAL_TIM_SET_COMPARE(&htim_pwm, TIM_CHANNEL_1, pwm_value);
            break;
        case 1:
            __HAL_TIM_SET_COMPARE(&htim_pwm, TIM_CHANNEL_2, pwm_value);
            break;
        case 2:
            __HAL_TIM_SET_COMPARE(&htim_pwm, TIM_CHANNEL_3, pwm_value);
            break;
        case 3:
            __HAL_TIM_SET_COMPARE(&htim_pwm, TIM_CHANNEL_4, pwm_value);
            break;
        case 4:
            // Use TIM3 for pump 5 if needed
            break;
    }
}

/**
 * @brief Set valve state
 * @param valve_id Valve ID (0-4)
 * @param state Valve state
 */
static void ROTS_ActuatorControl_SetValveState(uint8_t valve_id, ROTS_ActuatorState_t state)
{
    if (valve_id >= ROTS_MAX_VALVES) return;
    
    valve_states[valve_id] = state;
    
    GPIO_PinState pin_state = (state == ROTS_ACTUATOR_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    
    // Set valve pin based on valve ID
    switch (valve_id) {
        case 0:
            HAL_GPIO_WritePin(VALVE1_PORT, VALVE1_PIN, pin_state);
            break;
        case 1:
            HAL_GPIO_WritePin(VALVE2_PORT, VALVE2_PIN, pin_state);
            break;
        case 2:
            HAL_GPIO_WritePin(VALVE3_PORT, VALVE3_PIN, pin_state);
            break;
        case 3:
            HAL_GPIO_WritePin(VALVE4_PORT, VALVE4_PIN, pin_state);
            break;
        case 4:
            HAL_GPIO_WritePin(VALVE5_PORT, VALVE5_PIN, pin_state);
            break;
    }
}

/**
 * @brief Set fan speed
 * @param fan_id Fan ID (0-1)
 * @param speed Speed (0-255)
 */
static void ROTS_ActuatorControl_SetFanSpeed(uint8_t fan_id, uint8_t speed)
{
    if (fan_id >= ROTS_MAX_FANS) return;
    
    fan_states[fan_id] = (speed > 0) ? ROTS_ACTUATOR_ON : ROTS_ACTUATOR_OFF;
    
    // Set fan pin based on fan ID
    GPIO_PinState pin_state = (speed > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    
    switch (fan_id) {
        case 0:
            HAL_GPIO_WritePin(FAN1_PORT, FAN1_PIN, pin_state);
            break;
        case 1:
            HAL_GPIO_WritePin(FAN2_PORT, FAN2_PIN, pin_state);
            break;
    }
}
