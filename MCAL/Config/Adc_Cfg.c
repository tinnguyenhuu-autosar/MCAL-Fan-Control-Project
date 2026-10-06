/*******************************************************************************
 * @file    Adc_Cfg.c
 * @brief   File nguồn cấu hình ADC driver.
 *
 * @details Cấu hình group ADC và các hàm
 *          xử lý ngắt
 ******************************************************************************/

#include "Adc_Cfg.h"

/**********************************************************
 * Khai báo cấu hình kênh cho từng group
 **********************************************************/
static const Adc_ChannelConfigType Adc_Group0Channels[] = {
    {
        .ChannelId = ADC_Channel_0,
        .Rank = 1U,
        .SamplingTime = ADC_SampleTime_55Cycles5,
    },
};

static const Adc_ChannelConfigType Adc_Group1Channels[] = {
    {
        .ChannelId = ADC_Channel_0,
        .Rank = 1U,
        .SamplingTime = ADC_SampleTime_55Cycles5,
    },
    {
        .ChannelId = ADC_Channel_1,
        .Rank = 2U,
        .SamplingTime = ADC_SampleTime_55Cycles5,
    },
};

/*******************************************************************************
 * Dữ liệu cấu hình
 ******************************************************************************/
Adc_ValueGroupType dataGroup[ADC_MAX_GROUP]; /* Buffer lưu kết quả ADC. */

const Adc_GroupConfigType AdcGroupConfigList[ADC_NUM_GROUP_CONFIG] =
    {
        /* Group 0: ADC1, trigger phần mềm, chuyển đổi liên tục, dùng ngắt kèm DMA. */
        {
            .GroupId = ADC_GROUP_1,
            .AdcInstance = ADC1,
            .ChannelList = Adc_Group0Channels,
            .NumChannels = (uint8)(sizeof(Adc_Group0Channels) / sizeof(Adc_Group0Channels[0])),
            .ConversionMode = ADC_CONV_MODE_ONESHOT,
            .TriggerSource = ADC_TRIGG_SRC_SW,
            .HwTriggerSource = ADC_ExternalTrigConv_None,
            .AccessMode = ADC_ACCESS_MODE_SINGLE,
            .StreamBufferMode = ADC_STREAM_BUFFER_LINEAR,
            .StreamNumSamples = 1U,
            .DmaConfig = NULL_PTR,
            .DmaHalfCb = NULL_PTR,
            .DmaCompleteCb = NULL_PTR,
        },
};

/**********************************************************
 * Cấu hình tổng ADC Driver
 **********************************************************/
const Adc_ConfigType AdcDriverConfig = {
    .Groups = AdcGroupConfigList,
    .NumGroups = ADC_NUM_GROUP_CONFIG,
};

/*******************************************************************************
 * Hàm xử lý ngắt
 ******************************************************************************/
// void ADC1_2_IRQHandler(void)
// {
//     /* Chuyển xử lý ngắt ADC1 và ADC2 cho ADC driver. */
//     Adc_IsrHandler(ADC1);
//     Adc_IsrHandler(ADC2);
// }

void DMA1_Channel1_IRQHandler(void)
{
    /* Chuyển xử lý ngắt DMA1 channel 1 cho ADC driver. */
    Adc_DmaIsrHandler(DMA1_Channel1);
}
