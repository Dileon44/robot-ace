---
author: Dmitry Leonov
status: in progress
date:
  - 2026-04-09
tags:
  - FOC
  - Architecture
---
# Control cycle
ADC ISR (10 кГц, синхр. с ШИМ)
  └─ Clarke + Park (i_alpha/i_beta → id/iq)
  └─ PI ток (id, iq) → vd, vq
  └─ Inv Park + SVM → CCR1/2/3 (UDIS-атомарно)
  └─ использует θ_pred = θ_pll + ω_pll × dt_isr

TIM ISR (2 кГц)
  └─ запустить I2C DMA → AS5600 0x0C

I2C DMA callback (ISR)
  └─ angle_raw → volatile
  └─ foc_pll_run() → обновить θ_pll, ω_pll
  └─ xTaskNotifyFromISR(speed_task)

vTask_EncoderSpeed (2 кГц, разбуждается от DMA)
  └─ PI скорость → обновить g_iq_ref
  └─ каждые N итераций: xTaskNotify(position_task)

vTask_PositionControl (100–500 Гц, разбуждается от speed_task)
  └─ PI положение → обновить g_speed_ref

## as5600
- данные на частоте 800 КГц считываются за 70 мкс при частоте sysclk = 170. Из них около 5 мкс тратится на время формирования посылки и старт i2c.
- получилось выдавать по uart (2_000_000 байт/сек) посылку вида `%4u;u\r\n` за 0.3 мс. То есть на отправку такого сообщения тратится около 200 мкс. Если передавать float angle, то будет 0.6 мс.
