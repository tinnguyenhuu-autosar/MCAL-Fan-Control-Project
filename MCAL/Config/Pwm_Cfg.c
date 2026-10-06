/**********************************************************
 * @file    Pwm_Lcfg.c
 * @brief   PWM Driver Configuration Source File (AUTOSAR)
 * @details Cấu hình các kênh PWM dùng cho STM32F103
 **********************************************************/

#include "Pwm_Cfg.h"
#include "Pwm_Lcfg.h"

/* ==== Cấu hình từng kênh PWM ==== */
const Pwm_ChannelConfigType PwmChannelsConfig[] = {
    /* Channel 0: PA0 - TIM2_CH1 */
    {
        .TIMx = TIM2,
        .channel = 1,
        .classType = PWM_VARIABLE_PERIOD,
        .defaultPeriod = 999,       // 1ms (72MHz/72/1000)
        .defaultDutyCycle = 0x0000, // Duty 0%
        .polarity = PWM_HIGH,
        .idleState = PWM_LOW,
    },

};

/* ==== Cấu hình tổng PWM driver ==== */
const Pwm_ConfigType Pwm_Config = {
    .Channels = PwmChannelsConfig,
    .NumChannels = sizeof(PwmChannelsConfig) / sizeof(Pwm_ChannelConfigType)};
