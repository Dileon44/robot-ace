#ifndef __ENCODER_M_H
#define __ENCODER_M_H

#include "bsp_cfg.h"
#include "main.h"

/* ============================================================================
 * Abstract encoder module — vtable-based dispatch layer
 * ============================================================================ */

typedef enum {
	ENCODER_M_UNDEF = 0,
	ENCODER_M_AS5600,
	ENCODER_M_ENUM_SIZE,
} ENCODER_M_t;

/* --------------------------------------------------------------------------
 * Platform interface function pointer types
 * (implemented in encoder_m_<chip>__interface.c, call Pl_* directly)
 * -------------------------------------------------------------------------- */
typedef bool        (*EncoderM_Interface_Init_t)(void);
typedef bool        (*EncoderM_Interface_DeInit_t)(void);
typedef RET_STATE_t (*EncoderM_Interface_Read_t)(u8 wordAddr, u8* pData, u16 len);
typedef RET_STATE_t (*EncoderM_Interface_Write_t)(u8 wordAddr, const u8* pData, u16 len);

typedef struct {
	u8                          I2cAddr;
	EncoderM_Interface_Init_t   Init;
	EncoderM_Interface_DeInit_t DeInit;
	EncoderM_Interface_Read_t   Read;
	EncoderM_Interface_Write_t  Write;
} EncoderM_Interface_t;

/* --------------------------------------------------------------------------
 * Chip driver vtable function pointer types
 * (implemented in encoder_m_<chip>.c)
 * -------------------------------------------------------------------------- */
typedef bool        (*EncoderM_SetInterface_t)(EncoderM_Interface_t* pInterface);
typedef bool        (*EncoderM_Init_t)(void);
typedef bool        (*EncoderM_DeInit_t)(void);
typedef u16         (*EncoderM_GetRawAngle_t)(void);
typedef u16         (*EncoderM_GetAngle_t)(void);
typedef float       (*EncoderM_GetAngleDeg_t)(void);
typedef u8          (*EncoderM_GetStatus_t)(void);
typedef bool        (*EncoderM_IsMagnetDetected_t)(void);
typedef u8          (*EncoderM_GetAGC_t)(void);
typedef u16         (*EncoderM_GetMagnitude_t)(void);

typedef struct {
	ENCODER_M_t                 Module;
	char*                       Name;
	EncoderM_SetInterface_t     SetInterface;
	EncoderM_Init_t             Init;
	EncoderM_DeInit_t           DeInit;
	EncoderM_GetRawAngle_t      GetRawAngle;
	EncoderM_GetAngle_t         GetAngle;
	EncoderM_GetAngleDeg_t      GetAngleDeg;
	EncoderM_GetStatus_t        GetStatus;
	EncoderM_IsMagnetDetected_t IsMagnetDetected;
	EncoderM_GetAGC_t           GetAGC;
	EncoderM_GetMagnitude_t     GetMagnitude;
} EncoderM_t;

/* --------------------------------------------------------------------------
 * Module info — passed to EncoderM_Init() from BSP
 * -------------------------------------------------------------------------- */
typedef struct {
	EncoderM_Interface_t* InterfacePtr;
	EncoderM_t*           ModulePtr;
} EncoderM_Info_t;

/* --------------------------------------------------------------------------
 * Public BSP-level API
 * -------------------------------------------------------------------------- */
RET_STATE_t EncoderM_Init(EncoderM_Info_t* pInfo);
RET_STATE_t EncoderM_DeInit(void);
u16         EncoderM_GetRawAngle(void);
u16         EncoderM_GetAngle(void);
float       EncoderM_GetAngleDeg(void);
u8          EncoderM_GetStatus(void);
bool        EncoderM_IsMagnetDetected(void);
u8          EncoderM_GetAGC(void);
u16         EncoderM_GetMagnitude(void);

#endif /* __ENCODER_M_H */
