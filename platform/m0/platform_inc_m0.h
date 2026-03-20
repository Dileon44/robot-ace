#ifndef __PLATFORM_INC_M0_H
#define __PLATFORM_INC_M0_H

#include "stm32g4xx.h"
#include "platform_int_cfg_m0.h"

#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_comp.h"
#include "stm32g4xx_ll_cordic.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_crc.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_dmamux.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_fmac.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_hrtim.h"
#include "stm32g4xx_ll_i2c.h"
#include "stm32g4xx_ll_iwdg.h"
#include "stm32g4xx_ll_lptim.h"
#include "stm32g4xx_ll_lpuart.h"
#include "stm32g4xx_ll_opamp.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_rng.h"
#include "stm32g4xx_ll_rtc.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_ucpd.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_usb.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_wwdg.h"

// #include "stm32g4xx_ll_rcc.c"

//------------------------------------------------------------------------------------------------------------
#if !defined(PLATFORM_M0_CLOCK_RTC_LSE) && !defined(PLATFORM_M0_CLOCK_RTC_LSI)
/** @brief Select the generator for RTC */
#define PLATFORM_M0_CLOCK_RTC_LSE
#endif /* PLATFORM_M0_CLOCK_RTC_LSE */
//------------------------------------------------------------------------------------------------------------

#endif /* __PLATFORM_INC_M0_H */
