#include "gpio.h"
#include "sys.h"

#define USART_DEBUG_TX_RX_PORT GPIOB
#define USART_DEBUG_TX_PIN	   LL_GPIO_PIN_3
#define USART_DEBUG_RX_PIN	   LL_GPIO_PIN_4
#define USART_DEBUG_TX_RX_AF   LL_GPIO_AF_7

#define MOTOR_HIGH_KEYS_PORT GPIOA
#define MOTOR_AH_PIN		 LL_GPIO_PIN_8
#define MOTOR_BH_PIN		 LL_GPIO_PIN_9
#define MOTOR_CH_PIN		 LL_GPIO_PIN_10
#define MOTOR_AH_PIN_AF		 LL_GPIO_AF_6
#define MOTOR_BH_PIN_AF		 LL_GPIO_AF_6
#define MOTOR_CH_PIN_AF		 LL_GPIO_AF_6

#define MOTOR_LOW_KEYS_PORT GPIOB
#define MOTOR_AL_PIN		LL_GPIO_PIN_13
#define MOTOR_BL_PIN		LL_GPIO_PIN_14
#define MOTOR_CL_PIN		LL_GPIO_PIN_15

#define GPIO_HALL1_PORT				GPIOA
#define GPIO_HALL1_PIN				LL_GPIO_PIN_2
#define GPIO_HALL1_EXTI_LINE		LL_EXTI_LINE_2
#define GPIO_HALL1_IRQ				EXTI2_IRQn
#define GPIO_HALL1_IRQ_HDL			EXTI2_IRQHandler
#define GPIO_HALL1_IS_ACTIVE_FLAG() LL_EXTI_IsActiveFlag_0_31(GPIO_HALL1_EXTI_LINE)
#define GPIO_HALL1_CLEAR_FLAG()		LL_EXTI_ClearFlag_0_31(GPIO_HALL1_EXTI_LINE)
#define GPIO_HALL1_SYSCFG_EXTI_PORT LL_SYSCFG_EXTI_PORTA
#define GPIO_HALL1_SYSCFG_EXTI_LINE LL_SYSCFG_EXTI_LINE2
#define GPIO_HALL1_IRQ_CALL()		LL_EXTI_GenerateSWI_0_31(GPIO_HALL1_EXTI_LINE)

#define GPIO_HALL2_PORT				GPIOA
#define GPIO_HALL2_PIN				LL_GPIO_PIN_3
#define GPIO_HALL2_EXTI_LINE		LL_EXTI_LINE_3
#define GPIO_HALL2_IRQ				EXTI3_IRQn
#define GPIO_HALL2_IRQ_HDL			EXTI3_IRQHandler
#define GPIO_HALL2_IS_ACTIVE_FLAG() LL_EXTI_IsActiveFlag_0_31(GPIO_HALL2_EXTI_LINE)
#define GPIO_HALL2_CLEAR_FLAG()		LL_EXTI_ClearFlag_0_31(GPIO_HALL2_EXTI_LINE)
#define GPIO_HALL2_SYSCFG_EXTI_PORT LL_SYSCFG_EXTI_PORTA
#define GPIO_HALL2_SYSCFG_EXTI_LINE LL_SYSCFG_EXTI_LINE3

#define GPIO_HALL3_PORT				GPIOA
#define GPIO_HALL3_PIN				LL_GPIO_PIN_4
#define GPIO_HALL3_EXTI_LINE		LL_EXTI_LINE_4
#define GPIO_HALL3_IRQ				EXTI4_IRQn
#define GPIO_HALL3_IRQ_HDL			EXTI4_IRQHandler
#define GPIO_HALL3_IS_ACTIVE_FLAG() LL_EXTI_IsActiveFlag_0_31(GPIO_HALL3_EXTI_LINE)
#define GPIO_HALL3_CLEAR_FLAG()		LL_EXTI_ClearFlag_0_31(GPIO_HALL3_EXTI_LINE)
#define GPIO_HALL3_SYSCFG_EXTI_PORT LL_SYSCFG_EXTI_PORTA
#define GPIO_HALL3_SYSCFG_EXTI_LINE LL_SYSCFG_EXTI_LINE4

#define GPIO_SENS_THROTTLE_PORT GPIOA
#define GPIO_SENS_THROTTLE_PIN	LL_GPIO_PIN_0

#define GPIO_SENS_BRAKE_LEFT_PORT GPIOA
#define GPIO_SENS_BRAKE_LEFT_PIN  LL_GPIO_PIN_1

#define GPIO_SENS_CUR_A_PORT GPIOB	// cur_1
#define GPIO_SENS_CUR_A_PIN	 LL_GPIO_PIN_11

#define GPIO_SENS_CUR_B_PORT GPIOB	// cur_2
#define GPIO_SENS_CUR_B_PIN	 LL_GPIO_PIN_2

#define GPIO_SENS_CUR_C_PORT GPIOC	// cur_3
#define GPIO_SENS_CUR_C_PIN	 LL_GPIO_PIN_4

// #define GPIO_SENS_BRAKE_RIGHT_PIN   GPIOA
// #define GPIO_SENS_BRAKE_RIGHT_PORT  LL_GPIO_PIN_6

// #define GPIO_SENS_TEMP_MOTOR_PIN    GPIOA
// #define GPIO_SENS_TEMP_MOTOR_PORT   LL_GPIO_PIN_7

// #define GPIO_SENS_TEMP_MOSFET_PIN   GPIOC
// #define GPIO_SENS_TEMP_MOSFET_PORT  LL_GPIO_PIN_4

#define GPIO_LED_PORT GPIOC
#define GPIO_LED_PIN  LL_GPIO_PIN_6

// static Pl_Motor_ClbkHall_t GPIO_Motor_HallClbk = Pl_Stub_CommonClbk;
static Pl_Motor_ClbkHall_t GPIO_Motor_HallClbk = Pl_Stub_HallClbk;
typedef void (*GPIO_Action_FuncPtr_t)(GPIO_TypeDef*, u32);

GPIO_Action_FuncPtr_t GPIO_FuncList[] = {LL_GPIO_ResetOutputPin, LL_GPIO_SetOutputPin,
										 LL_GPIO_TogglePin};

void GPIO_USART_Debug_Tx_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= USART_DEBUG_TX_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate			= USART_DEBUG_TX_RX_AF;
	LL_GPIO_Init(USART_DEBUG_TX_RX_PORT, &GPIO_InitStruct);
}

void GPIO_USART_Debug_Rx_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= USART_DEBUG_RX_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate			= USART_DEBUG_TX_RX_AF;
	LL_GPIO_Init(USART_DEBUG_TX_RX_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_AH_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= MOTOR_AH_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_DOWN;
	GPIO_InitStruct.Alternate			= MOTOR_AH_PIN_AF;
	LL_GPIO_Init(MOTOR_HIGH_KEYS_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_BH_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= MOTOR_BH_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_DOWN;
	GPIO_InitStruct.Alternate			= MOTOR_BH_PIN_AF;
	LL_GPIO_Init(MOTOR_HIGH_KEYS_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_CH_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= MOTOR_CH_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_DOWN;
	GPIO_InitStruct.Alternate			= MOTOR_CH_PIN_AF;
	LL_GPIO_Init(MOTOR_HIGH_KEYS_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_AL_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= MOTOR_AL_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_GPIO_ResetOutputPin(MOTOR_LOW_KEYS_PORT, MOTOR_AL_PIN);
	LL_GPIO_Init(MOTOR_LOW_KEYS_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_BL_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= MOTOR_BL_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_GPIO_ResetOutputPin(MOTOR_LOW_KEYS_PORT, MOTOR_BL_PIN);
	LL_GPIO_Init(MOTOR_LOW_KEYS_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_CL_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= MOTOR_CL_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_GPIO_ResetOutputPin(MOTOR_LOW_KEYS_PORT, MOTOR_CL_PIN);
	LL_GPIO_Init(MOTOR_LOW_KEYS_PORT, &GPIO_InitStruct);
}

void GPIO_Motor_Hall1_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_HALL1_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	EXTI_InitStruct.Line_0_31			= GPIO_HALL1_EXTI_LINE;
	EXTI_InitStruct.LineCommand			= ENABLE;
	EXTI_InitStruct.Mode				= LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger				= LL_EXTI_TRIGGER_RISING_FALLING;

	LL_GPIO_Init(GPIO_HALL1_PORT, &GPIO_InitStruct);
	LL_EXTI_Init(&EXTI_InitStruct);
	LL_SYSCFG_SetEXTISource(GPIO_HALL1_SYSCFG_EXTI_PORT, GPIO_HALL1_SYSCFG_EXTI_LINE);
}

void GPIO_Motor_Hall2_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_HALL2_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	EXTI_InitStruct.Line_0_31			= GPIO_HALL2_EXTI_LINE;
	EXTI_InitStruct.LineCommand			= ENABLE;
	EXTI_InitStruct.Mode				= LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger				= LL_EXTI_TRIGGER_RISING_FALLING;

	LL_GPIO_Init(GPIO_HALL2_PORT, &GPIO_InitStruct);
	LL_EXTI_Init(&EXTI_InitStruct);
	LL_SYSCFG_SetEXTISource(GPIO_HALL2_SYSCFG_EXTI_PORT, GPIO_HALL2_SYSCFG_EXTI_LINE);
}

void GPIO_Motor_Hall3_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_HALL3_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	EXTI_InitStruct.Line_0_31			= GPIO_HALL3_EXTI_LINE;
	EXTI_InitStruct.LineCommand			= ENABLE;
	EXTI_InitStruct.Mode				= LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger				= LL_EXTI_TRIGGER_RISING_FALLING;

	LL_GPIO_Init(GPIO_HALL3_PORT, &GPIO_InitStruct);
	LL_EXTI_Init(&EXTI_InitStruct);
	LL_SYSCFG_SetEXTISource(GPIO_HALL3_SYSCFG_EXTI_PORT, GPIO_HALL3_SYSCFG_EXTI_LINE);
}

void GPIO_Motor_Hall_Init(Pl_Motor_ClbkHall_t pHallClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(GPIO_Motor_HallClbk, pHallClbk);

	GPIO_Motor_Hall1_Init();
	GPIO_Motor_Hall2_Init();
	GPIO_Motor_Hall3_Init();
}

// void GPIO_Motor_Throttle_Init(void) {
//     LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };

//     GPIO_InitStruct.Pin = ANALOG_THROTTLE_PIN;
//     GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
//     GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

//     LL_GPIO_Init(ANALOG_THROTTLE_PORT, &GPIO_InitStruct);
// }

u32 GPIO_Motor_GetStateHalls(void) {
	u32 pinState = 0;

	if (LL_GPIO_IsInputPinSet(GPIO_HALL1_PORT, GPIO_HALL1_PIN)) {
		pinState |= 0x00000001;
	}

	if (LL_GPIO_IsInputPinSet(GPIO_HALL2_PORT, GPIO_HALL2_PIN)) {
		pinState |= 0x00000002;
	}

	if (LL_GPIO_IsInputPinSet(GPIO_HALL3_PORT, GPIO_HALL3_PIN)) {
		pinState |= 0x00000004;
	}

	return pinState;
}

void GPIO_Motor_SetKeyAL(GPIO_ACTION_t action) {
	GPIO_FuncList[action](MOTOR_LOW_KEYS_PORT, MOTOR_AL_PIN);
}

void GPIO_Motor_SetKeyBL(GPIO_ACTION_t action) {
	GPIO_FuncList[action](MOTOR_LOW_KEYS_PORT, MOTOR_BL_PIN);
}

void GPIO_Motor_SetKeyCL(GPIO_ACTION_t action) {
	GPIO_FuncList[action](MOTOR_LOW_KEYS_PORT, MOTOR_CL_PIN);
}

void GPIO_Motor_HallIrqEnable(void) {
	Sys_NVIC_SetPrioEnable(GPIO_HALL1_IRQ, NVIC_IRQ_PRIO_HALL);
	Sys_NVIC_SetPrioEnable(GPIO_HALL2_IRQ, NVIC_IRQ_PRIO_HALL);
	Sys_NVIC_SetPrioEnable(GPIO_HALL3_IRQ, NVIC_IRQ_PRIO_HALL);
}

void GPIO_Hall1_CallIrq(void) {
	if (!GPIO_HALL1_IS_ACTIVE_FLAG()) {
		GPIO_HALL1_IRQ_CALL();
	}
}

void GPIO_HALL1_IRQ_HDL(void) {
	if (GPIO_HALL1_IS_ACTIVE_FLAG()) {
		GPIO_Motor_HallClbk(1);
		GPIO_HALL1_CLEAR_FLAG();
	}
}

void GPIO_HALL2_IRQ_HDL(void) {
	if (GPIO_HALL2_IS_ACTIVE_FLAG()) {
		GPIO_Motor_HallClbk(2);
		GPIO_HALL2_CLEAR_FLAG();
	}
}

void GPIO_HALL3_IRQ_HDL(void) {
	if (GPIO_HALL3_IS_ACTIVE_FLAG()) {
		GPIO_Motor_HallClbk(3);
		GPIO_HALL3_CLEAR_FLAG();
	}
}

void GPIO_SensThrottle_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_SENS_THROTTLE_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIO_SENS_THROTTLE_PORT, &GPIO_InitStruct);
}

void GPIO_SensBrakeLeft_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_SENS_BRAKE_LEFT_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIO_SENS_BRAKE_LEFT_PORT, &GPIO_InitStruct);
}

// void GPIO_SensBrakeRight_Init(void) {
//     LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//     GPIO_InitStruct.Pin  = GPIO_SENS_BRAKE_RIGHT_PIN;
//     GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
//     GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//     LL_GPIO_Init(GPIO_SENS_BRAKE_RIGHT_PORT, &GPIO_InitStruct);
// }

// void GPIO_SensTempMotor_Init(void) {
//     LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//     GPIO_InitStruct.Pin  = GPIO_SENS_TEMP_MOTOR_PIN;
//     GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
//     GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//     LL_GPIO_Init(GPIO_SENS_TEMP_MOTOR_PORT, &GPIO_InitStruct);
// }

// void GPIO_SensTempMosfet_Init(void) {
//     LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//     GPIO_InitStruct.Pin  = GPIO_SENS_TEMP_MOSFET_PIN;
//     GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
//     GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//     LL_GPIO_Init(GPIO_SENS_TEMP_MOSFET_PORT, &GPIO_InitStruct);
// }

void GPIO_ADCLowFreq_Init(void) {
	GPIO_SensThrottle_Init();
	GPIO_SensBrakeLeft_Init();
	// GPIO_SensBrakeRight_Init();
	// GPIO_SensTempMotor_Init();
	// GPIO_SensTempMosfet_Init();
}

void GPIO_SensCurA_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_SENS_CUR_A_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIO_SENS_CUR_A_PORT, &GPIO_InitStruct);
}

void GPIO_SensCurB_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_SENS_CUR_B_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIO_SENS_CUR_B_PORT, &GPIO_InitStruct);
}

void GPIO_SensCurC_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_SENS_CUR_C_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIO_SENS_CUR_C_PORT, &GPIO_InitStruct);
}

void GPIO_ADCHighFreq_Init(void) {
	GPIO_SensCurA_Init();
	GPIO_SensCurB_Init();
	GPIO_SensCurC_Init();
}

void GPIO_LedDebug_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin					= GPIO_LED_PIN;
	GPIO_InitStruct.Mode				= LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed				= LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType			= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull				= LL_GPIO_PULL_NO;

	LL_GPIO_SetOutputPin(GPIO_LED_PORT, GPIO_LED_PIN);
	LL_GPIO_Init(GPIO_LED_PORT, &GPIO_InitStruct);
}

__INLINE void GPIO_Led_Set(void) {
	LL_GPIO_SetOutputPin(GPIO_LED_PORT, GPIO_LED_PIN);
}

__INLINE void GPIO_Led_Reset(void) {
	LL_GPIO_ResetOutputPin(GPIO_LED_PORT, GPIO_LED_PIN);
}

__INLINE void GPIO_Led_Toggle(void) {
	LL_GPIO_TogglePin(GPIO_LED_PORT, GPIO_LED_PIN);
}