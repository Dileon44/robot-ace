#include "gpio.h"
#include "sys.h"

#define DEBUG_TX_RX_PORT GPIOB
#define DEBUG_TX_PIN	 LL_GPIO_PIN_3
#define DEBUG_RX_PIN	 LL_GPIO_PIN_4
#define DEBUG_TX_RX_AF	 LL_GPIO_AF_7
//
#define LED_PORT		 GPIOC
#define LED_PIN			 LL_GPIO_PIN_6
//
#define ENCODER_I2C_PORT GPIOB
#define ENCODER_SDA_PIN	 LL_GPIO_PIN_7
#define ENCODER_SCL_PIN	 LL_GPIO_PIN_8
#define ENCODER_I2C_AF	 LL_GPIO_AF_4
#define ENCODER_DIR_PORT GPIOB
#define ENCODER_DIR_PIN	 LL_GPIO_PIN_6

typedef void (*GPIO_Action_FuncPtr_t)(GPIO_TypeDef*, u32);

GPIO_Action_FuncPtr_t GPIO_FuncList[] = {
	LL_GPIO_ResetOutputPin,
	LL_GPIO_SetOutputPin,
	LL_GPIO_TogglePin,
};

void GPIO_Debug_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_GPIO_StructInit(&GPIO_InitStruct);

	GPIO_InitStruct.Pin		   = DEBUG_TX_PIN;
	GPIO_InitStruct.Mode	   = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed	   = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull	   = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate  = DEBUG_TX_RX_AF;
	LL_GPIO_Init(DEBUG_TX_RX_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = DEBUG_RX_PIN;
	LL_GPIO_Init(DEBUG_TX_RX_PORT, &GPIO_InitStruct);
}

void GPIO_Led_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_GPIO_StructInit(&GPIO_InitStruct);

	GPIO_InitStruct.Pin		   = LED_PIN;
	GPIO_InitStruct.Mode	   = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed	   = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull	   = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

__INLINE void GPIO_Led_Set(void) {
	LL_GPIO_SetOutputPin(LED_PORT, LED_PIN);
}

__INLINE void GPIO_Led_Reset(void) {
	LL_GPIO_ResetOutputPin(LED_PORT, LED_PIN);
}

__INLINE void GPIO_Led_Toggle(void) {
	LL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

void GPIO_Encoder_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_GPIO_StructInit(&GPIO_InitStruct);

	GPIO_InitStruct.Pin		   = ENCODER_SDA_PIN | ENCODER_SCL_PIN;
	GPIO_InitStruct.Mode	   = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed	   = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull	   = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate  = ENCODER_I2C_AF;
	LL_GPIO_Init(ENCODER_I2C_PORT, &GPIO_InitStruct);

	// DIR pin (PB6) — output push-pull, initial LOW (clockwise)
	LL_GPIO_ResetOutputPin(ENCODER_DIR_PORT, ENCODER_DIR_PIN);
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin		   = ENCODER_DIR_PIN;
	GPIO_InitStruct.Mode	   = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed	   = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull	   = LL_GPIO_PULL_NO;
	LL_GPIO_Init(ENCODER_DIR_PORT, &GPIO_InitStruct);
}

void GPIO_Encoder_DeInit(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_GPIO_StructInit(&GPIO_InitStruct);

	GPIO_InitStruct.Mode	   = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Speed	   = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull	   = LL_GPIO_PULL_NO;

	GPIO_InitStruct.Pin = ENCODER_SDA_PIN | ENCODER_SCL_PIN;
	LL_GPIO_Init(ENCODER_I2C_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = ENCODER_DIR_PIN;
	LL_GPIO_Init(ENCODER_DIR_PORT, &GPIO_InitStruct);
}

void GPIO_Encoder_Dir_Set(bool clockwise) {
	if (clockwise) {
		LL_GPIO_ResetOutputPin(ENCODER_DIR_PORT, ENCODER_DIR_PIN);
	} else {
		LL_GPIO_SetOutputPin(ENCODER_DIR_PORT, ENCODER_DIR_PIN);
	}
}