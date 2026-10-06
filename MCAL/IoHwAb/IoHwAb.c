/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "IoHwAb.h"
#include "Port_Cfg.h"
#include "Dio_Cfg.h"
#include "Adc_Cfg.h"
#include "Pwm_Cfg.h"

/* Khai báo các cấu hình*/
extern const Port_ConfigType Port_Config;
extern const Adc_ConfigType AdcDriverConfig;
extern const Pwm_ConfigType Pwm_Config;

/*******************************************************************************
 *  Định nghĩa hàm để gọi MCAL
 ******************************************************************************/
/*******************************************************************************
 * @brief       Hàm khởi tạo module IoHwAb.
 * @details     Thiết lập trạng thái ban đầu và config các driver
 * @param[in]   None
 * @return      E_OK      Config thành công
 * @return      E_NOT_OK  Config thất bại
 ******************************************************************************/
Std_ReturnType IoHwAb_Init(void)
{

    /* Config Port */
    Port_Init(&Port_Config);

    /* Config ADC */
    Adc_Init(&AdcDriverConfig);

    /* Bật ADC */
    Adc_StartGroupConversion(ADC_GROUP_1);

    /* Config PWM */
    Pwm_Init(&Pwm_Config);

    return E_OK;
}

/*******************************************************************************
 * @brief       Đọc nhiệt độ hiện tại.
 * @details     Đọc giá trị từ ADC rồi chuyển đổi sang độ C
 * @return      E_OK      Đọc giá trị nhiệt độ thành công.
 * @return      E_NOT_OK  Chưa đọc được giá trị nhiệt độ.
 ******************************************************************************/
Std_ReturnType IoHwAb_ReadTemperature(uint8 *temp)
{
    Adc_ValueGroupType adcValue = (Adc_ValueGroupType)0U;
    uint32 voltValue = 0U;

    /* Kiểm tra tham số đầu vào */
    if (temp == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra trạng thái đọc ADC thành công không*/
    if (Adc_ReadGroup(ADC_GROUP_1, &adcValue) != E_OK)
    {
        return E_NOT_OK;
    }
    else
    {
        /* Đọc giá trị ADC ban đầu sau khi đã chuyển đổi xong và tính giá trị điện áp. */
        voltValue = (uint32)((adcValue * 3.3) / 4095U);

        /* Chuyển đổi giá trị điện áp sang nhiệt độ (độ C) cho LM35*/
        *temp = (uint8)(voltValue / 0.01); /* LM35: 10mV là 1 độ C */
    }

    return E_OK;
}

/*******************************************************************************
 * @brief       Cài đặt duty cycle của quạt
 * @details
 * @param[in]   percent   Set mức quạt từ 0 -> 100 (%) là 0x0000 -> 0x8000
 * @return      E_OK      Cái đặt duty cycle thành công.
 * @return      E_NOT_OK  Cài đặt duty cycle thất bại
 ******************************************************************************/
Std_ReturnType IoHwAb_SetFanDuty(uint8 percent)
{
    uint16 dutyPwm;

    /* Chuyển đổi giá trị phần trăm sang duty cycle. */
    dutyPwm = (percent * 0x8000) / 100U;

    /* Áp giá trị duty cycle mới cho kênh PWM điều khiển quạt. */
    Pwm_SetDutyCycle(0u, dutyPwm);

    return E_OK;
}

/*******************************************************************************
 * @brief       Set trạng thái LED
 * @details
 * @param[in]   state     TRUE: LED ON. FALSE: LED OFF.
 * @return      E_OK      LED được bật
 * @return      E_NOT_OK  LED chưa được bật
 ******************************************************************************/
Std_ReturnType IoHwAb_SetLed(boolean state)
{

    if (state == TRUE)
    {
        /* Bật LED */
        Dio_WriteChannel(DIO_CHANNEL_C13, STD_HIGH);
    }
    else
    {
        /* Tắt LED */
        Dio_WriteChannel(DIO_CHANNEL_C13, STD_LOW);
    }

    return E_OK;
}
