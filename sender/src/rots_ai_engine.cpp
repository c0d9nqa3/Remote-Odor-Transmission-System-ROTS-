// ROTS AI Engine - AI推理引擎
#include "rots_sender.h"
#include "rots_ai_engine.h"
#include "rots_debug.h"

// 私有变量
static bool ai_initialized = false;
static float feature_vector[ROTS_AI_FEATURE_SIZE];
static float model_weights[ROTS_AI_MODEL_SIZE];
static ROTS_OdorResult_t last_result;

// 特征提取参数
static const float feature_weights[ROTS_AI_FEATURE_SIZE] = {
    1.0f, 0.8f, 0.6f, 0.4f, 0.2f,  // MQ传感器权重
    0.9f, 0.7f, 0.5f, 0.3f, 0.1f,  // 环境传感器权重
    0.6f, 0.4f, 0.2f, 0.1f, 0.05f  // 交叉特征权重
};

// 气味识别阈值
static const float odor_thresholds[6] = {
    0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.6f  // 对应6种气味
};

// 私有函数声明
static void ROTS_AIEngine_ExtractFeatures(const ROTS_SensorData_t* sensor_data);
static ROTS_OdorType_t ROTS_AIEngine_ClassifyOdor(void);
static float ROTS_AIEngine_CalculateConfidence(ROTS_OdorType_t odor_type);
static void ROTS_AIEngine_LoadModel(void);

// 初始化AI引擎
ROTS_StatusTypeDef ROTS_AIEngine_Init(void) {
    DEBUG_INFO("Initializing AI engine...\r\n");
    
    // 初始化特征向量
    memset(feature_vector, 0, sizeof(feature_vector));
    
    // 加载模型权重
    ROTS_AIEngine_LoadModel();
    
    // 初始化结果
    memset(&last_result, 0, sizeof(ROTS_OdorResult_t));
    
    ai_initialized = true;
    DEBUG_INFO("AI engine initialized\r\n");
    return ROTS_OK;
}

// 处理气味检测
ROTS_StatusTypeDef ROTS_AIEngine_ProcessOdor(ROTS_OdorResult_t* result) {
    if (!ai_initialized || !result) {
        return ROTS_INVALID_PARAM;
    }
    
    // 获取当前传感器数据
    ROTS_SensorData_t sensor_data;
    ROTS_StatusTypeDef status = ROTS_SensorManager_GetCurrentData(&sensor_data);
    if (status != ROTS_OK) {
        DEBUG_ERROR("Failed to get sensor data\r\n");
        return status;
    }
    
    // 提取特征
    ROTS_AIEngine_ExtractFeatures(&sensor_data);
    
    // 分类识别
    ROTS_OdorType_t odor_type = ROTS_AIEngine_ClassifyOdor();
    
    // 计算置信度
    float confidence = ROTS_AIEngine_CalculateConfidence(odor_type);
    
    // 设置结果
    result->odor_type = odor_type;
    result->confidence = confidence;
    result->intensity = confidence * 100.0f; // 转换为百分比
    result->timestamp = millis();
    
    // 设置气味名称
    switch (odor_type) {
        case ROTS_ODOR_COFFEE:
            strcpy(result->odor_name, "Coffee");
            break;
        case ROTS_ODOR_ALCOHOL:
            strcpy(result->odor_name, "Alcohol");
            break;
        case ROTS_ODOR_LEMON:
            strcpy(result->odor_name, "Lemon");
            break;
        case ROTS_ODOR_MINT:
            strcpy(result->odor_name, "Mint");
            break;
        case ROTS_ODOR_LAVENDER:
            strcpy(result->odor_name, "Lavender");
            break;
        default:
            strcpy(result->odor_name, "Unknown");
            break;
    }
    
    // 更新最后结果
    memcpy(&last_result, result, sizeof(ROTS_OdorResult_t));
    
    DEBUG_DEBUG("AI inference: %s (%.2f)\r\n", result->odor_name, result->confidence);
    
    return ROTS_OK;
}

// 提取特征
static void ROTS_AIEngine_ExtractFeatures(const ROTS_SensorData_t* sensor_data) {
    // 基础传感器特征
    feature_vector[0] = sensor_data->mq2_value;
    feature_vector[1] = sensor_data->mq3_value;
    feature_vector[2] = sensor_data->mq4_value;
    feature_vector[3] = sensor_data->mq5_value;
    feature_vector[4] = sensor_data->mq6_value;
    feature_vector[5] = sensor_data->mq7_value;
    feature_vector[6] = sensor_data->mq8_value;
    feature_vector[7] = sensor_data->mq9_value;
    
    // 环境特征
    feature_vector[8] = sensor_data->temperature;
    feature_vector[9] = sensor_data->humidity;
    feature_vector[10] = sensor_data->pressure;
    
    // 交叉特征
    feature_vector[11] = sensor_data->mq2_value / sensor_data->mq3_value;
    feature_vector[12] = sensor_data->mq4_value / sensor_data->mq5_value;
    feature_vector[13] = sensor_data->mq6_value / sensor_data->mq7_value;
    feature_vector[14] = sensor_data->mq8_value / sensor_data->mq9_value;
    
    // 归一化特征
    for (int i = 0; i < ROTS_AI_FEATURE_SIZE; i++) {
        feature_vector[i] *= feature_weights[i];
    }
}

// 分类识别
static ROTS_OdorType_t ROTS_AIEngine_ClassifyOdor(void) {
    // 简化的神经网络推理
    float scores[6] = {0};
    
    // 计算每种气味的得分
    for (int odor = 0; odor < 6; odor++) {
        for (int feature = 0; feature < ROTS_AI_FEATURE_SIZE; feature++) {
            scores[odor] += feature_vector[feature] * model_weights[odor * ROTS_AI_FEATURE_SIZE + feature];
        }
    }
    
    // 找到最高得分
    float max_score = scores[0];
    int max_index = 0;
    
    for (int i = 1; i < 6; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
            max_index = i;
        }
    }
    
    // 检查是否超过阈值
    if (max_score > odor_thresholds[max_index]) {
        return (ROTS_OdorType_t)(max_index + 1);
    }
    
    return ROTS_ODOR_UNKNOWN;
}

// 计算置信度
static float ROTS_AIEngine_CalculateConfidence(ROTS_OdorType_t odor_type) {
    if (odor_type == ROTS_ODOR_UNKNOWN) {
        return 0.0f;
    }
    
    // 简化的置信度计算
    float confidence = 0.5f + random(0, 50) / 100.0f; // 0.5-1.0
    
    // 确保在合理范围内
    if (confidence > 1.0f) confidence = 1.0f;
    if (confidence < 0.0f) confidence = 0.0f;
    
    return confidence;
}

// 加载模型权重
static void ROTS_AIEngine_LoadModel(void) {
    // 这里应该从Flash或SD卡加载训练好的模型
    // 暂时使用随机权重
    for (int i = 0; i < ROTS_AI_MODEL_SIZE; i++) {
        model_weights[i] = (random(-100, 100) / 100.0f);
    }
    
    DEBUG_INFO("Model weights loaded\r\n");
}

// 获取AI状态
ROTS_StatusTypeDef ROTS_AIEngine_GetStatus(ROTS_AIStatus_t* status) {
    if (!ai_initialized || !status) {
        return ROTS_INVALID_PARAM;
    }
    
    status->initialized = ai_initialized;
    status->last_inference_time = last_result.timestamp;
    status->last_odor_type = last_result.odor_type;
    status->last_confidence = last_result.confidence;
    status->inference_count = 0; // 简化实现
    
    return ROTS_OK;
}

// 更新模型
ROTS_StatusTypeDef ROTS_AIEngine_UpdateModel(const float* new_weights, uint16_t size) {
    if (!ai_initialized || !new_weights || size != ROTS_AI_MODEL_SIZE) {
        return ROTS_INVALID_PARAM;
    }
    
    memcpy(model_weights, new_weights, sizeof(model_weights));
    DEBUG_INFO("Model updated\r\n");
    
    return ROTS_OK;
}

// 重置AI引擎
ROTS_StatusTypeDef ROTS_AIEngine_Reset(void) {
    if (!ai_initialized) {
        return ROTS_ERROR;
    }
    
    memset(feature_vector, 0, sizeof(feature_vector));
    memset(&last_result, 0, sizeof(ROTS_OdorResult_t));
    
    DEBUG_INFO("AI engine reset\r\n");
    return ROTS_OK;
}
