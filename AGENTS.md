# robot-ace — Agent Instructions

Embedded firmware for a PMSM motor controller ACE (actuator control electronics). This unit will control the electronic drive unit in the robot's joint.

When writing the code, you need to keep in mind that this program will control power transistors on an electronic board, so you need to pay attention to safety - think about through currents in three-phase bridge systems. And you also need to understand that in any emergency situation, it is necessary to close the power transistors in order to save the board and the MCU.

Target: STM32G431RB (Cortex-M4F, 128 KB Flash, 32 KB RAM). RTOS: FreeRTOS V11.2.0.
Language: C17. No dynamic memory allocation in application code.

---

## Build Commands

The full build chain is: **VS Code task → `scripts/builder.sh` → `scripts/fw_builder.py` → CMake**.

`builder.sh` handles OS detection and virtual environment setup (creates `.venv` on first run,
installs `scripts/requirements.txt`), then delegates to `fw_builder.py` passing all arguments
through verbatim.

**Normal usage — invoke via `builder.sh`:**

```bash
# Dev build, -O0 (DEBUG_ENABLE=1)  →  app_dev_m0r0c0_o0
bash scripts/builder.sh rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=0

# Dev build, -O1                   →  app_dev_m0r0c0_o1
bash scripts/builder.sh rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=1

# Production build, -O0            →  app_prd_m0r0c0_o0
bash scripts/builder.sh rebuild preset=m0r0c0 target=app tag=prd bsp=0 opt=0

# Clean only
bash scripts/builder.sh clean preset=m0r0c0
```

VS Code build tasks pass these same argument strings (defined in `.vscode/settings.json`):

```json
"app_dev_m0r0c0_o0": "rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=0"
```

**Direct Python invocation** (requires `.venv` active or `tqdm` installed globally):

```bash
python3 scripts/fw_builder.py rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=0
python3 scripts/fw_builder.py clean preset=m0r0c0
```

**Direct CMake** (without any wrapper):

```bash
cmake --preset m0r0c0 -DFW_TYPE=dev -DFW_OPT=0
cmake --build build/m0r0c0 --target app.elf
cmake --build build/m0r0c0 --target boot.elf
```

Output artifact: `build/fw_m0.elf` (symlink to versioned ELF, e.g. `1.0.0.ecu.app.dev.m0r0c0.0.a1b2c3.elf`).

**There are no unit tests or linter in this project.** Validation is done by building and flashing to hardware.

Flashing via OpenOCD (CMSIS-DAP adapter):

```bash
openocd -f platform/cmsis-dap.cfg -f platform/m0/stm32g4x.cfg \
  -c "program build/fw_m0.elf verify reset exit"
```

---

## Architecture

Strict layering — never skip layers:

```
app/ (motor control, sensors, UART protocol, debug)
  └─► platform/platform.h  (Pl_* API only)
        └─► platform/m0/platform.c
              └─► platform/m0/c0/*.c  (STM32 LL drivers)
```

- App code calls only `Pl_*` functions. Never call `LL_*`, `TIM_*`, `ADC_*`, `GPIO_*` directly from app.
- Platform layer uses STM32 LL (Low-Layer) exclusively — no HAL.
- `shared/` and `lib/` are MCU-agnostic portable code.
- Global struct instances (`Motor_t motor`, `Sensors_t sensors`) are owned by their `.c` files;
  expose via getter: `Motor_GetMotorPtr()`, `Sensors_GetPtrSensors()`.

---

## Naming Conventions

**Functions:**

```c
Module_VerbNoun()                // public app API
Pl_Module_VerbNoun()             // platform HAL functions
Pl_Stub_VerbNoun()               // no-op stub callbacks
vTask_ModuleVerb()               // FreeRTOS task functions (static)
FreeRTOS_Module_InitComponents() // FreeRTOS init wrappers
Module_TaskCreate()              // task creation guards
```

**Variables:**

```c
camelCase                        // local variables
pCamelCase                       // pointer parameters (p prefix)
Module_HandleName                // static task/queue handles (PascalCase)
UPPER_SNAKE_CASE[]               // static storage arrays/buffers
camelCase                        // struct instance fields
```

**Types — all `_t` suffix, UPPER_SNAKE_CASE:**

```c
typedef struct { ... } ControlInfo_t;
typedef enum  { ... } RET_STATE_t;
typedef void (*Pl_Common_Clbk_t)(void);  // function pointers
```

Exception: filter/PID use tag + `T` prefix: `struct SFilter` / `typedef volatile struct SFilter TFilter`.

**Macros:** `UPPER_SNAKE_CASE`. Align values with spaces for readability.

**Files:** `snake_case.c` / `snake_case.h`

**Enum values:** `UPPER_SNAKE_CASE` with module prefix (`RET_STATE_SUCCESS`, `HALLS_STEP1`).

---

## Types

Always use the project aliases from `shared/def_types.h`:

```c
u8, u16, u32, u64   // unsigned integers
s8, s16, s32, s64   // signed integers
```

Use `float` directly (no typedef). Use `bool` from `<stdbool.h>` directly.
Never use raw `uint32_t` / `int32_t` in application or platform code — only in LL init structs.

---

## Code Style

**Indentation:** tabs (not spaces).

**Braces:** Allman style for platform-layer functions; K&R (same-line open brace) acceptable in app layer. Always use braces even for single-statement `if`/`else`.

**Header guards:**

```c
#ifndef __MODULE_NAME_H
#define __MODULE_NAME_H
// ...
#endif /* __MODULE_NAME_H */
```

Double-underscore prefix, uppercase, closing comment required.

**Include order** (in `.c` files):

1. Own module header: `#include "bldc.h"`
2. Sibling modules: `#include "../motor.h"`
3. Platform: `#include "platform.h"`
4. Shared/lib: `#include "def_types.h"`, `#include "ring_buff.h"`
5. Third-party LL headers last (rarely needed directly in app)

No `<system>` includes in `.c` files — pull through `def_types.h` or `main.h`.

**Comments:** Use `//` line comments. Use `/* === ... === */` banners for section separators in platform code. Doxygen (`/** @brief */`) only for high-level config in headers.

**Struct initialization:** use designated initializers:

```c
Motor_t Motor = {
    .controlInfo = { .direction = 0, .dutyCycle = 0.0f },
    .filterCurA  = { .T = 0.08f, .Calc = TFilter_Calc },
};
```

**Unused parameters:** suppress with `DISCARD_UNUSED(x)` (expands to `((void)x)`).

---

## Error Handling

Functions that can fail return `RET_STATE_t`. Functions that cannot fail return `void`.

```c
RET_STATE_t rs = RET_STATE_UNDEF;  // always initialize to UNDEF

// Early exit macro (prefer for sequential steps):
RETURN_IF_UNSUCCESS(SomeFunc());

// Manual check:
rs = SomeFunc();
if (rs != RET_STATE_SUCCESS) {
    return rs;
}
```

Key states: `RET_STATE_SUCCESS`, `RET_STATE_ERROR`, `RET_STATE_ERR_PARAM`,
`RET_STATE_ERR_MEMORY`, `RET_STATE_ERR_TIMEOUT`, `RET_STATE_ERR_BUSY`.

**Panic / assertion:**

```c
PANIC();           // ErrorHandler → disable IRQ + breakpoint + infinite loop
ASSERT_CHECK(x);   // calls PANIC() if x == 0 (active when ASSERT_CHECK_ENABLE defined)
```

**Debug-only code:** wrap with `#if DEBUG_ENABLE` / `#if !DEBUG_ENABLE`.

---

## FreeRTOS Task Pattern

Every task module follows this exact structure:

```c
// 1. Static handle
static TaskHandle_t Module_Handle = NULL;

// 2. Task function
static void vTask_ModuleName(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for(;;) {
        vTaskDelayUntil(&xLastWakeTime, TASK_PERIOD_MS);
        // work
    }
}

// 3. Create function (guard against double-creation)
void Module_TaskCreate(void) {
    if (!Module_Handle) {
        xTaskCreate(vTask_ModuleName, "Task name",
                    MODULE_TASK_STACK, NULL,
                    MODULE_TASK_PRIORITY, &Module_Handle);
    }
}

// 4. FreeRTOS init entry point (called from main.c)
void FreeRTOS_Module_InitComponents(void) {
    Module_TaskCreate();
}
```

Stack sizes and priorities are defined in `app/conf/tasks_stack_and_priority.h`.
Use `configMINIMAL_STACK_SIZE` multiples for stack sizes.
Use `TASK_PRIORITY_NN` constants for priorities.

Critical sections: `SYS_CRITICAL_ON()` / `SYS_CRITICAL_OFF()` (map to FreeRTOS critical section API).

---

## Key Module APIs

**Filter (first-order IIR low-pass):**

```c
TFilter f = { .T = 0.05f, .calc = TFilter_Calc };  // T = Tsample / Tfilter
f.input = rawValue;
f.calc(&f);
filtValue = f.output;
```

**PID controller:**

```c
volatile struct PI_t pid = { .Kp=1.0f, .Ki=0.0f, .dt=0.001f,
                             .outMax=100.0f, .outMin=-100.0f, .calc=PID_Calc };
pid.in = setpoint - measurement;
pid.calc(&pid);
output = pid.out;
```

Declare `volatile` when accessed from ISR context.

**Ring buffer:**

```c
RingBuff_Init(&rb, backingBuf, sizeof(backingBuf), NULL);
RingBuff_InterruptCallback(&rb, newByteCount);           // call from ISR
RingBuff_Str_Search(&rb, DELAY_1_MILLISECOND, "token");  // blocking search
RingBuff_Str_SetSearch(&rb, timeout_ms, patterns, n, &matchIdx);
```

**Debug print** (dev builds only):

```c
DEBUG_PRINT("value: %d\n", val);   // routes to USART via syscalls.c
```

---

## Compile-time Flags

| Flag | Values | Effect |
|---|---|---|
| `DEBUG_ENABLE` | `0` / `1` | Enables debug UART, disables IWDG |
| `FW_OPT` | `0` / `1` | `-O0` / `-O1` |
| `ASSERT_CHECK_ENABLE` | defined / not | Enables `ASSERT_CHECK()` macro |
| `ERR_HANDLE_ENABLE` | `0` / `1` | Enables `ERROR_HANDLER()` macro |
| `USE_FULL_LL_DRIVER` | always set | STM32 full LL driver |
| `HSE_VALUE` | `24000000U` | External crystal frequency |
