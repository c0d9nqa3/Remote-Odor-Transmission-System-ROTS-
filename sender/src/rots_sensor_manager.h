// ROTS Sensor Manager Header
#ifndef ROTS_SENSOR_MANAGER_H
#define ROTS_SENSOR_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rots_sender.h"

// 传感器状态结构
typedef struct {
    bool initialized;
    uint32_t last_read_time;
    float temperature;
    float humidity;
    float pressure;
    uint8_t sensor_health; // 0-100%
} ROTS_SensorStatus_t;

// 函数声明
ROTS_StatusTypeDef ROTS_SensorManager_Init(void);
ROTS_StatusTypeDef ROTS_SensorManager_ReadSensors(ROTS_SensorData_t* data);
void ROTS_SensorManager_UpdateData(const ROTS_SensorData_t* data);
ROTS_StatusTypeDef ROTS_SensorManager_GetCurrentData(ROTS_SensorData_t* data);
ROTS_StatusTypeDef ROTS_SensorManager_GetHistoryData(ROTS_SensorData_t* data, uint8_t count);
ROTS_StatusTypeDef ROTS_SensorManager_CalibrateSensors(void);
ROTS_StatusTypeDef ROTS_SensorManager_GetStatus(ROTS_SensorStatus_t* status);

// 传感器读取函数
float ROTS_SensorManager_ReadTemperature(void);
float ROTS_SensorManager_ReadHumidity(void);
float ROTS_SensorManager_ReadPressure(void);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_SENSOR_MANAGER_H */
