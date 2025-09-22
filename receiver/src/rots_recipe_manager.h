/**
 * @file rots_recipe_manager.h
 * @brief ROTS Recipe Manager Module Header
 * @author ROTS Team
 * @date 2024
 * 
 * Header file for recipe management
 */

#ifndef ROTS_RECIPE_MANAGER_H
#define ROTS_RECIPE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "rots_receiver.h"

/* Recipe structure */
typedef struct {
    ROTS_OdorType_t odor_type;
    char name[16];
    uint8_t pump_ratios[ROTS_MAX_PUMPS];  // Pump ratios (0-100%)
    bool valve_states[ROTS_MAX_VALVES];   // Valve states
    uint16_t mixing_time;                 // Mixing time in ms
    uint8_t fan_speed;                    // Fan speed (0-100%)
} ROTS_Recipe_t;

/* Function Prototypes */
ROTS_StatusTypeDef ROTS_RecipeManager_Init(void);
ROTS_StatusTypeDef ROTS_RecipeManager_GetRecipe(ROTS_OdorType_t odor_type, ROTS_Recipe_t* recipe);
ROTS_StatusTypeDef ROTS_RecipeManager_AddCustomRecipe(ROTS_Recipe_t* recipe);
ROTS_StatusTypeDef ROTS_RecipeManager_UpdateRecipe(ROTS_Recipe_t* recipe);
ROTS_StatusTypeDef ROTS_RecipeManager_DeleteRecipe(ROTS_OdorType_t odor_type);
ROTS_StatusTypeDef ROTS_RecipeManager_GetAllRecipes(ROTS_Recipe_t* recipes, uint8_t max_count, uint8_t* actual_count);

#ifdef __cplusplus
}
#endif

#endif /* ROTS_RECIPE_MANAGER_H */
