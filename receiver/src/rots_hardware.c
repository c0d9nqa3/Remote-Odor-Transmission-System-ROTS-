// ROTS Hardware Driver - 硬件驱动
#include "rots_receiver.h"
#include "rots_hardware.h"
#include "rots_debug.h"

// 系统时钟配置
ROTS_StatusTypeDef ROTS_SystemClock_Init(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // 配置主振荡器
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    // 配置系统时钟
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    DEBUG_INFO("System clock initialized: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    return ROTS_OK;
}

// GPIO初始化
ROTS_StatusTypeDef ROTS_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIO时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    // 配置状态LED
    GPIO_InitStruct.Pin = ERROR_LED_PIN | STATUS_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ERROR_LED_PORT, &GPIO_InitStruct);
    
    // 配置泵使能引脚
    GPIO_InitStruct.Pin = PUMP1_EN_PIN | PUMP2_EN_PIN;
    HAL_GPIO_Init(PUMP1_EN_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = PUMP3_EN_PIN | PUMP4_EN_PIN | PUMP5_EN_PIN;
    HAL_GPIO_Init(PUMP3_EN_PORT, &GPIO_InitStruct);
    
    // 配置阀控制引脚
    GPIO_InitStruct.Pin = VALVE1_PIN | VALVE2_PIN | VALVE3_PIN | VALVE4_PIN | VALVE5_PIN;
    HAL_GPIO_Init(VALVE1_PORT, &GPIO_InitStruct);
    
    // 配置风扇控制引脚
    GPIO_InitStruct.Pin = FAN1_PIN | FAN2_PIN;
    HAL_GPIO_Init(FAN1_PORT, &GPIO_InitStruct);
    
    // 配置ESP8266控制引脚
    GPIO_InitStruct.Pin = GPIO_PIN_4; // ESP8266 Reset
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    DEBUG_INFO("GPIO initialized\r\n");
    return ROTS_OK;
}

// PWM初始化
ROTS_StatusTypeDef ROTS_PWM_Init(void)
{
    TIM_HandleTypeDef htim_pwm;
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    // 配置TIM2用于PWM
    htim_pwm.Instance = TIM2;
    htim_pwm.Init.Prescaler = 84 - 1;  // 1MHz
    htim_pwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_pwm.Init.Period = 1000 - 1;   // 1kHz PWM
    htim_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    if (HAL_TIM_PWM_Init(&htim_pwm) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    // 配置PWM通道
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    // 配置每个泵的PWM通道
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_4);
    
    // 启动PWM
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_4);
    
    DEBUG_INFO("PWM initialized\r\n");
    return ROTS_OK;
}

// UART初始化
ROTS_StatusTypeDef ROTS_UART_Init(void)
{
    UART_HandleTypeDef huart1, huart2;
    
    // 配置USART1用于调试
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    // 配置USART2用于ESP8266
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    DEBUG_INFO("UART initialized\r\n");
    return ROTS_OK;
}

// I2C初始化
ROTS_StatusTypeDef ROTS_I2C_Init(void)
{
    I2C_HandleTypeDef hi2c1;
    
    // 配置I2C1用于OLED显示
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        return ROTS_ERROR;
    }
    
    DEBUG_INFO("I2C initialized\r\n");
    return ROTS_OK;
}

// 硬件自检
ROTS_StatusTypeDef ROTS_Hardware_SelfTest(void)
{
    DEBUG_INFO("Starting hardware self-test...\r\n");
    
    // 测试LED
    HAL_GPIO_WritePin(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET);
    
    // 测试泵使能引脚
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        GPIO_PinState pin_state = (i < 2) ? PUMP1_EN_PIN : PUMP3_EN_PIN;
        GPIO_TypeDef* port = (i < 2) ? PUMP1_EN_PORT : PUMP3_EN_PORT;
        
        HAL_GPIO_WritePin(port, pin_state, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(port, pin_state, GPIO_PIN_RESET);
    }
    
    // 测试阀控制引脚
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        GPIO_PinState pin_state = (GPIO_PinState)(VALVE1_PIN << i);
        HAL_GPIO_WritePin(VALVE1_PORT, pin_state, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(VALVE1_PORT, pin_state, GPIO_PIN_RESET);
    }
    
    // 测试风扇控制引脚
    HAL_GPIO_WritePin(FAN1_PORT, FAN1_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(FAN1_PORT, FAN1_PIN, GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(FAN1_PORT, FAN2_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(FAN1_PORT, FAN2_PIN, GPIO_PIN_RESET);
    
    DEBUG_INFO("Hardware self-test completed\r\n");
    return ROTS_OK;
}

// 设置泵速度
void ROTS_Hardware_SetPumpSpeed(uint8_t pump_id, uint8_t speed)
{
    if (pump_id >= ROTS_MAX_PUMPS) return;
    
    // 限制速度范围
    if (speed > 100) speed = 100;
    
    // 计算PWM值
    uint32_t pwm_value = (speed * 1000) / 100;
    
    // 设置PWM占空比
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
            // 使用TIM3或其他定时器
            break;
    }
    
    DEBUG_DEBUG("Pump %d speed set to %d%%\r\n", pump_id, speed);
}

// 设置阀状态
void ROTS_Hardware_SetValveState(uint8_t valve_id, ROTS_ActuatorState_t state)
{
    if (valve_id >= ROTS_MAX_VALVES) return;
    
    GPIO_PinState pin_state = (state == ROTS_ACTUATOR_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState pin = (GPIO_PinState)(VALVE1_PIN << valve_id);
    
    HAL_GPIO_WritePin(VALVE1_PORT, pin, pin_state);
    
    DEBUG_DEBUG("Valve %d set to %s\r\n", valve_id, (state == ROTS_ACTUATOR_ON) ? "ON" : "OFF");
}

// 设置风扇速度
void ROTS_Hardware_SetFanSpeed(uint8_t fan_id, uint8_t speed)
{
    if (fan_id >= ROTS_MAX_FANS) return;
    
    GPIO_PinState pin_state = (speed > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState pin = (fan_id == 0) ? FAN1_PIN : FAN2_PIN;
    
    HAL_GPIO_WritePin(FAN1_PORT, pin, pin_state);
    
    DEBUG_DEBUG("Fan %d speed set to %d%%\r\n", fan_id, speed);
}

// 读取ADC值
uint16_t ROTS_Hardware_ReadADC(uint8_t channel)
{
    // 这里需要实现ADC读取
    // 暂时返回模拟值
    return 0;
}

// 读取温度
float ROTS_Hardware_ReadTemperature(void)
{
    // 这里需要实现温度传感器读取
    // 暂时返回模拟值
    return 25.0f;
}

// 读取湿度
float ROTS_Hardware_ReadHumidity(void)
{
    // 这里需要实现湿度传感器读取
    // 暂时返回模拟值
    return 50.0f;
}
