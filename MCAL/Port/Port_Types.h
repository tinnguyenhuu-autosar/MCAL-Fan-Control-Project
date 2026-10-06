#include "Std_Types.h"

/**
 * @brief Định nghĩa các ID cho cổng GPIO (Port ID)
 */
#define PORT_ID_A 0 /**< Ánh xạ cho cổng GPIOA */
#define PORT_ID_B 1 /**< Ánh xạ cho cổng GPIOB */
#define PORT_ID_C 2 /**< Ánh xạ cho cổng GPIOC */
#define PORT_ID_D 3 /**< Ánh xạ cho cổng GPIOD */

/**
 * @brief Macro xác định con trỏ Port theo PortNum
 */
#define PORT_GET_PTR(PortNum)                                            \
    (((PortNum) == PORT_ID_A) ? GPIOA : ((PortNum) == PORT_ID_B) ? GPIOB \
                                    : ((PortNum) == PORT_ID_C)   ? GPIOC \
                                    : ((PortNum) == PORT_ID_D)   ? GPIOD \
                                                                 : NULL)

/**
 * @brief Macro xác định chân GPIO (bit mask) cho từng chân GPIO
 */
#define PORT_GET_PIN_MASK(PinNum) (1U << (PinNum))

/**
 * @brief Định nghĩa các chế đọ Mode và trạng thái cho Pin
 */
#define PORT_PIN_MODE_DIO 0 /**< Chế độ Digital I/O */
#define PORT_PIN_MODE_ADC 1 /**< Chế độ Analog Input */
#define PORT_PIN_MODE_PWM 2 /**< Chế độ PWM Output */
#define PORT_PIN_MODE_SPI 3 /**< Chế độ SPI */

#define PORT_PIN_PULL_NONE 0 /**< Không có điện trở kéo lên/xuống */
#define PORT_PIN_PULL_UP 1   /**< Điện trở kéo lên */
#define PORT_PIN_PULL_DOWN 2 /**< Điện trở kéo xuống */

#define PORT_PIN_LEVEL_LOW 0  /**< Mức logic thấp */
#define PORT_PIN_LEVEL_HIGH 1 /**< Mức logic cao */

/**
 * Macro định nghĩa phiên bản, vendor, modunle ID cho VersionInfo
 */
#define PORT_VENDOR_ID 1234U
#define PORT_MODULE_ID 81U
#define PORT_SW_MAJOR_VERSION 1U
#define PORT_SW_MINOR_VERSION 0U
#define PORT_SW_PATCH_VERSION 0U

/**
 * Định nghĩa kiểu dữ liệu của Port Driver AUTOSAR
 */
/**
 * @typedef Port_Pinstype
 * @brief Kiểu dữ liệu cho một chân Port (0-47, A0-A15, B0-B15, C0-C15, D0-D15)
 */
typedef uint8 Port_PinType;

/**
 * @typedef Port_PinDirectionType
 * @brief Kiểu dữ liệu cho hướng của chân Port (Input hoặc Output)
 */
typedef enum
{
    PORT_PIN_IN = 0x00, /**< Chân Port là Input */
    PORT_PIN_OUT = 0x01 /**< Chân Port là Output */
} Port_PinDirectionType;

/**
 * @typedef Port_ModeType
 * @brief Kiểu dữ liệu cho chế độ hoạt động của chân Port (DIO, ADC, PWM, SPI)
 */
typedef uint8 Port_PinModeType;

/**
 * @struct Port_PinConfigType
 * @brief Cấu hình cho từng chân pin
 */
typedef struct
{
    uint8 PortNum;                   /**0=A, 1=B, 2=C, 3=D */
    uint8 PinNum;                    /**<0-15 */
    Port_PinModeType Mode;           /**< Chế độ hoạt động của chân (DIO, ADC, PWM, SPI) */
    Port_PinDirectionType Direction; /**< Hướng của chân (Input hoặc Output) */
    uint8 Speed;                     /**< Tốc độ của chân (nếu là Output) */
    uint8 DirectionChangeable;       /**< Cho phép thay đổi hướng hay không 1=cho phép, 0=không cho phép */
    uint8 Level;                     /**< Mức logic ban đầu (nếu là Output) */
    uint8 Pull;                      /**< Loại điện trở kéo lên/xuống (Pull-up, Pull-down, None) */
    uint8 ModeChangeable;            /**< Cho phép thay đổi chế độ hay không 1=cho phép, 0=không cho phép */
} Port_PinConfigType;

/**
 * @struct Port_ConfigType
 * @brief Cấu hình toàn bộ tập hợp Pin (gán khi init)
 */
typedef struct
{
    const Port_PinConfigType *PinConfigs; /**< Con trỏ tới mảng cấu hình pin */
    uint16 PinCount;                      /**< Số lượng chân cấu hình */
} Port_ConfigType;