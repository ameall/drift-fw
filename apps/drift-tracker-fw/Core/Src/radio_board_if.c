/* radio_board_if.c - STM32WL5MOCH6TR Implementation */

#include "radio_board_if.h"
#include "stm32wlxx_hal.h"

/* Define fixed power level (Choose either RBI_CONF_RFO_HP or RBI_CONF_RFO_LP) */
#define FIXED_POWER_MODE  RBI_CONF_RFO_HP  // High Power Mode (+22 dBm)

/* Initialize the radio board interface */
int32_t RBI_Init(void)
{
    /* Enable SMPS mode for power efficiency */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    HAL_PWREx_SMPS_SetMode(PWR_SMPS_BYPASS);


    return 0;
}

/* Deinitialize the radio board interface */
int32_t RBI_DeInit(void)
{
    return 0;
}

/* Configure RF Switch (Not needed since output power is fixed) */
int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
    return 0;  // No RF switch needed
}

/* Get the configured TX power mode */
int32_t RBI_GetTxConfig(void)
{
    return FIXED_POWER_MODE; // Return fixed power configuration
}

/* Check if the board uses a TCXO */
int32_t RBI_IsTCXO(void)
{
    return 1; // Integrated 32 MHz TCXO is used
}

/* Check if the board uses SMPS */
int32_t RBI_IsDCDC(void)
{
    return 1; // SMPS mode is enabled
}

/* Get max power configuration */
int32_t RBI_GetRFOMaxPowerConfig(RBI_RFOMaxPowerConfig_TypeDef Config)
{
    if (Config == RBI_RFO_LP_MAXPOWER)
    {
        return 15; // Low power mode max (+15 dBm)
    }
    else
    {
        return 22; // High power mode max (+22 dBm)
    }
}
