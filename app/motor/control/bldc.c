#include "bldc.h"
#include "../../debug/debug_interface.h"
#include "../platform.h"
#include "../sensors.h"

#include "platform_inc_m0.h"

typedef enum {
	HALLS_STEP1 = 0x0000001,
	HALLS_STEP2 = 0x0000003,
	HALLS_STEP3 = 0x0000002,
	HALLS_STEP4 = 0x0000006,
	HALLS_STEP5 = 0x0000004,
	HALLS_STEP6 = 0x0000005,
} HallState_t;

void BLDC_Control(u32 halls, bool direction, bool isStop) {
	if (isStop || (halls == 0x00000007) || (halls == 0x00000000)) {
		Pl_Motor_PWMA_Disable();
		Pl_Motor_PWMB_Disable();
		Pl_Motor_PWMC_Disable();
		Pl_Motor_ResetKeyAL();
		Pl_Motor_ResetKeyBL();
		Pl_Motor_ResetKeyCL();
		return;
	}

	if (direction) {
		// switch (halls) {
		//     case HALLS_STEP1: //
		//         Pl_Motor_PWMB_Disable();
		//         Pl_Motor_ResetKeyBL();
		//         Pl_Motor_SetKeyAL();
		//         Pl_Motor_PWMC_Enable();
		//         break;
		//     case HALLS_STEP2: //
		//         Pl_Motor_PWMA_Disable();
		//         Pl_Motor_ResetKeyAL();
		//         // Pl_Motor_SetKeyBL();
		//         // Pl_Motor_PWMC_Enable();
		//         break;
		//     case HALLS_STEP3: //
		//         Pl_Motor_PWMC_Disable();
		//         Pl_Motor_ResetKeyCL();
		//         Pl_Motor_SetKeyBL();
		//         Pl_Motor_PWMA_Enable();
		//         break;
		//     case HALLS_STEP4: //
		//         Pl_Motor_PWMB_Disable();
		//         Pl_Motor_ResetKeyBL();
		//         Pl_Motor_SetKeyCL();
		//         Pl_Motor_PWMA_Enable();
		//         break;
		//     case HALLS_STEP5: //
		//         Pl_Motor_PWMA_Disable();
		//         Pl_Motor_ResetKeyAL();
		//         Pl_Motor_SetKeyCL();
		//         Pl_Motor_PWMB_Enable();
		//         break;
		//     case HALLS_STEP6: //
		//         Pl_Motor_PWMC_Disable();
		//         Pl_Motor_ResetKeyCL();
		//         Pl_Motor_SetKeyAL();
		//         Pl_Motor_PWMB_Enable();
		//         break;
		//     default:
		//         break;
		// }
	} else {
		switch (halls) {
			case HALLS_STEP1:
				//
				// Pl_Motor_PWMB_Disable();
				// Pl_Motor_ResetKeyAL();
				//
				Pl_Motor_PWMC_Disable();
				Pl_Motor_ResetKeyCL();
				Pl_Motor_SetKeyBL();
				Pl_Motor_PWMA_Enable();
				break;
			case HALLS_STEP2:
				//
				// Pl_Motor_PWMC_Disable();
				// Pl_Motor_ResetKeyAL();
				//
				Pl_Motor_PWMB_Disable();
				Pl_Motor_ResetKeyBL();
				Pl_Motor_SetKeyCL();
				Pl_Motor_PWMA_Enable();
				break;
			case HALLS_STEP3:
				//
				// Pl_Motor_PWMC_Disable();
				// Pl_Motor_ResetKeyBL();
				//
				Pl_Motor_PWMA_Disable();
				Pl_Motor_ResetKeyAL();
				Pl_Motor_SetKeyCL();
				Pl_Motor_PWMB_Enable();
				break;
			case HALLS_STEP4:
				//
				// Pl_Motor_PWMA_Disable();
				// Pl_Motor_ResetKeyBL();
				//
				Pl_Motor_PWMC_Disable();
				Pl_Motor_ResetKeyCL();
				Pl_Motor_SetKeyAL();
				Pl_Motor_PWMB_Enable();
				break;
			case HALLS_STEP5:
				//
				// Pl_Motor_PWMA_Disable();
				// Pl_Motor_ResetKeyCL();
				//
				Pl_Motor_PWMB_Disable();
				Pl_Motor_ResetKeyBL();
				Pl_Motor_SetKeyAL();
				Pl_Motor_PWMC_Enable();
				break;
			case HALLS_STEP6:
				//
				// Pl_Motor_PWMB_Disable();
				// Pl_Motor_ResetKeyCL();
				//
				Pl_Motor_PWMA_Disable();
				Pl_Motor_ResetKeyAL();
				Pl_Motor_SetKeyBL();
				Pl_Motor_PWMC_Enable();
				break;
			default:
				break;
		}
	}
}
