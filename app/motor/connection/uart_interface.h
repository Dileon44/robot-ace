// #ifndef __UART_INTERFACE_H
// #define __UART_INTERFACE_H

// #include "main.h"

// typedef enum {
// 	UART_INT_CTRL_SRC_UART,
// 	UART_INT_CTRL_SRC_THROTTLE,
// } UART_INT_CTRL_SRC_t;

// typedef struct {
//     struct Uart_Int_Buttons_t {
//         bool isStop;
//         bool isStart;
//     } Buttons;

//     struct Uart_Int_SendData_t {
//         bool isSend;
//     } UartSendData;

//     UART_INT_CTRL_SRC_t CtrlSrc;

//     struct Uart_Int_ShapeIn_t {
//         struct Uart_Int_ShapeInStep_t {
//             float speedDesired;
//         } Step;

//         struct Uart_Int_ShapeInSin_t {
//             float ampl;
//             float freq;
//         } Sin;

//         struct Uart_Int_ShapeInMeander_t {
//             float ampl;
//             float freq;
//         } Meander;

//         struct Uart_Int_ShapeInSawtooth_t {
//             float ampl;
//             float freq;
//         } Sawtooth;
//     } ShapeIn;

//     struct Uart_Int_PrintParams_t {
//         bool  speedDesired_kmph;
//         bool  speedRaw_kmph;
//         bool  speedFilt_kmph;
//         bool  outPI_kmph;
//         bool  rpm;
//     } PrintParams;
// } Uart_Int_RxData_t;

// // #define UART_INTERFACE_STOP_CMD_SIZE            5
// // #define UART_INTERFACE_START_CMD_SIZE           6

// // #define UART_INTERFACE_UART_STATE_CMD_SIZE      5

// // #define UART_INTERFACE_CTRL_SRC_CMD_SIZE        8
// // #define UART_INTERFACE_CTRL_SRC_STATE_SIZE      9

// // #define UART_INTERFACE_SHAPE_CMD_SIZE           8

// // #define UART_INTERFACE_SHAPE_STEP_CMD_SIZE      5
// // #define UART_INTERFACE_SHAPE_STEP_SPDES_SIZE    10

// // #define UART_INTERFACE_SHAPE_SIN_CMD_SIZE       4
// // #define UART_INTERFACE_SHAPE_SIN_AMPL_SIZE      8
// // #define UART_INTERFACE_SHAPE_SIN_FREQ_SIZE      8

// // #define UART_INTERFACE_SHAPE_MNDR_CMD_SIZE      8
// // #define UART_INTERFACE_SHAPE_MNDR_AMPL_SIZE     9
// // #define UART_INTERFACE_SHAPE_MNDR_FREQ_SIZE     9

// // #define UART_INTERFACE_SHAPE_SAW_CMD_SIZE       9
// // #define UART_INTERFACE_SHAPE_SAW_AMPL_SIZE      8
// // #define UART_INTERFACE_SHAPE_SAW_FREQ_SIZE      8

// // #define UART_INTERFACE_PRINT_SPDES_CMD_SIZE     6
// // #define UART_INTERFACE_PRINT_SPRAW_CMD_SIZE     6
// // #define UART_INTERFACE_PRINT_SPFILT_CMD_SIZE    7
// // #define UART_INTERFACE_PRINT_OUTPI_CMD_SIZE     3
// // #define UART_INTERFACE_PRINT_RPM_CMD_SIZE       4

// // typedef struct {
// //     struct Uart_Int_ButtonsData_t {
// //         char stopCmd [UART_INTERFACE_STOP_CMD_SIZE];
// //         char startCmd [UART_INTERFACE_START_CMD_SIZE];
// //     } ButtonsData;

// //     struct Uart_Int_SendData_t {
// //         char uartCmd [UART_INTERFACE_UART_STATE_CMD_SIZE];
// //         bool state;
// //     } UartSendData;

// //     struct Uart_Int_CtrlSrcData_t {
// //         char cmd [UART_INTERFACE_CTRL_SRC_CMD_SIZE];
// //         char state [UART_INTERFACE_CTRL_SRC_STATE_SIZE];
// //     } CtrlSrcData;

// //     struct Uart_Int_ShapeInData_t {
// //         char cmd [UART_INTERFACE_SHAPE_CMD_SIZE];

// //         struct CtrlSrcStepData_t {
// //             char cmd [UART_INTERFACE_SHAPE_STEP_CMD_SIZE];
// //             char speedDesiredCmd [UART_INTERFACE_SHAPE_STEP_SPDES_SIZE];
// //             float speedDesired;
// //         } Step;

// //         struct CtrlSrcSinData_t {
// //             char cmd [UART_INTERFACE_SHAPE_SIN_CMD_SIZE];
// //             char amplCmd [UART_INTERFACE_SHAPE_SIN_AMPL_SIZE];
// //             char freqCmd [UART_INTERFACE_SHAPE_SIN_FREQ_SIZE];
// //             float ampl;
// //             float freq;
// //         } Sin;

// //         struct CtrlSrcMeanderData_t {
// //             char cmd [UART_INTERFACE_SHAPE_MNDR_CMD_SIZE];
// //             char ampl [UART_INTERFACE_SHAPE_MNDR_AMPL_SIZE];
// //             char freq [UART_INTERFACE_SHAPE_MNDR_FREQ_SIZE];
// //             float ampl;
// //             float freq;
// //         } Meander;

// //         struct CtrlSrcSawtoothData_t {
// //             char cmd [UART_INTERFACE_SHAPE_SAW_CMD_SIZE];
// //             char ampl [UART_INTERFACE_SHAPE_SAW_AMPL_SIZE];
// //             char freq [UART_INTERFACE_SHAPE_SAW_FREQ_SIZE];
// //             float ampl;
// //             float freq;
// //         } Sawtooth;
// //     } ShapeInData;

// //     struct Uart_Int_PrintParamsData_t {
// //         char cmdSpeedDesired_kmph [UART_INTERFACE_PRINT_SPDES_CMD_SIZE];
// //         char cmdSpeedRaw_kmph [UART_INTERFACE_PRINT_SPRAW_CMD_SIZE];
// //         char cmdSpeedFilt_kmph [UART_INTERFACE_PRINT_SPFILT_CMD_SIZE];
// //         char cmdOutPI_kmph [UART_INTERFACE_PRINT_OUTPI_CMD_SIZE];
// //         char cmdRpm [UART_INTERFACE_PRINT_RPM_CMD_SIZE];
// //         bool  speedDesired_kmph;
// //         bool  speedRaw_kmph;
// //         bool  speedFilt_kmph;
// //         bool  outPI_kmph;
// //         bool  rpm;
// //     } PrintParamsData;
// // } Uart_Int_RxData_t;

// #endif /* __UART_INTERFACE_H */