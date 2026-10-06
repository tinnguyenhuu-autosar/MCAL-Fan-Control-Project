/**
* @file     Adc.h
* @brief    Analog-to-Digital Converter (ADC) Driver Header
* @details  Khai báo kiểu dữ liệu và API ADC Driver theo chuẩn AUTOSAR,
*           sử dụng thư viện SPL của STM32F103.
* @note     ADC Driver chỉ quản lý ngoại vi ADC/DMA/IRQ.
            Cấu hình chân GPIO thuộc về Port Driver.
*/

#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"
#include "Adc_Types.h"

#include "Adc_Cfg.h"

/**
 * API chuẩn SWS ADC
 */
/**
 * @brief   Khởi tạo ADC Driver
 * @details Hàm này chỉ cấu hình phần ADC peripheral (clock, init, calibration,
 *         DMA, IRQ). Cấu hình chân GPIO analog thuộc về Port Driver.
 * @param[in]   ConfigPtr: Con trỏ đến cấu hình tổng ADC Driver
 */
void Adc_Init(const Adc_ConfigType *ConfigPrt);

/**
 * @brief Gán buffer kết quả ADC cho một ADC group
 * @details Buffer này là vùng nhớ đích để Driver lưu kết quả chuyển đổi
 *          (Single/Streaming). API này cần gọi trước khi start conversion.
 * @param[in]   Group: Nhóm ADC cần gán buffer
 * @param[in]   DataBufferPtr: Con trỏ đến buffer kết quả
 * @return  Std_ReturnType: E_OK nếu thành công, E_NOT_OK nếu thất bại
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPrt);

/**
 * @brief Đưa ADC Driver về trạng thái uninit (de-initialize)
 * @details Tắt conversion/trigger/IRQ/DMA vaf reset runtime state cuar driver.
 *          API này chỉ có hiệu lực khi 'ADC_DEINIT_API==STD_ON'.
 */
void Adc_DeInit(void);

/**
 * @brief Bắt đầu chuyển đổi cho 1 gr
 * @details Với gr sw_trigg: dri sẽ phát lệnh start conv.
 *          Với gr hw_trigg: cần bật trigg trc = API tương ứng.
 * @param[in] Group ID gr logic cần bắt đầu chuyển đổi
 */
void Adc_StartGroupConversion(Adc_GroupType Group);

/**
 * @brief Dừng chuyển đổi của 1 gr
 * @details API này dùng conv và đưa trạng tháy gr về IDLE.
 * @param[in] Group ID gr logic cần dừng chuyển đổi.
 */
void Adc_StopGroupConversion(Adc_GroupType Group);

/**
 * @brief Đọc kết quả chuyển đổi mới nhất của gr
 * @details Kết quả sẽ đc copy ra vùng nhớ 'DataBufferPrt' theo cấu hình gr.
 *          API này chỉ có hiệu lực khi 'ADC_READ_GROUP_API==STD_ON'.
 * @param[in] Group ID gr logic cần đọc dữ liệu
 * @param[out] DataBufferPrt con trỏ vùng nhớ nhận dữ liệu
 * @return E_OK nếu đọc thành công, E_NOT_OK nếu không có dữ liệu hợp lệ hoặc sai precondition
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPrt);

/**
 * @brief Bật HW trigg cho gr
 * @details Chỉ áp dụng cho gr có cấu hình trigg nguồn phần cứng.
 *          API này chỉ có hiệu lực khi 'ADC_HW_TRIGGER_API==STD_ON'
 * @param[in] Group ID gr logic cần bật HW trigg.
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Tắt hw trigg cho gr
 * @details chỉ áp dụng cho gr cấu hình trigg nguồn phần cứng.
 *          API này chỉ có hiệu lực khi 'ADC_HW_TRIGGER_API==STD_ON'
 * @param[in] Group ID gr cần tắt HW trigg
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Bật Notif callback cho gr
 * @details Khi EOC interrupt của ADC uint xảy ra, callback gr tương ứng
 *          sẽ đc gọi nếu bật notif.
 * @param[in] Group ID gr cần bật notif.
 */
void Adc_EnableGroupNotification(Adc_GroupType Group);

/**
 * @brief Tắt notif callback cho gr
 * @details Sau khi tắt, callback của gr sẽ ko còn đc dispath từ ISR.
 *          Nếu ko còn gr nào bật notif trên cùng ADC unit,
 *          driver có thể disable EOC interrupt của unit đó để giảm tải ngắt.
 * @param[in] Group ID gr cần tắt notif.
 */
void Adc_DisableGroupNotification(Adc_GroupType Group);

/**
 * @brief Lấy trạng thái hiện tại của gr
 * @details Trạng thái phản ánh runtime state do ADC driver quản lý nội bộ.
 *          Mếu module chưa khởi tạo or gr ko hợp lệ, giá trị trả về an toàn là 'ADC_IDLE'.
 * @param[in] Group ID gr cần đọc trạng thái.
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**
 * @brief lấy con trỏ mẫu cuối và số lượng mẫu hợp lệ của gr
 * @details Dùng cho access mode streaming dể upper layer lấy vùng dữ liệu gần nhất
 * @param[in] Group Id gr cần truy vấn
 * @param[out] PrtToSamplePrt con trỏ nhận địa chỉ mẫu
 * @return  Số lượng mẫu hợp lệ hiện có trong buffer
 */
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PrtToSamplePrt);

/**
 * @brief Lấy thông tin version của ADC Driv
 * @details API điền vendor ID, module ID và SW ver thwo macro cấu hình.
 *          API này chỉ có hiệu lực khi 'ADC_VERSION_INFO_API==STD_ON'
 * @param[out] VersionInfo con trỏ cấu trúc nhận thông tin version
 */
void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/**
 * @brief Áp trạng thái nguồn đã chuẩn bị cho ADC driver
 * @details API này thực hiện bước apply sau khi gọi 'Adc_PreparePowerState'.
 *          Nếu chưa prepare or target ko hợp lệ, hàm trả lỗi theo quy ước AUTOSAR.
 * @param[out] Result con trỏ nhận kết quả yêu cần powerstate.
 * @return  E_OK nếu chuyển thành công, E_NOT_OK nếu lỗi precondition/trạng thái
 */
Std_ReturnType Adc_SetPowerState(Adc_PowerStateRequestResultType *Result);

/**
 * @brief lấy trạng thái nguồn hiện tại của ADC driv
 * @details Trạng thái này phản ánh mức nguồn mà driver đag áp dụng thực tế cho ADC unit.
 * @param[out] CurrentPowerState con trỏ nhận trạng thái nguồn hiện tại
 * @param[out] Result            con trỏ nhận kết quả dịch vụ
 * @return E_OK nếu chuyển thành công, E_NOT_OK nếu lỗi precondition/trạng thái
 */
Std_ReturnType Adc_GetCurrentPowerState(Adc_PowerStateType *CurrentPowerState,
                                        Adc_PowerStateRequestResultType *Result);

/**
 * @brief Lấy trạng thái nguồn mục tiêu đag đc chuẩn bị
 * @details Trạng thái này đc set bởi 'Adc_PreparePowerState' và dùng cho bc apply.
 * @param[out] TargetPowerState con trỏ nhận trạng thái nguồn mục tiêu
 * @param[out] Result           con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu chuyển thành công, E_NOT_OK nếu lỗi precondition/trạng thái
 */
Std_ReturnType Adc_GetTargetPowerState(Adc_PowerStateType *TargetPowerState,
                                       Adc_PowerStateRequestResultType *Result);

/**
 * @brief Chuẩn bị trạng thái nguồn mục tiêu cho ADC driv
 * @details API chỉ ghi nhận mục tiêu chuyển trạng thái, chưa áp phần cứng ngay.
 *          Việc áp thực tế đc thực hiện khi gọi 'Adc_SetPowerState'.
 * @param[in] PowerState Trạng thái nguồn mục tiêu cần chuẩn bị
 * @param[out] Result    Con trỏ nhận kết quả dịch vụ
 * @return E_OK nếu chuyển thành công, E_NOT_OK nếu ko hỗ trợ/ko hợp lệ
 *  */
Std_ReturnType Adc_PreparePowerState(Adc_PowerStateType PowerState,
                                     Adc_PowerStateRequestResultType *Result);

/**
 * @brief Hàm nền quản lý chuyển trạng thái nguồn bất đồng bộ
 * @details Chỉ hoạt động khi 'ADC_POWER_STATE_ASYNCH_TRANSITION_MODE==STD_ON'.
 */
void Adc_Main_PowerTransitionManager(void);

/**
 * API nội bộ cho adapter IRQ (MCAL integration layer)
 * Được gọi từ vector IRQ trong file cấu hình Adc_Cfg.c
 */
/**
 * @brief Handler logic cho ngắt ADCx
 * @details Hàm này ko phải vector IRQ trực tiếp; vector sẽ gọi vào đây.
 * @param[in] AdcInstance Con trỏ ADC instance phát sinh ngắt
 */
void Adc_IsrHandler(ADC_TypeDef *AdcInstance);

/**
 * @brief Handler logic cho ngắt DMA channel của ADC
 * @details Hàm này xử lý HT/TC flag và dispath callback DMA theo gr.
 * @param[in] Dmachannel con trỏ DMA channel phát sinh ngắt.
 */
void Adc_DmaIsrHandler(DMA_Channel_TypeDef *Dmachannel);

#endif /* ADC_H */