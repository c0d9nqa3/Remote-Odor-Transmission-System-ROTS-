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
    UART_HandleTypeDef huart_esp8266;
    char at_cmd[100];
    char response[200];
    uint32_t timeout;
    
    // Initialize UART for ESP8266
    huart_esp8266.Instance = USART2;
    huart_esp8266.Init.BaudRate = 115200;
    huart_esp8266.Init.WordLength = UART_WORDLENGTH_8B;
    huart_esp8266.Init.StopBits = UART_STOPBITS_1;
    huart_esp8266.Init.Parity = UART_PARITY_NONE;
    huart_esp8266.Init.Mode = UART_MODE_TX_RX;
    huart_esp8266.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart_esp8266.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart_esp8266) != HAL_OK) {
        return ROTS_COMM_ERROR;
    }
    
    // Reset ESP8266
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Reset pin
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(2000);
    
    // Test AT command
    strcpy(at_cmd, "AT\r\n");
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)at_cmd, strlen(at_cmd), 1000);
    HAL_Delay(1000);
    
    // Set WiFi mode
    strcpy(at_cmd, "AT+CWMODE=1\r\n");
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)at_cmd, strlen(at_cmd), 1000);
    HAL_Delay(1000);
    
    // Connect to WiFi
    sprintf(at_cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ROTS_WIFI_SSID, ROTS_WIFI_PASSWORD);
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)at_cmd, strlen(at_cmd), 1000);
    HAL_Delay(5000);
    
    // Check connection status
    strcpy(at_cmd, "AT+CIPSTATUS\r\n");
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)at_cmd, strlen(at_cmd), 1000);
    HAL_Delay(1000);
    
    // Enable multiple connections
    strcpy(at_cmd, "AT+CIPMUX=1\r\n");
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)at_cmd, strlen(at_cmd), 1000);
    HAL_Delay(1000);
    
    wifi_connected = true;
    return ROTS_OK;
}

ROTS_StatusTypeDef ROTS_Communication_ConnectMQTT(void)
{
    UART_HandleTypeDef huart_esp8266;
    char mqtt_cmd[200];
    char response[200];
    
    // Initialize UART for ESP8266
    huart_esp8266.Instance = USART2;
    huart_esp8266.Init.BaudRate = 115200;
    huart_esp8266.Init.WordLength = UART_WORDLENGTH_8B;
    huart_esp8266.Init.StopBits = UART_STOPBITS_1;
    huart_esp8266.Init.Parity = UART_PARITY_NONE;
    huart_esp8266.Init.Mode = UART_MODE_TX_RX;
    huart_esp8266.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart_esp8266.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart_esp8266) != HAL_OK) {
        return ROTS_COMM_ERROR;
    }
    
    // Connect to MQTT broker
    sprintf(mqtt_cmd, "AT+CIPSTART=0,\"TCP\",\"%s\",%d\r\n", ROTS_MQTT_BROKER_HOST, ROTS_MQTT_BROKER_PORT);
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)mqtt_cmd, strlen(mqtt_cmd), 1000);
    HAL_Delay(2000);
    
    // Send MQTT CONNECT packet
    uint8_t mqtt_connect[] = {
        0x10, 0x1A, 0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04, 0x02, 0x00, 0x3C, 0x00, 0x0F,
        'R', 'O', 'T', 'S', '_', 'R', 'E', 'C', 'E', 'I', 'V', 'E', 'R', '_', '0', '0', '1'
    };
    
    sprintf(mqtt_cmd, "AT+CIPSEND=0,%d\r\n", sizeof(mqtt_connect));
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)mqtt_cmd, strlen(mqtt_cmd), 1000);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart_esp8266, mqtt_connect, sizeof(mqtt_connect), 1000);
    HAL_Delay(1000);
    
    // Subscribe to command topic
    uint8_t mqtt_subscribe[] = {
        0x82, 0x1A, 0x00, 0x01, 0x00, 0x16, 'r', 'o', 't', 's', '/', 'c', 'o', 'm', 'm', 'a', 'n', 'd', '/', '0', '0', '1', 0x00
    };
    
    sprintf(mqtt_cmd, "AT+CIPSEND=0,%d\r\n", sizeof(mqtt_subscribe));
    HAL_UART_Transmit(&huart_esp8266, (uint8_t*)mqtt_cmd, strlen(mqtt_cmd), 1000);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart_esp8266, mqtt_subscribe, sizeof(mqtt_subscribe), 1000);
    HAL_Delay(1000);
    
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
