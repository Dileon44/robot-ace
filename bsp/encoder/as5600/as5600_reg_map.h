#ifndef __AS5600_REG_MAP_H
#define __AS5600_REG_MAP_H

/* ============================================================================
 * AS5600 — 12-bit magnetic angle sensor register map
 * Datasheet: ams, v1-06, 2018-Jun-20
 * I2C address: 0x36 (7-bit, fixed)
 * ============================================================================ */

/* Configuration registers (R/W/P — OTP burnable) */
#define AS5600_REG_ZMCO         0x00u  /* Number of OTP writes, R */
#define AS5600_REG_ZPOS_H       0x01u  /* Zero position [11:8], R/W/P */
#define AS5600_REG_ZPOS_L       0x02u  /* Zero position [7:0] */
#define AS5600_REG_MPOS_H       0x03u  /* Max position [11:8], R/W/P */
#define AS5600_REG_MPOS_L       0x04u
#define AS5600_REG_MANG_H       0x05u  /* Max angle [11:8], R/W/P */
#define AS5600_REG_MANG_L       0x06u
#define AS5600_REG_CONF_H       0x07u  /* Config bits[13:8], R/W/P */
#define AS5600_REG_CONF_L       0x08u  /* Config bits[7:0] */

/* Output registers (R) */
#define AS5600_REG_STATUS       0x0Bu  /* Magnet status: MH, ML, MD */
#define AS5600_REG_RAW_ANGLE_H  0x0Cu  /* Raw angle bits[11:8] — no hysteresis */
#define AS5600_REG_RAW_ANGLE_L  0x0Du  /* Raw angle bits[7:0] */
#define AS5600_REG_ANGLE_H      0x0Eu  /* Scaled angle bits[11:8] (±10 LSB hysteresis) */
#define AS5600_REG_ANGLE_L      0x0Fu  /* Scaled angle bits[7:0] */

/* Diagnostic registers (R) */
#define AS5600_REG_AGC          0x1Au  /* Automatic gain control (0–255 at 3.3V) */
#define AS5600_REG_MAGNITUDE_H  0x1Bu  /* CORDIC magnitude bits[11:8] */
#define AS5600_REG_MAGNITUDE_L  0x1Cu  /* CORDIC magnitude bits[7:0] */

/* OTP burn register (W) */
#define AS5600_REG_BURN         0xFFu

/* STATUS register bits */
#define AS5600_STATUS_MH_BIT    (1u << 5)  /* AGC at minimum: magnet too close/strong */
#define AS5600_STATUS_ML_BIT    (1u << 4)  /* AGC at maximum: magnet too far/weak */
#define AS5600_STATUS_MD_BIT    (1u << 3)  /* Magnet detected — normal operation */

/* 12-bit angle mask */
#define AS5600_ANGLE_MASK       0x0FFFu

/* --------------------------------------------------------------------------
 * CONF register [13:0] bit fields
 * -------------------------------------------------------------------------- */
#define AS5600_CONF_PM_SHIFT    0u
#define AS5600_CONF_PM_MASK     (0x03u << AS5600_CONF_PM_SHIFT)
#define AS5600_CONF_HYST_SHIFT  2u
#define AS5600_CONF_HYST_MASK   (0x03u << AS5600_CONF_HYST_SHIFT)
#define AS5600_CONF_OUTS_SHIFT  4u
#define AS5600_CONF_OUTS_MASK   (0x03u << AS5600_CONF_OUTS_SHIFT)
#define AS5600_CONF_PWMF_SHIFT  6u
#define AS5600_CONF_PWMF_MASK   (0x03u << AS5600_CONF_PWMF_SHIFT)
#define AS5600_CONF_SF_SHIFT    8u
#define AS5600_CONF_SF_MASK     (0x03u << AS5600_CONF_SF_SHIFT)
#define AS5600_CONF_FTH_SHIFT   10u
#define AS5600_CONF_FTH_MASK    (0x07u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_WD_SHIFT    13u
#define AS5600_CONF_WD_MASK     (0x01u << AS5600_CONF_WD_SHIFT)

/* PM — Power Mode */
#define AS5600_CONF_PM_NOM      (0x00u << AS5600_CONF_PM_SHIFT)  /* Normal (continuous) */
#define AS5600_CONF_PM_LPM1     (0x01u << AS5600_CONF_PM_SHIFT)  /* Low power 1 (5 ms poll) */
#define AS5600_CONF_PM_LPM2     (0x02u << AS5600_CONF_PM_SHIFT)  /* Low power 2 (20 ms) */
#define AS5600_CONF_PM_LPM3     (0x03u << AS5600_CONF_PM_SHIFT)  /* Low power 3 (100 ms) */

/* HYST — Hysteresis */
#define AS5600_CONF_HYST_OFF    (0x00u << AS5600_CONF_HYST_SHIFT)
#define AS5600_CONF_HYST_1LSB   (0x01u << AS5600_CONF_HYST_SHIFT)
#define AS5600_CONF_HYST_2LSB   (0x02u << AS5600_CONF_HYST_SHIFT)
#define AS5600_CONF_HYST_3LSB   (0x03u << AS5600_CONF_HYST_SHIFT)

/* SF — Slow Filter (step response) */
#define AS5600_CONF_SF_16X      (0x00u << AS5600_CONF_SF_SHIFT)  /* 2.2 ms */
#define AS5600_CONF_SF_8X       (0x01u << AS5600_CONF_SF_SHIFT)  /* 1.1 ms */
#define AS5600_CONF_SF_4X       (0x02u << AS5600_CONF_SF_SHIFT)  /* 0.55 ms */
#define AS5600_CONF_SF_2X       (0x03u << AS5600_CONF_SF_SHIFT)  /* 0.286 ms — fastest */

/* FTH — Fast Filter Threshold */
#define AS5600_CONF_FTH_NONE    (0x00u << AS5600_CONF_FTH_SHIFT)  /* Slow filter only */
#define AS5600_CONF_FTH_6LSB    (0x01u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_FTH_7LSB    (0x02u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_FTH_9LSB    (0x03u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_FTH_18LSB   (0x04u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_FTH_21LSB   (0x05u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_FTH_24LSB   (0x06u << AS5600_CONF_FTH_SHIFT)
#define AS5600_CONF_FTH_10LSB   (0x07u << AS5600_CONF_FTH_SHIFT)

/* Recommended config for FOC/PMSM: PM=NOM, HYST=OFF, SF=2x, FTH=6LSB, WD=OFF
 * Results in CONF = 0x0700 (CONF_H = 0x07, CONF_L = 0x00) */
#define AS5600_CONF_FOC_VALUE \
	(AS5600_CONF_PM_NOM | AS5600_CONF_HYST_OFF | AS5600_CONF_SF_2X | AS5600_CONF_FTH_6LSB)

#endif /* __AS5600_REG_MAP_H */
