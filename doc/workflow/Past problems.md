---
author: Dmitry Leonov
status: in progress
date:
  - "2026-03-29"
tags:
---
# Past problems
###### 1. launch st-link
![[vscode st-link launch problem.png|500]]
Проблема в том, что `servertype: "stlink"` в cortex-debug использует `st-util` из пакета `stlink-tools` — он не установлен.
