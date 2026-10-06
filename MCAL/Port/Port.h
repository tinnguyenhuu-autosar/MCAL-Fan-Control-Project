/**
 * @file Port.h
 * @brief Header file for Port module (AUTOSAR Standard)
 */
#ifndef PORT_H
#define PORT_H

#include "Port_Cfg.h"

#include "Std_Types.h"
#include "stm32f10x_gpio.h"

/**
 * Khai báo prototype các hàm API của Port Driver AUTOSAR
 */

/**
 * @brief   Khởi tạo toàn bộ các Port/Pin
 * @param[in] ConfigPtr Con trỏ tới cấu hình Port/Pin
 */
void Port_Init(const Port_ConfigType *ConfigPtr);

/**
 * @brief   Đổi chiều một chân Port (nếu được phép)
 * @param[in] Pin        Số hiệu pin
 * @param[in] Direction  Chiều cần đặt
 */
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction);

/**
 * @brief   Làm mới lại chiều tất cả các pin không cho đổi chiều runtime
 */
void Port_RefreshPortDirection(void);

/**
 * @brief   Lấy thông tin version của Port Driver
 * @param[out] versioninfo  Con trỏ tới cấu trúc Std_VersionInfoType để nhận version
 */
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo);

/**
 * @brief   Đổi mode chức năng cho một chân pin (nếu cho phép)
 * @param[in] Pin    Số hiệu pin
 * @param[in] Mode   Mode chức năng cần set
 */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode);

#endif /* PORT_H */