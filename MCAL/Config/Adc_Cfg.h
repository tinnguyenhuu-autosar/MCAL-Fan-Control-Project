/*******************************************************************************
 * @file    Adc_Cfg.h
 * @brief   File header cấu hình ADC driver.
 *
 * @details Sử dụng các macro để cấu hình trong ADC
 *
 ******************************************************************************/

#ifndef ADC_CONFIG_H
#define ADC_CONFIG_H

/*******************************************************************************
 * Khai báo include
 ******************************************************************************/
#include "Std_Types.h"
#include "Adc_Types.h"

#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

/*******************************************************************************
 * Thông tin module
 ******************************************************************************/
#define ADC_VENDOR_ID (0x181U)  /* ID vendor của module ADC. */
#define ADC_MODULE_ID (0x01U)   /* ID module ADC. */
#define ADC_INSTANCE_ID (0x00U) /* ID instance của module ADC. */

#define ADC_SW_MAJOR_VERSION (1U) /* Version major của phần mềm. */
#define ADC_SW_MINOR_VERSION (0U) /* Version minor của phần mềm. */
#define ADC_SW_PATCH_VERSION (0U) /* Version patch của phần mềm. */

/*******************************************************************************
 * group && channel ADC Info
 ******************************************************************************/
#define ADC_GROUP_1 (0U) /* Group ADC1. */
#define ADC_GROUP_2 (1U) /* Group ADC2. */

#define ADC_MAX_GROUP (2U)        /* Số group ADC tối đa. */
#define ADC_MAX_CHANNEL (8U)      /* Số channel ADC tối đa trong group. */
#define ADC_NUM_GROUP_CONFIG (1U) /* Số group ADC được cấu hình. */

/*******************************************************************************
 * Trạng thái nguồn ADC
 ******************************************************************************/
#define ADC_FULL_POWER_STATE ((Adc_PowerStateType)0x00U) /* Trạng thái full power. */
#define ADC_LOW_POWER_STATE ((Adc_PowerStateType)0x01U)  /* Trạng thái low power. */

/*******************************************************************************
 * Macro ánh xạ group ADC sang peripheral ADC
 ******************************************************************************/
#define GET_GROUP_ADC(groupID) \
    (((groupID) == ADC_GROUP_1) ? (ADC1) : (((groupID) == ADC_GROUP_2) ? (ADC2) : (NULL_PTR)))

/*******************************************************************************
 * Hằng cấu hình channel ADC
 ******************************************************************************/
enum
{
    ADC_CHANNEL_0 = 0U,   /* Kênh ADC0, ví dụ PA0. */
    ADC_CHANNEL_1 = 1U,   /* Kênh ADC1, ví dụ PA1. */
    ADC_CHANNEL_2 = 2U,   /* Kênh ADC2, ví dụ PA2. */
    ADC_CHANNEL_3 = 3U,   /* Kênh ADC3, ví dụ PA3. */
    ADC_CHANNEL_4 = 4U,   /* Kênh ADC4, ví dụ PA4. */
    ADC_CHANNEL_5 = 5U,   /* Kênh ADC5, ví dụ PA5. */
    ADC_CHANNEL_6 = 6U,   /* Kênh ADC6, ví dụ PA6. */
    ADC_CHANNEL_7 = 7U,   /* Kênh ADC7, ví dụ PA7. */
    ADC_CHANNEL_8 = 8U,   /* Kênh ADC8, ví dụ PB0. */
    ADC_CHANNEL_9 = 9U,   /* Kênh ADC9, ví dụ PB1. */
    ADC_CHANNEL_10 = 10U, /* Kênh ADC10, ví dụ PC0. */
    ADC_CHANNEL_11 = 11U, /* Kênh ADC11, ví dụ PC1. */
    ADC_CHANNEL_12 = 12U, /* Kênh ADC12, ví dụ PC2. */
    ADC_CHANNEL_13 = 13U, /* Kênh ADC13, ví dụ PC3. */
    ADC_CHANNEL_14 = 14U, /* Kênh ADC14, ví dụ PC4. */
    ADC_CHANNEL_15 = 15U, /* Kênh ADC15, ví dụ PC5. */
    ADC_CHANNEL_16 = 16U, /* Kênh ADC16, cảm biến nhiệt độ chip. */
    ADC_CHANNEL_17 = 17U  /* Kênh ADC17, điện áp tham chiếu Vref. */
};

/*******************************************************************************
 * Khai báo dữ liệu cấu hình
 ******************************************************************************/
extern Adc_ValueGroupType dataGroup[ADC_MAX_GROUP];                        /* Buffer lưu kết quả ADC. */
extern const Adc_GroupConfigType AdcGroupConfigList[ADC_NUM_GROUP_CONFIG]; /* Danh sách cấu hình group ADC. */
extern const Adc_ConfigType AdcDriverConfig;                               /* Cấu hình tổng của ADC driver. */

/*******************************************************************************
 * Khai báo hàm xử lý ngắt nội bộ
 ******************************************************************************/
void Adc_IsrHandler(ADC_TypeDef *AdcInstance);
void Adc_DmaIsrHandler(DMA_Channel_TypeDef *DmaChannel);

#endif /* ADC_CONFIG_H */
