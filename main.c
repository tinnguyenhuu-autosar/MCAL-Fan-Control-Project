#include "IoHwAb.h"

#define TEMP_LOW (30U)
#define TEMP_HIGH (40U)

/* Hàm delay đơn giản (không dùng timer) */
static void Delay(uint32 nCount)
{
    for (; nCount != 0U; nCount--)
        ;
}

int main(void)
{
    uint8 nhietDo = 0U;

    /* Cấu hình Port, Dio, Adc, Pwm */
    IoHwAb_Init();

    while (1)
    {
        if (IoHwAb_ReadTemperature(&nhietDo) == E_OK)
        {
            if (nhietDo < TEMP_LOW)
            {
                IoHwAb_SetFanDuty(0U); /* Tắt quạt */
                IoHwAb_SetLed(FALSE);  /* Tắt LED */
            }
            else if (nhietDo < TEMP_HIGH)
            {
                IoHwAb_SetFanDuty(50U); /* Quạt 50% */
                IoHwAb_SetLed(TRUE);    /* Bật LED */
            }
            else
            {
                IoHwAb_SetFanDuty(100U); /* Quạt 100% */
                IoHwAb_SetLed(TRUE);     /* Bật LED */
            }
        }
        else
        {
            /* Đọc ADC lỗi -> chạy quạt 100% cho an toàn */
            IoHwAb_SetFanDuty(100U);
            IoHwAb_SetLed(TRUE);
        }

        Delay(500U);
    }
}