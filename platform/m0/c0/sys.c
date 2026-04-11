#include "sys.h"

#define ARM_CM_DEMCR	  (*(u32*)0xE000EDFC)
#define ARM_CM_DWT_CTRL	  (*(u32*)0xE0001000)
#define ARM_CM_DWT_CYCCNT (*(u32*)0xE0001004)

static const char* ResetSrcList[] = {"UNKNOWN", "IWDG", "LOW_POWER", "PIN", "POR",
									 "SOFT",	"WWDG", "BOR",		 "SBF"};

const char* Sys_ResetFlag_GetStr(void) {
	return ResetSrcList[Sys_ResetFlag_Get()];
}

SYS_RESET_FLAG_t Sys_ResetFlag_Get(void) {
	SYS_RESET_FLAG_t rstFlag = SYS_RESET_FLAG_UNKNOWN;

	if (LL_RCC_IsActiveFlag_IWDGRST())
		rstFlag = SYS_RESET_FLAG_IWDG;
	else if (LL_RCC_IsActiveFlag_LPWRRST())
		rstFlag = SYS_RESET_FLAG_LP;
	else if (LL_RCC_IsActiveFlag_PINRST())
		rstFlag = SYS_RESET_FLAG_PIN;
	else if (LL_RCC_IsActiveFlag_SFTRST())
		rstFlag = SYS_RESET_FLAG_SOFT;
	else if (LL_RCC_IsActiveFlag_WWDGRST())
		rstFlag = SYS_RESET_FLAG_WWDG;
	else if (LL_RCC_IsActiveFlag_BORRST())
		rstFlag = SYS_RESET_FLAG_BOR;
	else if (LL_RCC_IsActiveFlag_OBLRST())
		rstFlag = SYS_RESET_FLAG_OBL;

	return rstFlag;
}

void Sys_ResetFlag_Clear(void) {
	LL_RCC_ClearResetFlags();
	LL_PWR_ClearFlag_SB();
}

static void toggle_lse_clock(void) {
	LL_RCC_LSE_Disable();
	LL_RCC_LSE_Enable();

	volatile u32 cnt = 0;
	while (!LL_RCC_LSE_IsReady() && cnt < 1000000)	//1000000 cycles is 180ms at 100MHz
		cnt++;
}

u32 Sys_LSE_IsReadyAndClkSrc(void) {
	return LL_RCC_GetRTCClockSource() == LL_RCC_RTC_CLKSOURCE_LSE && LL_RCC_LSE_IsReady();
}

u32 Sys_LSI_IsReadyAndClkSrc(void) {
	return LL_RCC_GetRTCClockSource() == LL_RCC_RTC_CLKSOURCE_LSI && LL_RCC_LSI_IsReady();
}

void Sys_EnableBkpAccess(void) {
	if (!LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_PWR))
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	if (!LL_PWR_IsEnabledBkUpAccess())
		LL_PWR_EnableBkUpAccess();
}

void Sys_RealTimeClock_Config(void) {
	Sys_EnableBkpAccess();

#ifdef PLATFORM_M0_CLOCK_RTC_LSI
	if (LL_RCC_LSI_IsReady())
		LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);

	if (Sys_LSI_IsReadyAndClkSrc())
		return;
#endif /* PLATFORM_M0_CLOCK_RTC_LSI */

#ifdef PLATFORM_M0_CLOCK_RTC_LSE
	for (u32 attempt = 0; attempt < 3; attempt++) {
		if (Sys_LSE_IsReadyAndClkSrc())
			break;

		if (LL_RCC_LSE_IsReady()) {
			LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
			volatile u32 cnt = 0;
			while (cnt < 100000)  //1000000 cycles is 18ms at 100MHz
				cnt++;

			if (Sys_LSE_IsReadyAndClkSrc())
				break;
		}
		/*volatile int a, q, w;
		a = LL_RCC_GetRTCClockSource();
		q = LL_RCC_LSE_IsReady();
		w = LL_RCC_LSI_IsReady();*/

		//we can't change clock source on the fly (if it's HSI for example) without BackupDomainReset(), so just make it
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();

		toggle_lse_clock();
	}
#endif /* PLATFORM_M0_CLOCK_RTC_LSE */
}

void Sys_MainClock_Config(void) {

	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	LL_PWR_EnableRange1BoostMode();

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
	while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
	}

	LL_PWR_EnableRange1BoostMode();
	LL_RCC_HSE_Enable();
	while (LL_RCC_HSE_IsReady() != 1) {
	}

	LL_RCC_LSI_Enable();
	while (LL_RCC_LSI_IsReady() != 1) {
	}

	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 85, LL_RCC_PLLR_DIV_2);
	LL_RCC_PLL_EnableDomain_SYS();
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() != 1) {
	}

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
	/* Wait till System clock is ready */
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
	}

	/* Insure 1us transition state at intermediate medium speed clock*/
	for (__IO uint32_t i = (170 >> 1); i != 0; i--)
		;

	/* Set AHB prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	SystemCoreClockUpdate();
	LL_Init1msTick(SystemCoreClock);
	SysTick_Config(SystemCoreClock / 1000);

	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
}

u32* Sys_UID_GetStrAndPtr(char* pDst) {
	u32* UIDptr = (u32*)UID_BASE;
	if (pDst)
		sprintf(pDst, "%08x-%08x-%08x", (int)UIDptr[0], (int)UIDptr[1], (int)UIDptr[2]);

	return UIDptr;
}

void Sys_CPU_GetStrAndPtr(char* pDst) {
	if (pDst)
		sprintf(pDst, "%04x/%04x", LL_DBGMCU_GetRevisionID(), LL_DBGMCU_GetDeviceID());
}

u32 Sys_MCU_GetFlashSize(void) {
	return LL_GetFlashSize();
}

void Sys_StandbyMode_Enter(void) {
	LL_PWR_ClearFlag_WU();

	LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
	LL_LPM_EnableDeepSleep();
	__WFI();
}

bool Sys_CounterCPU_Init(void) {
	if (ARM_CM_DWT_CTRL != 0) {						 // See if DWT is available
		ARM_CM_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	 // 1. Enable trace first
		ARM_CM_DWT_CYCCNT = 0;						 // 2. Reset counter
		ARM_CM_DWT_CTRL |= DWT_CTRL_CYCCNTENA_Msk;	 // 3. Enable CYCCNT
	}

	return true;
}

u32 Sys_CounterCPU_Get(void) {
	return ARM_CM_DWT_CYCCNT;
}

void Sys_MCU_Reset(void) {
	NVIC_SystemReset();
}

void Sys_NVIC_SetPrioEnable(IRQn_Type irq, u16 prio) {
	NVIC_SetPriority(irq, prio);
	NVIC_EnableIRQ(irq);
}

void Sys_NVIC_Disable(IRQn_Type irq) {
	NVIC_SetPriority(irq, 0);
	NVIC_DisableIRQ(irq);
}
