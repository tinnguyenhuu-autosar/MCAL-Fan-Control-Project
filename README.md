# AUTOSAR MCAL Fan Control Project

## Mục lục

- [1. Mục tiêu](#1-mục-tiêu)
- [2. Mô tả chức năng yêu cầu](#2-mô-tả-chức-năng-yêu-cầu)
- [3. Danh sách linh kiện sử dụng](#3-danh-sách-linh-kiện-sử-dụng)
- [4. Kiến trúc phần mềm (AUTOSAR Classic)](#4-kiến-trúc-phần-mềm-autosar-classic)
  - [4.1 MCAL Drivers](#41-mcal-drivers)
  - [4.2 Module IoHwAb](#42-module-iohwab)
- [5. Cấu trúc cây thư mục](#5-cấu-trúc-cây-thư-mục)
- [6. Luồng hoạt động](#6-luồng-hoạt-động)
- [7. Build Project](#7-build-project)
- [8. Kết quả Build](#8-kết-quả-build)

---

# 1. Mục tiêu

Project thực hiện xây dựng mô hình **Fan Control trên STM32F103C8T6** theo kiến trúc phần mềm định hướng **AUTOSAR Classic**.

Mục tiêu chính:

- Xây dựng các MCAL Driver cơ bản cho STM32F103.
- Áp dụng cách tổ chức module và configuration theo AUTOSAR.
- Sử dụng **ADC** để đọc tín hiệu cảm biến nhiệt độ LM35DZ.
- Sử dụng **PWM** để điều khiển tốc độ quạt DC 12V.
- Sử dụng **DIO** để điều khiển LED trạng thái.
- Xây dựng lớp **IoHwAb** để tạo abstraction giữa application và MCAL.
- Build firmware bằng **GNU Arm Embedded Toolchain**.

---

# 2. Mô tả chức năng yêu cầu

Hệ thống thực hiện điều khiển quạt dựa trên nhiệt độ đo được từ cảm biến LM35DZ.

### Chức năng chính

1. Khởi tạo Port Driver.
2. Khởi tạo ADC Driver.
3. Đọc giá trị ADC từ cảm biến nhiệt độ.
4. Chuyển đổi giá trị ADC sang điện áp và nhiệt độ.
5. Điều khiển duty cycle PWM của quạt.
6. Điều khiển LED trạng thái thông qua DIO.

### Luồng xử lý

```
LM35DZ
   │
   ▼
  ADC
   │
   ▼
 IoHwAb
   │
   ├── Temperature
   │
   └── Fan Duty
          │
          ▼
         PWM
          │
          ▼
     MOSFET Module
          │
          ▼
       DC Fan 12V
```

---

# 3. Danh sách linh kiện sử dụng

| Linh kiện                   | Mô tả                          |
| --------------------------- | ------------------------------ |
| STM32F103C8T6 ("Blue Pill") | Vi điều khiển chính            |
| LM35DZ                      | Cảm biến nhiệt độ analog       |
| Module MOSFET               | Mạch công suất điều khiển quạt |
| Quạt DC 12V (80x80mm)       | Thiết bị được điều khiển       |
| LED built-in                | Hiển thị trạng thái            |

---

# 4. Kiến trúc phần mềm (AUTOSAR Classic)

Project được tổ chức theo hướng phân lớp của AUTOSAR Classic:

```text
+----------------------------------+
|           Application            |
+----------------------------------+
                │
                ▼
+----------------------------------+
|             IoHwAb               |
|        I/O Hardware Abstraction  |
+----------------------------------+
                │
                ▼
+----------------------------------+
|               MCAL               |
|                                  |
|  ADC     DIO     PORT     PWM    |
+----------------------------------+
                │
                ▼
+----------------------------------+
|          STM32F103 / SPL         |
+----------------------------------+
                │
                ▼
+----------------------------------+
|             Hardware             |
+----------------------------------+
```

## 4.1 MCAL Drivers

Project triển khai các driver MCAL chính:

### ADC

- Khởi tạo ADC.
- Cấu hình ADC Group.
- Start/Stop conversion.
- Đọc kết quả ADC.
- Quản lý configuration của ADC.

### DIO

- Đọc trạng thái GPIO.
- Ghi trạng thái GPIO.
- Hỗ trợ thao tác Channel và Channel Group.

### PORT

- Cấu hình GPIO pin.
- Cấu hình direction Input/Output.
- Cấu hình pin mode.
- Cấu hình Pull-up/Pull-down.
- Thay đổi direction/mode khi được cho phép bởi configuration.

### PWM

- Khởi tạo PWM.
- Cấu hình timer/channel.
- Điều khiển duty cycle.
- Sử dụng PWM channel để điều khiển quạt.

---

## 4.2 Module IoHwAb

`IoHwAb` đóng vai trò abstraction giữa application và các MCAL Driver.

Các API chính:

```c
IoHwAb_Init();
IoHwAb_ReadTemperature();
IoHwAb_SetFanDuty();
IoHwAb_SetLed();
```

Ví dụ:

```
Application
     │
     ▼
IoHwAb_ReadTemperature()
     │
     ▼
Adc_ReadGroup()
     │
     ▼
ADC Hardware
```

Điều khiển quạt:

```
Application
     │
     ▼
IoHwAb_SetFanDuty()
     │
     ▼
Pwm_SetDutyCycle()
     │
     ▼
TIM2_CH1
     │
     ▼
MOSFET
     │
     ▼
12V Fan
```

---

# 5. Cấu trúc cây thư mục

```text
Project_FanConTrol_AUTOSAR/
│
├── MCAL/
│   │
│   ├── Adc/
│   │   ├── Adc.c
│   │   ├── Adc.h
│   │   └── Adc_Types.h
│   │
│   ├── Dio/
│   │   ├── Dio.c
│   │   └── Dio.h
│   │
│   ├── Port/
│   │   ├── Port.c
│   │   ├── Port.h
│   │   └── Port_Types.h
│   │
│   ├── Pwm/
│   │   ├── Pwm.c
│   │   └── Pwm.h
│   │
│   ├── Config/
│   │   ├── Adc_Cfg.c
│   │   ├── Adc_Cfg.h
│   │   ├── Port_Cfg.c
│   │   ├── Port_Cfg.h
│   │   ├── Pwm_Cfg.c
│   │   └── Pwm_Cfg.h
│   │
│   ├── IoHwAb/
│   │   ├── IoHwAb.c
│   │   └── IoHwAb.h
│   │
│   ├── Types/
│   │   └── Std_Type.h
│   │
│   └── platform/
│       ├── bsp/
│       ├── debug/
│       ├── include/
│       └── spl/
│
├── main.c
├── Makefile
├── .gitignore
└── README.md
```

---

# 6. Luồng hoạt động

Khi firmware khởi động:

```text
System Startup
      │
      ▼
IoHwAb_Init()
      │
      ├── Port_Init()
      │
      ├── Adc_Init()
      │
      ├── Adc_StartGroupConversion()
      │
      └── Pwm_Init()
```

Sau khi khởi tạo, hệ thống thực hiện:

```text
ADC Sensor
    │
    ▼
Read ADC Value
    │
    ▼
Calculate Temperature
    │
    ▼
Determine Fan Duty
    │
    ▼
PWM Output
    │
    ▼
Fan
```

PWM sử dụng:

```text
TIM2_CH1 → PA0
```

---

# 7. Build Project

### Toolchain

- ARM GNU Toolchain
- GNU Make
- STM32F103C8T6
- STM32 Standard Peripheral Library

### Build

Mở terminal tại thư mục project:

```bash
make
```

Clean build:

```bash
make clean
make
```

Firmware output:

```text
build/FIRMWARE.elf
build/FIRMWARE.bin
```

---

# 8. Kết quả Build

Firmware hiện tại build thành công với kích thước:

```text
text    data    bss     dec     hex
6636      0      76    6712    1a38
```

- Flash (`text`): **6636 bytes**
- Initialized data (`data`): **0 bytes**
- RAM (`bss`): **76 bytes**
- Tổng: **6712 bytes**
