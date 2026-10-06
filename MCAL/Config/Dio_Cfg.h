/**
 * @file Dio_Cfg.h
 * @brief Configuration file for Dio module (AUTOSAR Standard)
 */
#ifndef DIO_CFG_H
#define DIO_CFG_H

/**
 * @brief List of pins for GPIO Port A,B,C,D
 * @details This macro defines the list of pins for each GPIO port.
 */
#define DIO_PORTA_PIN_LIST                    \
    X(A, 0)                                   \
    X(A, 1)                                   \
    X(A, 2) X(A, 3)                           \
        X(A, 4) X(A, 5) X(A, 6) X(A, 7)       \
            X(A, 8) X(A, 9) X(A, 10) X(A, 11) \
                X(A, 12) X(A, 13) X(A, 14) X(A, 15)

#define DIO_PORTB_PIN_LIST                    \
    X(B, 0)                                   \
    X(B, 1)                                   \
    X(B, 2) X(B, 3)                           \
        X(B, 4) X(B, 5) X(B, 6) X(B, 7)       \
            X(B, 8) X(B, 9) X(B, 10) X(B, 11) \
                X(B, 12) X(B, 13) X(B, 14) X(B, 15)

#define DIO_PORTC_PIN_LIST                    \
    X(C, 0)                                   \
    X(C, 1)                                   \
    X(C, 2) X(C, 3)                           \
        X(C, 4) X(C, 5) X(C, 6) X(C, 7)       \
            X(C, 8) X(C, 9) X(C, 10) X(C, 11) \
                X(C, 12) X(C, 13) X(C, 14) X(C, 15)

#define DIO_PORTD_PIN_LIST                    \
    X(D, 0)                                   \
    X(D, 1)                                   \
    X(D, 2) X(D, 3)                           \
        X(D, 4) X(D, 5) X(D, 6) X(D, 7)       \
            X(D, 8) X(D, 9) X(D, 10) X(D, 11) \
                X(D, 12) X(D, 13) X(D, 14) X(D, 15)

#define DIO_ALL_PIN_LIST \
    DIO_PORTA_PIN_LIST   \
    DIO_PORTB_PIN_LIST   \
    DIO_PORTC_PIN_LIST   \
    DIO_PORTD_PIN_LIST

#endif /* DIO_CFG_H */