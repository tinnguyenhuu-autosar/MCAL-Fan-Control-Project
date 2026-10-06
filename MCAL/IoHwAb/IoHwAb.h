/*******************************************************************************
 * @file    IoHwAb.h
 * @brief   File header khai báo API cho module I/O Hardware Abstraction.
 *
 * @details Khai báo hàm trong file .c để cấu hình MCAL
 ******************************************************************************/

#ifndef IOHWAB_H
#define IOHWAB_H

/*******************************************************************************
 * Khai báo include
 ******************************************************************************/
#include "Adc.h"
#include "Dio.h"
#include "Port.h"
#include "Pwm.h"
#include "Std_Types.h"

/*******************************************************************************
 * Khai báo API
 ******************************************************************************/

/**
 * @brief   Khởi tạo I/O Hardware Abstraction.
 * @details Hàm này khởi tạo toàn bộ hoặc một phần IoHwAb tùy cấu hình dự án.
 *          Nếu dự án có nhiều Init ID, tên hàm có thể được mở rộng theo dạng
 *          IoHwAb_Init<InitId>. Theo AUTOSAR, ConfigPtr hiện tại không được dùng
 *          và phải truyền NULL_PTR.
 *
 * @param[in] ConfigPtr Con trỏ tới bộ cấu hình được chọn.
 */
Std_ReturnType IoHwAb_Init(void);

/*******************************************************************************
 * @brief       Hàm đọc nhiệt độ hiện tại.
 * @details     Đọc giá trị từ ADC rồi chuyển đổi sang độ C
 * @return      E_OK      Đọc giá trị nhiệt độ thành công.
 * @return      E_NOT_OK  Chưa đọc được giá trị nhiệt độ.
 ******************************************************************************/
Std_ReturnType IoHwAb_ReadTemperature(uint8 *temp);

/*******************************************************************************
 * @brief       Cài đặt duty cycle của quạt
 * @details
 * @param[in]   percent   Set mức quạt từ 0 -> 100 (%) là 0x0000 -> 0x8000
 * @return      E_OK      Cái đặt duty cycle thành công.
 * @return      E_NOT_OK  Cài đặt duty cycle thất bại
 ******************************************************************************/
Std_ReturnType IoHwAb_SetFanDuty(uint8 percent);

/*******************************************************************************
 * @brief       Set trạng thái LED
 * @details
 * @param[in]   state     TRUE: LED ON. FALSE: LED OFF.
 * @return      E_OK      LED được bật
 * @return      E_NOT_OK  LED chưa được bật
 ******************************************************************************/
Std_ReturnType IoHwAb_SetLed(boolean state);

#endif /* IOHWAB_H */
