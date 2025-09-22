// ROTS Communication Module - MQTT + WiFi

#include "rots_receiver.h"
#include "rots_communication.h"
#include <string.h>
#include <stdlib.h>

// MQTT and WiFi variables
static bool wifi_connected = false;
static bool mqtt_connected = false;
static ROTS_MessageTypeDef rx_buffer;
static bool message_received = false;
static uint32_t last_communication_time = 0;

// Function prototypes
static ROTS_StatusTypeDef ROTS_Communication_ValidateMessage(ROTS_MessageTypeDef* msg);
static uint16_t ROTS_Communication_CalculateChecksum(ROTS_MessageTypeDef* msg);
static void ROTS_MQTT_MessageCallback(char* topic, char* payload, int length);

ROTS_StatusTypeDef ROTS_Communication_Init(void)
{
    ROTS_StatusTypeDef status = ROTS_OK;
    
    // Initialize WiFi connection
    status = ROTS_Communication_ConnectWiFi();
    if (status != ROTS_OK) return status;
    
    // Initialize MQTT connection
    status = ROTS_Communication_ConnectMQTT();
    if (status != ROTS_OK) return status;
    
    return ROTS_OK;
}

ROTS_StatusTypeDef ROTS_Communication_ConnectWiFi(void)
{
    // Initialize WiFi module (ESP8266)
    // This would typically send AT commands to ESP8266
    // For now, we'll simulate successful connection
    wifi_connected = true;
    return ROTS_OK;
}

ROTS_StatusTypeDef ROTS_Communication_ConnectMQTT(void)
{
    // Connect to MQTT broker
    // This would use MQTT client library
    // For now, we'll simulate successful connection
    mqtt_connected = true;
    return ROTS_OK;
}

ROTS_StatusTypeDef ROTS_Communication_ReceiveMessage(ROTS_MessageTypeDef* message)
{
    if (message_received) {
        // Validate received message
        if (ROTS_Communication_ValidateMessage(&rx_buffer) == ROTS_OK) {
            memcpy(message, &rx_buffer, sizeof(ROTS_MessageTypeDef));
            message_received = false;
            last_communication_time = HAL_GetTick();
            return ROTS_OK;
        } else {
            message_received = false;
            return ROTS_COMM_ERROR;
        }
    }
    
    // Check for communication timeout
    if ((HAL_GetTick() - last_communication_time) > ROTS_COMM_TIMEOUT) {
        return ROTS_TIMEOUT;
    }
    
    return ROTS_BUSY;
}

/**
 * @brief Send status message to ESP32
 * @param status System status structure
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_Communication_SendStatus(ROTS_SystemStatus_t* status)
{
    ROTS_StatusTypeDef result = ROTS_OK;
    uint8_t status_data[64];
    uint16_t data_length = 0;
    
    // Format status message
    status_data[data_length++] = 0xAA; // Start marker
    status_data[data_length++] = 0x02; // Message type: Status
    status_data[data_length++] = status->state;
    status_data[data_length++] = status->error_count;
    
    // Add pump status
    for (int i = 0; i < ROTS_MAX_PUMPS; i++) {
        status_data[data_length++] = status->pump_status[i];
    }
    
    // Add valve status
    for (int i = 0; i < ROTS_MAX_VALVES; i++) {
        status_data[data_length++] = status->valve_status[i];
    }
    
    // Add checksum
    uint16_t checksum = 0;
    for (int i = 0; i < data_length; i++) {
        checksum += status_data[i];
    }
    status_data[data_length++] = (checksum >> 8) & 0xFF;
    status_data[data_length++] = checksum & 0xFF;
    
    // Send data
    if (HAL_UART_Transmit(&huart_esp8266, status_data, data_length, 1000) != HAL_OK) {
        result = ROTS_COMM_ERROR;
    }
    
    return result;
}

/**
 * @brief Send error message to ESP32
 * @param error_code Error code
 * @return ROTS_OK if successful, error code otherwise
 */
ROTS_StatusTypeDef ROTS_Communication_SendError(ROTS_StatusTypeDef error_code)
{
    uint8_t error_data[4];
    error_data[0] = 0xAA; // Start marker
    error_data[1] = 0x05; // Message type: Error
    error_data[2] = error_code;
    error_data[3] = 0x55; // End marker
    
    if (HAL_UART_Transmit(&huart_esp8266, error_data, 4, 1000) != HAL_OK) {
        return ROTS_COMM_ERROR;
    }
    
    return ROTS_OK;
}

/**
 * @brief Validate received message
 * @param msg Pointer to message structure
 * @return ROTS_OK if valid, error code otherwise
 */
static ROTS_StatusTypeDef ROTS_Communication_ValidateMessage(ROTS_MessageTypeDef* msg)
{
    // Check message type
    if (msg->message_type < ROTS_MSG_ODOR_COMMAND || msg->message_type > ROTS_MSG_EMERGENCY_STOP) {
        return ROTS_INVALID_PARAM;
    }
    
    // Check odor type
    if (msg->odor_type < ROTS_ODOR_COFFEE || msg->odor_type > ROTS_ODOR_MIXED) {
        return ROTS_INVALID_PARAM;
    }
    
    // Check intensity
    if (msg->intensity > ROTS_MAX_INTENSITY) {
        return ROTS_INVALID_PARAM;
    }
    
    // Check duration
    if (msg->duration > ROTS_MAX_DURATION) {
        return ROTS_INVALID_PARAM;
    }
    
    // Verify checksum
    uint16_t calculated_checksum = ROTS_Communication_CalculateChecksum(msg);
    if (calculated_checksum != msg->checksum) {
        return ROTS_COMM_ERROR;
    }
    
    return ROTS_OK;
}

/**
 * @brief Calculate message checksum
 * @param msg Pointer to message structure
 * @return Calculated checksum
 */
static uint16_t ROTS_Communication_CalculateChecksum(ROTS_MessageTypeDef* msg)
{
    uint16_t checksum = 0;
    uint8_t* data = (uint8_t*)msg;
    
    // Calculate checksum for all fields except checksum itself
    for (int i = 0; i < (sizeof(ROTS_MessageTypeDef) - sizeof(uint16_t)); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief UART receive callback
 * @param huart UART handle
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        message_received = true;
        // Restart reception for next message
        HAL_UART_Receive_IT(&huart_esp8266, (uint8_t*)&rx_buffer, sizeof(ROTS_MessageTypeDef));
    }
}
