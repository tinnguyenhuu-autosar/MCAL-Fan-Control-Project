/**********************************************************
 * @file    Adc.c
 * @brief   ADC Driver Source File
 * @details Hiện thực API ADC chuẩn AUTOSAR cho STM32F103 (SPL).
 *          Driver chỉ cấu hình ngoại vi ADC/DMA/IRQ; không cấu hình GPIO.
 *          GPIO analog cần được cấu hình bởi Port Driver trước khi dùng ADC.
 **********************************************************/

#include "Adc.h"

#include "stm32f10x_rcc.h"
#include "misc.h"
#include <stddef.h>

/**********************************************************
 * @brief   Runtime data cho từng ADC group
 * @details Cấu trúc này lưu state nội bộ của mỗi group trong quá trình chạy:
 *          buffer kết quả, trạng thái BUSY/IDLE, số mẫu hợp lệ và cờ trigger/notification.
 **********************************************************/
typedef struct
{
    Adc_ValueGroupType *ResultBufferPtr;
    Adc_ValueGroupType LastValues[ADC_MAX_CHANNEL];
    Adc_StreamNumSampleType ValidSamples;
    Adc_StatusType Status;
    uint8 NotificationEnabled;
    uint8 HwTriggerEnabled;
} Adc_GroupRuntimeType;

/**********************************************************
 * @brief   Biến static của driver
 * @details Gom các biến trạng thái toàn cục của module ADC:
 *          con trỏ config hiện tại, trạng thái init, runtime của từng group,
 *          và thông tin power-state.
 **********************************************************/
static const Adc_ConfigType *Adc_CurrentConfigPtr = NULL_PTR;
static Adc_GroupRuntimeType Adc_RuntimeData[ADC_MAX_GROUP];
static uint8 Adc_IsInitialized = 0U;

static Adc_PowerStateType Adc_CurrentPowerState = ADC_FULL_POWER;
static Adc_PowerStateType Adc_TargetPowerState = ADC_FULL_POWER;
static uint8 Adc_IsPowerStatePrepared = 0U;

/**********************************************************
 * @brief   Lấy cấu hình group theo ID logic
 * @details Hàm xác thực cả con trỏ config tổng và biên `Group` trước khi trả cấu hình.
 *          Đây là gate-check dùng lại ở hầu hết public API để giảm lặp mã kiểm tra.
 * @param[in] Group ID group logic
 * @return  Con trỏ tới cấu hình group nếu hợp lệ, ngược lại trả `NULL_PTR`
 **********************************************************/
static const Adc_GroupConfigType *Adc_GetGroupConfig(Adc_GroupType Group)
{
    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return NULL_PTR;
    }

    if (Group >= Adc_CurrentConfigPtr->NumGroups)
    {
        return NULL_PTR;
    }

    return &Adc_CurrentConfigPtr->Groups[Group];
}

/**********************************************************
 * @brief   Khởi tạo ADC Driver với bộ cấu hình được chọn
 * @details API này thiết lập baseline cho từng ADC unit:
 *          bật clock ngoại vi, cấu hình ADCCLK, init mặc định, disable trigger/IRQ/DMA,
 *          enable ADC và chạy calibration an toàn.
 *          Hàm không cấu hình GPIO analog; chân phải được Port Driver setup trước.
 * @param[in] ConfigPtr Con trỏ cấu hình tổng thể ADC (mảng group và thông số liên quan)
 **********************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    uint8 i;
    uint8 adc1Initialized;
    uint8 adc2Initialized;

    if (Adc_IsInitialized != 0U)
    {
        return;
    }

    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->Groups == NULL_PTR) ||
        (ConfigPtr->NumGroups == 0U) || (ConfigPtr->NumGroups > ADC_MAX_GROUP))
    {
        return;
    }

    Adc_CurrentConfigPtr = ConfigPtr;

    /* F1: ADCCLK tối đa 14MHz, thường dùng PCLK2/6 = 12MHz */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    adc1Initialized = 0U;
    adc2Initialized = 0U;

    for (i = 0U; i < ConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &ConfigPtr->Groups[i];

        if (groupCfg->AdcInstance == ADC1)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        }
        else if (groupCfg->AdcInstance == ADC2)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
        }
        else
        {
            continue;
        }

        if ((groupCfg->AdcInstance == ADC1) && (adc1Initialized == 0U))
        {
            adc1Initialized = 1U;
        }
        else if ((groupCfg->AdcInstance == ADC2) && (adc2Initialized == 0U))
        {
            adc2Initialized = 1U;
        }
        else
        {
            continue;
        }

        /* Init baseline trên mỗi ADC unit, cấu hình group cụ thể sẽ set lại khi Start/EnableHwTrigger */
        {
            ADC_InitTypeDef adcInit;
            adcInit.ADC_Mode = ADC_Mode_Independent;
            adcInit.ADC_ScanConvMode = DISABLE;
            adcInit.ADC_ContinuousConvMode = DISABLE;
            adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
            adcInit.ADC_DataAlign = ADC_DataAlign_Right;
            adcInit.ADC_NbrOfChannel = 1U;
            ADC_Init(groupCfg->AdcInstance, &adcInit);
        }

        ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, DISABLE);
        ADC_DMACmd(groupCfg->AdcInstance, DISABLE);
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
        ADC_Cmd(groupCfg->AdcInstance, ENABLE);
    }

    for (i = 0U; i < ConfigPtr->NumGroups; i++)
    {
        Adc_RuntimeData[i].Status = ADC_IDLE;
        Adc_RuntimeData[i].HwTriggerEnabled = (ConfigPtr->Groups[i].TriggerSource == ADC_TRIGG_SRC_SW) ? 1U : 0U;
    }

    Adc_CurrentPowerState = ADC_FULL_POWER;
    Adc_TargetPowerState = ADC_FULL_POWER;
    Adc_IsPowerStatePrepared = 0U;
    Adc_IsInitialized = 1U;
}

/**********************************************************
 * @brief   Gán vùng nhớ buffer kết quả cho một ADC group
 * @details Driver lưu con trỏ buffer tại runtime để phục vụ cơ chế đọc kết quả
 *          bằng polling hoặc DMA/ISR callback. API này phải được gọi trước khi start group.
 * @param[in] Group         ID group logic cần gán buffer
 * @param[in] DataBufferPtr Con trỏ vùng dữ liệu nhận mẫu ADC
 * @return  `E_OK` nếu hợp lệ, `E_NOT_OK` nếu module chưa init hoặc tham số sai
 **********************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
    if (Adc_IsInitialized == 0U)
    {
        return E_NOT_OK;
    }

    if (Adc_GetGroupConfig(Group) == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (DataBufferPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    Adc_RuntimeData[Group].ResultBufferPtr = DataBufferPtr;
    Adc_RuntimeData[Group].ValidSamples = 0U;

    return E_OK;
}

/**********************************************************
 * @brief   De-initialize ADC Driver và trả ngoại vi về trạng thái an toàn
 * @details API sẽ dừng conversion đang chạy, disable trigger/IRQ, tắt DMA theo từng group,
 *          sau đó disable ADC unit đã dùng và reset toàn bộ runtime state.
 *          API chỉ biên dịch khi `ADC_DEINIT_API == STD_ON`.
 **********************************************************/
void Adc_DeInit(void)
{
#if (ADC_DEINIT_API == STD_ON)
    uint8 i;
    uint8 adc1Handled;
    uint8 adc2Handled;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        Adc_IsInitialized = 0U;
        return;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];

        ADC_SoftwareStartConvCmd(groupCfg->AdcInstance, DISABLE);
        ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, DISABLE);
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
        ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);

        Adc_DisableDmaForGroup(groupCfg);
    }

    adc1Handled = 0U;
    adc2Handled = 0U;

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];

        if (groupCfg->AdcInstance == ADC1)
        {
            if (adc1Handled != 0U)
            {
                continue;
            }
            adc1Handled = 1U;
        }
        else if (groupCfg->AdcInstance == ADC2)
        {
            if (adc2Handled != 0U)
            {
                continue;
            }
            adc2Handled = 1U;
        }
        else
        {
            continue;
        }

        ADC_Cmd(groupCfg->AdcInstance, DISABLE);
    }

    Adc_CurrentConfigPtr = NULL_PTR;
    Adc_CurrentPowerState = ADC_FULL_POWER;
    Adc_TargetPowerState = ADC_FULL_POWER;
    Adc_IsPowerStatePrepared = 0U;
    Adc_IsInitialized = 0U;
#else
    /* API bị tắt bởi pre-compile switch */
#endif
}

/**********************************************************
 * @brief   Bắt đầu quá trình chuyển đổi cho một group
 * @details Hàm kiểm tra đầy đủ điều kiện trước khi start:
 *          module đã init, group hợp lệ, đã có result buffer, trạng thái không BUSY
 *          và trigger mode/conversion mode hợp lệ. Sau đó apply cấu hình phần cứng
 *          theo group, cập nhật trạng thái BUSY và kích hoạt SW start nếu cần.
 * @param[in] Group ID group logic cần bắt đầu chuyển đổi
 **********************************************************/
void Adc_StartGroupConversion(Adc_GroupType Group)
{
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    if (Adc_RuntimeData[Group].Status == ADC_BUSY)
    {
        return;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        return;
    }

    if ((groupCfg->TriggerSource == ADC_TRIGG_SRC_HW) && (Adc_RuntimeData[Group].HwTriggerEnabled == 0U))
    {
        return;
    }

    if ((groupCfg->ConversionMode != ADC_CONV_MODE_ONESHOT) &&
        (groupCfg->ConversionMode != ADC_CONV_MODE_CONTINUOUS))
    {
        return;
    }

    if ((Adc_RuntimeData[Group].NotificationEnabled != 0U) && (groupCfg->NotificationCb != NULL_PTR))
    {
        ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, ENABLE);
        Adc_EnsureAdcNvicEnabled();
    }
    else
    {
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
    }

    Adc_RuntimeData[Group].Status = ADC_BUSY;
    Adc_RuntimeData[Group].ValidSamples = 0U;

    if (groupCfg->TriggerSource == ADC_TRIGG_SRC_SW)
    {
        ADC_ClearFlag(groupCfg->AdcInstance, ADC_FLAG_EOC);
        ADC_SoftwareStartConvCmd(groupCfg->AdcInstance, ENABLE);
    }
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Dừng quá trình chuyển đổi của một group
 * @details Hàm hủy lệnh software start, xóa cờ EOC, disable DMA liên quan
 *          và đưa trạng thái group về `ADC_IDLE`.
 *          API chỉ biên dịch khi `ADC_ENABLE_START_STOP_GROUP_API == STD_ON`.
 * @param[in] Group ID group logic cần dừng chuyển đổi
 **********************************************************/
void Adc_StopGroupConversion(Adc_GroupType Group)
{
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    if (Adc_RuntimeData[Group].Status == ADC_IDLE)
    {
        return;
    }

    ADC_SoftwareStartConvCmd(groupCfg->AdcInstance, DISABLE);
    ADC_ClearFlag(groupCfg->AdcInstance, ADC_FLAG_EOC);
    Adc_DisableDmaForGroup(groupCfg);

    Adc_RuntimeData[Group].Status = ADC_IDLE;
    Adc_RuntimeData[Group].ValidSamples = 0U;
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Đọc dữ liệu chuyển đổi của group ra buffer do caller cung cấp
 * @details Với group dùng DMA, dữ liệu được lấy từ runtime buffer đã được DMA cập nhật.
 *          Với group không dùng DMA, hàm đọc trực tiếp từ thanh ghi DR và lưu vào cache.
 *          Sau khi đọc xong, cập nhật `ValidSamples` và trạng thái COMPLETED/STREAM_COMPLETED.
 * @param[in] Group          ID group logic cần đọc
 * @param[out] DataBufferPtr Con trỏ buffer đích nhận dữ liệu kết quả
 * @return  `E_OK` nếu đọc thành công, `E_NOT_OK` nếu precondition không thỏa
 **********************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
#if (ADC_READ_GROUP_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;
    uint8 i;

    if (Adc_IsInitialized == 0U)
    {
        return E_NOT_OK;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (DataBufferPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (Adc_RuntimeData[Group].Status == ADC_IDLE)
    {
        return E_NOT_OK;
    }

    if (groupCfg->DmaConfig != NULL_PTR)
    {
        for (i = 0U; (i < groupCfg->NumChannels) && (i < ADC_MAX_CHANNELS_PER_GROUP); i++)
        {
            Adc_RuntimeData[Group].LastValues[i] = Adc_RuntimeData[Group].ResultBufferPtr[i];
        }
    }
    else
    {
        for (i = 0U; (i < groupCfg->NumChannels) && (i < ADC_MAX_CHANNELS_PER_GROUP); i++)
        {
            Adc_ValueGroupType value = (Adc_ValueGroupType)ADC_GetConversionValue(groupCfg->AdcInstance);
            Adc_RuntimeData[Group].LastValues[i] = value;
            Adc_RuntimeData[Group].ResultBufferPtr[i] = value;
        }
    }

    for (i = 0U; (i < groupCfg->NumChannels) && (i < ADC_MAX_CHANNELS_PER_GROUP); i++)
    {
        DataBufferPtr[i] = Adc_RuntimeData[Group].LastValues[i];
    }

    Adc_RuntimeData[Group].ValidSamples = groupCfg->NumChannels;
    Adc_RuntimeData[Group].Status = (groupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) ? ADC_STREAM_COMPLETED : ADC_COMPLETED;

    return E_OK;
#else
    (void)Group;
    (void)DataBufferPtr;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Bật hardware trigger cho group được cấu hình nguồn trigger phần cứng
 * @details Hàm chỉ chấp nhận group có `TriggerSource == ADC_TRIGG_SRC_HW`.
 *          Nếu group đang BUSY thì từ chối để tránh thay đổi trigger giữa phiên đo.
 *          Khi hợp lệ, driver bật cờ runtime và áp lại cấu hình phần cứng cho group.
 * @param[in] Group ID group logic cần bật hardware trigger
 **********************************************************/
void Adc_EnableHardwareTrigger(Adc_GroupType Group)
{
#if (ADC_HW_TRIGGER_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    if (groupCfg->TriggerSource != ADC_TRIGG_SRC_HW)
    {
        return;
    }

    if (Adc_RuntimeData[Group].Status == ADC_BUSY)
    {
        return;
    }

    Adc_RuntimeData[Group].HwTriggerEnabled = 1U;
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Tắt hardware trigger cho group
 * @details Chỉ áp dụng cho group có nguồn trigger phần cứng.
 *          API xóa cờ runtime `HwTriggerEnabled` và disable external trigger command.
 * @param[in] Group ID group logic cần tắt hardware trigger
 **********************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType Group)
{
#if (ADC_HW_TRIGGER_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    if (groupCfg->TriggerSource != ADC_TRIGG_SRC_HW)
    {
        return;
    }

    Adc_RuntimeData[Group].HwTriggerEnabled = 0U;
    ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, DISABLE);
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Bật notification callback cho group
 * @details Khi bật thành công, driver sẽ enable EOC interrupt cho ADC unit tương ứng
 *          và đảm bảo NVIC đã cấu hình để ISR có thể dispatch callback.
 *          API chỉ biên dịch khi `ADC_GRP_NOTIF_CAPABILITY == STD_ON`.
 * @param[in] Group ID group logic cần bật notification
 **********************************************************/
void Adc_EnableGroupNotification(Adc_GroupType Group)
{
#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    if (groupCfg->NotificationCb == NULL_PTR)
    {
        return;
    }

    Adc_RuntimeData[Group].NotificationEnabled = 1U;

    ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);
    ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, ENABLE);
    Adc_EnsureAdcNvicEnabled();
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Tắt notification callback của group
 * @details Driver xóa cờ runtime `NotificationEnabled` của group.
 *          Nếu không còn group nào trên cùng ADC unit bật notification,
 *          EOC interrupt của unit đó sẽ được disable.
 * @param[in] Group ID group logic cần tắt notification
 **********************************************************/
void Adc_DisableGroupNotification(Adc_GroupType Group)
{
#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    Adc_RuntimeData[Group].NotificationEnabled = 0U;

    if (Adc_HasEnabledNotificationOnUnit(groupCfg->AdcInstance) == 0U)
    {
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
        ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);
    }
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Lấy trạng thái runtime hiện tại của group
 * @details Nếu module chưa init hoặc group không hợp lệ, hàm trả về `ADC_IDLE`
 *          như trạng thái an toàn mặc định.
 * @param[in] Group ID group logic cần truy vấn
 * @return  Trạng thái thuộc `Adc_StatusType` của group
 **********************************************************/
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group)
{
    if (Adc_IsInitialized == 0U)
    {
        return ADC_IDLE;
    }

    if (Adc_GetGroupConfig(Group) == NULL_PTR)
    {
        return ADC_IDLE;
    }

    return Adc_RuntimeData[Group].Status;
}

/**********************************************************
 * @brief   Lấy con trỏ buffer mẫu hiện tại và số lượng mẫu hợp lệ
 * @details API trả về con trỏ tới result buffer đã đăng ký bằng `Adc_SetupResultBuffer`.
 *          Thường dùng cho access mode streaming để upper layer lấy dữ liệu gần nhất.
 * @param[in] Group ID group logic cần truy vấn
 * @param[out] PtrToSamplePtr Con trỏ nhận địa chỉ buffer mẫu
 * @return  Số lượng mẫu hợp lệ đang có trong runtime (`ValidSamples`)
 **********************************************************/
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PtrToSamplePtr)
{
    if (Adc_IsInitialized == 0U)
    {
        return 0U;
    }

    if (PtrToSamplePtr == NULL_PTR)
    {
        return 0U;
    }

    if (Adc_GetGroupConfig(Group) == NULL_PTR)
    {
        return 0U;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        return 0U;
    }

    *PtrToSamplePtr = Adc_RuntimeData[Group].ResultBufferPtr;
    return Adc_RuntimeData[Group].ValidSamples;
}

/**********************************************************
 * @brief   Trả thông tin phiên bản của ADC Driver
 * @details Điền dữ liệu vendor/module/software version vào cấu trúc `Std_VersionInfoType`.
 *          API chỉ có hiệu lực khi `ADC_VERSION_INFO_API == STD_ON`.
 * @param[out] VersionInfo Con trỏ cấu trúc nhận version info
 **********************************************************/
void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo)
{
#if (ADC_VERSION_INFO_API == STD_ON)
    if (VersionInfo == NULL_PTR)
    {
        return;
    }

    VersionInfo->vendorID = ADC_VENDOR_ID;
    VersionInfo->moduleID = ADC_MODULE_ID;
    VersionInfo->sw_major_version = ADC_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = ADC_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = ADC_SW_PATCH_VERSION;
#else
    (void)VersionInfo;
#endif
}

/**********************************************************
 * @brief   Chuẩn bị trạng thái nguồn mục tiêu cho ADC Driver
 * @details API chỉ ghi nhận `Adc_TargetPowerState` và đánh dấu đã chuẩn bị;
 *          việc áp thực tế được thực hiện bởi `Adc_SetPowerState`.
 * @param[in] PowerState Trạng thái nguồn mục tiêu cần chuẩn bị
 * @param[out] Result    Con trỏ nhận kết quả yêu cầu power-state
 * @return  `E_OK` khi chuẩn bị hợp lệ, `E_NOT_OK` khi không hỗ trợ hoặc sai điều kiện
 **********************************************************/
Std_ReturnType Adc_PreparePowerState(Adc_PowerStateType PowerState,
                                     Adc_PowerStateRequestResultType *Result)
{
    if (Result == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    if ((PowerState != ADC_FULL_POWER) && (PowerState != ADC_LOW_POWER_STATE))
    {
        *Result = ADC_POWER_STATE_NOT_SUPP;
        return E_NOT_OK;
    }

    Adc_TargetPowerState = PowerState;
    Adc_IsPowerStatePrepared = 1U;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    (void)PowerState;
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Áp trạng thái nguồn đã chuẩn bị lên các ADC unit
 * @details Hàm kiểm tra chuỗi gọi API (phải Prepare trước), validate trạng thái mục tiêu,
 *          sau đó bật/tắt ngoại vi ADC tương ứng và cập nhật trạng thái nguồn hiện tại.
 * @param[out] Result Con trỏ nhận kết quả yêu cầu power-state
 * @return  `E_OK` nếu áp thành công, `E_NOT_OK` nếu lỗi thứ tự gọi hoặc trạng thái không hợp lệ
 **********************************************************/
Std_ReturnType Adc_SetPowerState(Adc_PowerStateRequestResultType *Result)
{
    if (Result == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    if (Adc_IsPowerStatePrepared == 0U)
    {
        *Result = ADC_SEQUENCE_ERROR;
        return E_NOT_OK;
    }

    if ((Adc_TargetPowerState != ADC_FULL_POWER) && (Adc_TargetPowerState != ADC_LOW_POWER_STATE))
    {
        *Result = ADC_POWER_STATE_NOT_SUPP;
        return E_NOT_OK;
    }

    Adc_ApplyPowerStateToAllUnits(Adc_TargetPowerState);

    Adc_CurrentPowerState = Adc_TargetPowerState;
    Adc_IsPowerStatePrepared = 0U;

    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Lấy trạng thái nguồn hiện tại của ADC Driver
 * @details Trạng thái trả về là giá trị runtime do driver quản lý sau lần SetPowerState gần nhất.
 * @param[out] CurrentPowerState Con trỏ nhận trạng thái nguồn hiện tại
 * @param[out] Result            Con trỏ nhận kết quả dịch vụ
 * @return  `E_OK` nếu đọc thành công, `E_NOT_OK` nếu tham số sai hoặc module chưa init
 **********************************************************/
Std_ReturnType Adc_GetCurrentPowerState(Adc_PowerStateType *CurrentPowerState,
                                        Adc_PowerStateRequestResultType *Result)
{
    if ((CurrentPowerState == NULL_PTR) || (Result == NULL_PTR))
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    *CurrentPowerState = Adc_CurrentPowerState;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Lấy trạng thái nguồn mục tiêu đang được lưu trong driver
 * @details Giá trị này được đặt bởi `Adc_PreparePowerState` và dùng cho bước apply.
 * @param[out] TargetPowerState Con trỏ nhận trạng thái nguồn mục tiêu
 * @param[out] Result           Con trỏ nhận kết quả dịch vụ
 * @return  `E_OK` nếu đọc thành công, `E_NOT_OK` nếu tham số sai hoặc module chưa init
 **********************************************************/
Std_ReturnType Adc_GetTargetPowerState(Adc_PowerStateType *TargetPowerState,
                                       Adc_PowerStateRequestResultType *Result)
{
    if ((TargetPowerState == NULL_PTR) || (Result == NULL_PTR))
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    *TargetPowerState = Adc_TargetPowerState;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Hàm nền quản lý chuyển power-state bất đồng bộ
 * @details Khi bật chế độ async (`ADC_POWER_STATE_ASYNCH_TRANSITION_MODE == STD_ON`),
 *          hàm này sẽ kiểm tra cờ prepared và gọi `Adc_SetPowerState` để hoàn tất chuyển trạng thái.
 *          Ở chế độ đồng bộ hoặc không hỗ trợ low-power, hàm là no-op an toàn.
 **********************************************************/
void Adc_Main_PowerTransitionManager(void)
{
    if ((Adc_IsInitialized != 0U) && (Adc_IsPowerStatePrepared != 0U))
    {
        Adc_PowerStateRequestResultType resultDummy;
        (void)Adc_SetPowerState(&resultDummy);
    }
}

/**********************************************************
 * @brief   Logic ISR cho ngắt EOC của ADCx
 * @details Hàm được gọi bởi vector adapter trong `Adc_Cfg.c`.
 *          ISR sẽ lọc các group thuộc đúng ADC instance, cập nhật giá trị mẫu cuối,
 *          cập nhật trạng thái runtime và gọi notification callback nếu được bật.
 * @param[in] AdcInstance Con trỏ ADC instance phát sinh ngắt
 **********************************************************/
void Adc_IsrHandler(ADC_TypeDef *AdcInstance)
{
    uint8 i;

    if ((Adc_IsInitialized == 0U) || (AdcInstance == NULL_PTR) ||
        (Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return;
    }

    if (ADC_GetITStatus(AdcInstance, ADC_IT_EOC) == RESET)
    {
        return;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];

        if (groupCfg->AdcInstance != AdcInstance)
        {
            continue;
        }

        if (Adc_RuntimeData[i].NotificationEnabled == 0U)
        {
            continue;
        }

        {
            Adc_ValueGroupType value = (Adc_ValueGroupType)ADC_GetConversionValue(AdcInstance);
            Adc_RuntimeData[i].LastValues[0] = value;
            if (Adc_RuntimeData[i].ResultBufferPtr != NULL_PTR)
            {
                Adc_RuntimeData[i].ResultBufferPtr[0] = value;
            }
            Adc_RuntimeData[i].ValidSamples = 1U;
            Adc_RuntimeData[i].Status = ADC_COMPLETED;
        }

        if (groupCfg->NotificationCb != NULL_PTR)
        {
            groupCfg->NotificationCb();
        }
    }

    ADC_ClearITPendingBit(AdcInstance, ADC_IT_EOC);
}

/**********************************************************
 * @brief   Logic ISR cho ngắt DMA channel phục vụ ADC
 * @details Hàm xử lý cả hai sự kiện Half Transfer và Transfer Complete.
 *          Khi TC xảy ra, runtime sẽ cập nhật `LastValues`, `ValidSamples`, trạng thái group
 *          và gọi callback DMA complete nếu có cấu hình.
 * @param[in] DmaChannel Con trỏ DMA channel phát sinh ngắt
 **********************************************************/
void Adc_DmaIsrHandler(DMA_Channel_TypeDef *DmaChannel)
{
    uint8 i;

    if ((Adc_IsInitialized == 0U) || (DmaChannel == NULL_PTR) ||
        (Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];
        uint8 j;

        if ((groupCfg->DmaConfig == NULL_PTR) || (groupCfg->DmaConfig->DmaChannel != DmaChannel))
        {
            continue;
        }

        if ((groupCfg->DmaConfig->DmaHtFlag != 0U) && (DMA_GetITStatus(groupCfg->DmaConfig->DmaHtFlag) != RESET))
        {
            DMA_ClearITPendingBit(groupCfg->DmaConfig->DmaHtFlag);
            if (groupCfg->DmaHalfCb != NULL_PTR)
            {
                groupCfg->DmaHalfCb();
            }
        }

        if ((groupCfg->DmaConfig->DmaTcFlag != 0U) && (DMA_GetITStatus(groupCfg->DmaConfig->DmaTcFlag) != RESET))
        {
            DMA_ClearITPendingBit(groupCfg->DmaConfig->DmaTcFlag);

            if (Adc_RuntimeData[i].ResultBufferPtr != NULL_PTR)
            {
                for (j = 0U; (j < groupCfg->NumChannels) && (j < ADC_MAX_CHANNEL); j++)
                {
                    Adc_RuntimeData[i].LastValues[j] = Adc_RuntimeData[i].ResultBufferPtr[j];
                }
            }

            Adc_RuntimeData[i].ValidSamples = groupCfg->NumChannels;
            Adc_RuntimeData[i].Status = (groupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) ? ADC_STREAM_COMPLETED : ADC_COMPLETED;

            if (groupCfg->DmaCompleteCb != NULL_PTR)
            {
                groupCfg->DmaCompleteCb();
            }
        }
    }
}
