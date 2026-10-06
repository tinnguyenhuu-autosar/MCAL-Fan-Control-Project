/**
 * @file Dio.h
 * @brief Header file for Dio module (AUTOSAR Standard)
 */
#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"
#include "Dio_Cfg.h"
#include "stm32f10x_gpio.h"
#include <stddef.h>

/**
 * @brief Định nghĩa các ID cho cổng GPIO (Dio Port ID)
 */
#define DIO_PORT_A 0 /**< Ánh xạ cho cổng GPIOA */
#define DIO_PORT_B 1 /**< Ánh xạ cho cổng GPIOB */
#define DIO_PORT_C 2 /**< Ánh xạ cho cổng GPIOC */
#define DIO_PORT_D 3 /**< Ánh xạ cho cổng GPIOD */

/**
 * @brief Macro xác định cổng GPIO dựa trên ChannelId
 */
#define DIO_GET_PORT(ChannelId)   \
    (((ChannelId) < 16)   ? GPIOA \
     : ((ChannelId) < 32) ? GPIOB \
     : ((ChannelId) < 48) ? GPIOC \
     : ((ChannelId) < 64) ? GPIOD \
                          : NULL)

/**
 * @brief Macro xác định chân GPIO (bit mask) dựa trên ChannelId
 */
#define DIO_GET_PIN(ChannelId) (1 << ((ChannelId) % 16))

/**
 * @brief Macro tạo ChannelId từ GPIO Port và Pin Index
 * @param[in] GPIOx Giá trị đại diện Port (0, 1, 2, 3...)
 * @param[in] Pin   Số thứ tự chân (0-15)
 */
#define DIO_CHANNEL(GPIOx, Pin) (((GPIOx) << 4) + (Pin))

/**
 * Định nghĩa Channel ID cho tất cả các chân
 */
/*==================================================================================================
*  SINH ENUM Dio_ChannelIdType TỰ ĐỘNG TỪ DIO_ALL_PIN_LIST
==================================================================================================*/
#define X(port, pin) DIO_CHANNEL_##port##pin = DIO_CHANNEL(DIO_PORT_##port, pin),

typedef enum
{
    DIO_ALL_PIN_LIST
} Dio_ChannelIdType;

#undef X

/**
 * Type definitions
 */

/**
 * @brief kiểu dữ liệu cho 1 kênh Dio (channel)
 * @details định danh cho 1 chân (pin) cụ thể
 */
typedef uint8 Dio_ChannelType;

/**
 * @brief   Kiểu dữ liệu cho một cổng Dio (Port)
 * @details Định danh cho một cổng (port) cụ thể
 */
typedef uint16 Dio_PortType;

/**
 * @brief   Cấu trúc định nghĩa một nhóm các kênh Dio (Channel Group)
 * @details Dùng để thao tác với tập hợp con các chân trong cùng một Port
 */
typedef struct
{
    Dio_PortType port; /**< Cổng Dio của nhóm */
    uint8 offset;      /**< Vị trí bắt đầu (độ dịch) của nhóm bit */
    uint8 mask;        /**< Mặt nạ (mask) xác định các bit thuộc nhóm */
} Dio_ChannelGroupType;

/**
 * @brief   Kiểu dữ liệu cho mức logic của một kênh Dio
 * @details Các mức logic này sẽ là STD_HIGH (1) hoặc STD_LOW (0)
 */
typedef uint8 Dio_LevelType;

/**
 * @brief   Kiểu dữ liệu cho mức logic của một cổng Dio
 * @details Mỗi cổng có thể chứa nhiều kênh, do đó mức logic của cổng là tập hợp trạng thái của các chân đó
 */
typedef uint16 Dio_PortLevelType;

/**
 * FUNCTION DEFINITIONS
 */

/**
 * @func    Dio_ReadChannel
 * @brief   Đọc trạng thái của một kênh Dio
 *
 * @param[in] ChannelId ID của kênh Dio cần đọc
 *
 * @return  Dio_LevelType
 *          - STD_HIGH: Mức logic cao.
 *          - STD_LOW:  Mức logic thấp.
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @func    Dio_WriteChannel
 * @brief   Ghi trạng thái logic cho một kênh Dio
 *
 * @param[in] ChannelId ID của kênh Dio cần ghi.
 * @param[in] Level     Trạng thái cần ghi (STD_HIGH hoặc STD_LOW)
 *
 * @return  void
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @func    Dio_ReadPort
 * @brief   Đọc trạng thái của toàn bộ một cổng Dio
 *
 * @param[in] PortId ID của cổng DIO cần đọc
 *
 * @return  Dio_PortLevelType Trạng thái logic của toàn bộ cổng
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @func    Dio_WritePort
 * @brief   Ghi trạng thái logic cho toàn bộ một cổng Dio
 *
 * @param[in] PortId ID của cổng Dio cần ghi
 * @param[in] Level  Giá trị trạng thái logic cần ghi
 *
 * @return  void
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @func    Dio_ReadChannelGroup
 * @brief   Đọc mức logic của một nhóm kênh DIO.
 *
 * @param[in] GroupIdPtr Con trỏ đến cấu hình nhóm DIO.
 *
 * @return  Dio_PortLevelType Trạng thái logic của nhóm kênh (đã dịch bit).
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType *GroupIdPtr);

/**
 * @func    Dio_WriteChannelGroup
 * @brief   Ghi mức logic cho một nhóm kênh DIO.
 *
 * @param[in] GroupIdPtr Con trỏ đến cấu hình nhóm DIO.
 * @param[in] Level      Mức logic cần ghi cho nhóm (chưa dịch bit).
 *
 * @return  void
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @func    Dio_GetVersionInfo
 * @brief   Lấy thông tin phiên bản của DIO Driver
 *
 * @param[out] VersionInfo Con trỏ để lưu thông tin phiên bản
 *
 * @return  void
 */
void Dio_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/**
 * @func    Dio_FlipChannel
 * @brief   Lật trạng thái logic của một kênh DIO
 *
 * @param[in] ChannelId ID của kênh DIO cần lật
 *
 * @return  Dio_LevelType Trạng thái mới của kênh sau khi lật
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId);

#endif /* DIO_H */
