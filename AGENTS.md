# robot-ace — Agent Instructions

Embedded firmware for a PMSM motor controller ACE (actuator control electronics). This unit will control the electronic drive unit in the robot's joint.

When writing the code, you need to keep in mind that this program will control power transistors on an electronic board, so you need to pay attention to safety - think about through currents in three-phase bridge systems. And you also need to understand that in any emergency situation, it is necessary to close the power transistors in order to save the board and the MCU.

Target: STM32G431CBU (Cortex-M4F, 128 KB Flash, 32 KB RAM). RTOS: FreeRTOS V11.2.0.

Languages: **C17** and **C++23**. The project is mid-migration to C++ — see [C++ Rules](#c-rules)
for which layer is written in which language. Both standards are enforced by CMake
(`CMAKE_C_STANDARD 17`, `CMAKE_CXX_STANDARD 23`, `CMAKE_CXX_EXTENSIONS OFF`).

---

## Build Commands

The full build chain is: **VS Code task → `scripts/builder.sh` → `scripts/fw_builder.py` → CMake**.

`builder.sh` handles OS detection and virtual environment setup (creates `.venv` on first run,
installs `scripts/requirements.txt`), then delegates to `fw_builder.py` passing all arguments
through verbatim. It also runs `git submodule update --init --recursive` on each build.

**Normal usage — invoke via `builder.sh`:**

```bash
# Dev build, -Og (DEBUG_ENABLE=1)  →  app_dev_m0r0c0_og
bash scripts/builder.sh rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=0

# Dev build, -O1                   →  app_dev_m0r0c0_o1
bash scripts/builder.sh rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=1

# Production build, -O1            →  app_prd_m0r0c0_o1
bash scripts/builder.sh rebuild preset=m0r0c0 target=app tag=prd bsp=0 opt=1

# Bootloader build
bash scripts/builder.sh rebuild preset=m0r0c0 target=boot tag=dev bsp=0 opt=0

# Clean only
bash scripts/builder.sh clean preset=m0r0c0
```

VS Code build tasks pass these same argument strings (defined in `.vscode/settings.json`):

```json
"app_dev_m0r0c0_og": "rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=0"
```

**Direct Python invocation** (requires `.venv` active or `tqdm` installed globally):

```bash
python3 scripts/fw_builder.py rebuild preset=m0r0c0 target=app tag=dev bsp=0 opt=0
python3 scripts/fw_builder.py clean preset=m0r0c0
```

**Direct CMake** (without any wrapper):

```bash
cmake --preset m0r0c0 -DFW_TYPE=dev -DFW_OPT=0
cmake --build build/m0r0c0 --target app
cmake --build build/m0r0c0 --target boot
```

Output artifacts:

- `build/fw_m0.elf` / `build/fw_m0.hex` / `build/fw_m0.bin` — app (flat-named copies)
- `build/boot_m0.elf` / `build/boot_m0.hex` / `build/boot_m0.bin` — bootloader (flat-named copies)
- `artifacts/` — versioned copies of all ELF/HEX/BIN files, e.g. `1.0.0.ecu.app.dev.m0r0c0.0.a1b2c3.elf`

The internal CMake build output resides in `build/m0r0c0/app/` (not used directly).

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
app/       (motor control, sensors, UART protocol, debug — FreeRTOS tasks)
  conf/    (FreeRTOSConfig.h, dbg_cfg.h, lib_cfg.h, main_cfg.h, rtos_tasks_stack_and_prio.h)
boot/      (bootloader — bare-metal, no RTOS, separate CMake target)
  conf/    (dbg_cfg.h, lib_cfg.h, main_cfg.h)
  └─► platform/platform.h  (Pl_* API only)
        └─► platform/m0/platform.c
              └─► platform/m0/c0/*.c     (STM32 LL peripheral drivers: adc, dma, gpio, int, sys, tim, usart)
              └─► platform/m0/core/      (CMSIS, STM32 LL/HAL headers+src, USB Device stack)

shared/    (MCU-agnostic: types, macros, delay, syscalls — INTERFACE lib)
lib/       (MCU-agnostic: ring_buff, ring_list, rand, shared_mutex — CMake target: libs)
thirdparty/freertos_kernel/   (FreeRTOS V11.2.0 — linked into app only)
thirdparty/wsh-shell/         (wsh-shell v2.4 STATIC lib — linked into app only)
```

- App code calls only `Pl_*` functions. Never call `LL_*`, `TIM_*`, `ADC_*`, `GPIO_*` directly from app.
- Platform layer uses STM32 LL (Low-Layer) exclusively — no HAL (HAL subset exists in `core/` only for USB PCD).
- `shared/` and `lib/` are MCU-agnostic portable code. The CMake target for `lib/` is `libs` (plural).
- `boot/` links only `platform` and `shared` — no FreeRTOS, no `libs`, no `wsh_shell`.
- `app/CMakeLists.txt` uses `file(GLOB_RECURSE ...)` over both `*.c` and `*.cpp` — any source
  added under `app/` is auto-included. `bsp/`, `shared/` and `lib/` are INTERFACE libraries and
  list their sources explicitly, so a new file there must be added to the local `CMakeLists.txt`.
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

## C++ Rules

The project is migrating from C to C++. Everything in this section applies to `.cpp` / `.hpp`
files only — C sources keep the conventions above unchanged.

### Which layer is written in which language

| Layer | Language | Reason |
|---|---|---|
| `bsp/` | **C++23** | Several drivers behind one interface — the case polymorphism is for |
| `app/` | **C++23** for new code | Application logic, FOC, tasks. Existing `.c` modules stay until they need reworking anyway |
| `shared/` | C17, plus C++-only `.hpp` headers | Types and macros are shared with C; `critical_section.hpp` is C++-only |
| `platform/` | C17 | Register-level LL code. C++ adds nothing here but risk |
| `lib/` | C17 | Working, debugged code. Rewrite only with a functional reason |
| `boot/` | C17 | Bare-metal loader — fewer dependencies is better |
| `thirdparty/` | as-is | FreeRTOS and wsh-shell are never modified |

Do not convert a working C module to C++ just to convert it. Write new code in C++; migrate old
code when it is being substantially reworked for other reasons.

### Language subset

The profile is set by the flags in the root [CMakeLists.txt](CMakeLists.txt) and is not optional:
`-fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -fno-unwind-tables
-fno-asynchronous-unwind-tables`, plus `-Wsuggest-override -Woverloaded-virtual`.

**Never use:**

| What | Why |
|---|---|
| Exceptions | +4.3 KB and unbounded stack-unwind latency — unacceptable in the FOC path |
| RTTI, `dynamic_cast`, `typeid` | Type tables in Flash, not needed |
| `new` / `delete` | No heap. Every operator is trapped into `ErrorHandler()` in [app/cpp_runtime.cpp](app/cpp_runtime.cpp) |
| `std::string`, `std::vector`, `std::map`, `std::function` | Heap and/or kilobytes of code |
| `<iostream>`, `std::shared_ptr` | Tens of KB / atomic refcount + heap |
| Non-trivial global constructors | Static init order fiasco — see below |
| Function-local `static` | `-fno-threadsafe-statics` strips the guard, leaving them non-reentrant |
| Multiple or virtual inheritance | Opaque vtable cost |

**Use freely** (cost ~zero): `constexpr` / `consteval`, `enum class`, `namespace`, references,
overloading, default arguments, `static_assert`, `[[nodiscard]]`, `[[maybe_unused]]`, RAII guards,
`std::array`, and the header-only parts of the STL — `<type_traits>`, `<concepts>`, `<bit>`,
`<limits>`, `<span>`, `<utility>`, non-allocating `<algorithm>`.

### Naming

`namespace` replaces the C module prefix — `bsp::encoder::GetPtr()`, not `EncoderM_GetPtr()`.

```cpp
namespace bsp::encoder {          // namespaces: lowercase, nested with ::
class IDriver { ... };            // interfaces: I prefix
class As5600 final : public IDriver {
  public:
    bool  Init() override;        // methods: PascalCase
    u16   GetRawAngle() override;
  private:
    IBus& bus_;                   // data members: trailing underscore
};
constinit As5600 As5600Dev{As5600BusDev};   // objects: PascalCase
}  // namespace bsp::encoder
```

- **Types** (class / struct / enum class): `PascalCase`, **no `_t` suffix** — that suffix marks C types.
- **`constexpr` constants:** `PascalCase`. `UPPER_SNAKE_CASE` stays reserved for macros, so the two
  never look alike.
- **Locals and parameters:** `camelCase`, same as C.
- **Files:** `snake_case.hpp` / `snake_case.cpp`. Header guard `__FILE_NAME_HPP`, closing comment required.
- **TU-local entities:** anonymous `namespace { ... }`, not `static`.

### Classes and polymorphism

Follow the pattern already established in [bsp/bsp_module.hpp](bsp/bsp_module.hpp):

- Interfaces are pure-virtual with a **`protected`, non-virtual destructor**. Instances are static
  and never deleted, so a virtual destructor would only add a vtable slot and drag in `operator delete`.
- Concrete classes are `final`.
- `override` on every override — `-Wsuggest-override` makes a missed one a warning and a mismatched
  signature a compile error.
- Single public inheritance only, and only to implement an interface.
- `virtual` only where the polymorphism is genuinely resolved at runtime (swappable drivers).
  **Never on the ISR or FOC path** — use templates / CRTP there.
- Prefer references over pointers where null is impossible (`IBus& bus_`) — the null check disappears.

### Static initialization — the hard rule

Constructors of global objects run in `__libc_init_array` **before `main()`**, therefore before
`Pl_Init()` and before the clock is configured. Order across translation units is unspecified.

**Every global object must be `constinit`, with a `constexpr` constructor that touches no hardware.**
All hardware work goes into an explicit `Init()` called from `main()` in a controlled order — the
sequence in [app/main.c](app/main.c) is the contract and must not be bypassed.

```cpp
constexpr explicit As5600(IBus& bus) : bus_(bus) {}   // no hardware access
...
constinit As5600 As5600Dev{As5600BusDev};             // no .init_array entry
```

Check with `arm-none-eabi-objdump -s -j .init_array build/fw_m0.elf` — new entries mean a
constructor slipped into pre-`main()` startup.

### The C / C++ boundary

- Every C header included from C++ is wrapped in `#ifdef __cplusplus extern "C" { #endif`.
- A C++ header included from a C-visible header goes **before** the `extern "C"` block so it keeps
  C++ linkage — see [bsp/bsp.h](bsp/bsp.h).
- Anything called *from* C must be `extern "C"`: ISRs referenced by `startup.s`, FreeRTOS task
  functions, LL callbacks, `main`. Vector-table names are not mangled — a missed `extern "C"` here
  is a link error at best and a silently unused handler at worst.
- **Enums across the boundary:** `-fshort-enums` is on by default for `arm-none-eabi`, so a C enum
  is **1 byte**. A scoped enum must pin the same width — `enum class Foo : u8` — otherwise the two
  sides disagree on size inside structs and through pointers, silently.

### Casts

`static_cast` / `reinterpret_cast`, never C-style casts in C++ code (see [bsp/encoder/as5600/as5600.cpp](bsp/encoder/as5600/as5600.cpp)).
`-Wold-style-cast` is deliberately **not** enabled — CMSIS and LL headers would drown the build in warnings.

### RAII

Critical sections use the guards from [shared/critical_section.hpp](shared/critical_section.hpp)
instead of the `SYS_CRITICAL_ON()` / `SYS_CRITICAL_OFF()` pair:

```cpp
#include "critical_section.hpp"

RET_STATE_t Foo() {
    sys::CriticalSection cs;          // task context
    RETURN_IF_UNSUCCESS(Bar());       // section still closes on this early return
    return RET_STATE_SUCCESS;
}

void ISR() {
    sys::CriticalSectionIsr cs;       // ISR context — carries the FreeRTOS mask
}
```

Generated code is identical to the raw macro pair (verified). **Always give the guard a name** —
`sys::CriticalSection();` builds a temporary that dies at the end of that statement and protects
nothing, with no compiler diagnostic.

### Error handling in C++

Same `RET_STATE_t` and the same rules as [Error Handling](#error-handling) below. Add `[[nodiscard]]`
to fallible functions so an ignored error code becomes a warning.

> Known inconsistency: `IModule::Init()` / `IModuleBus::Init()` in `bsp/` return `bool`, losing the
> reason for a failure. New interfaces should return `RET_STATE_t`.

### Templates

Keep the number of instantiations bounded, and put logic that does not depend on the template
parameters into a non-template base class — otherwise `Ring<256>` and `Ring<64>` become two
independent copies of the same code. A template is not automatically smaller: a CRTP task base
measured *larger* than one `constinit const` table walked by a single loop.

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

Key states: `RET_STATE_SUCCESS`, `RET_STATE_ERROR`, `RET_STATE_WARNING`,
`RET_STATE_ERR_PARAM`, `RET_STATE_ERR_MEMORY`, `RET_STATE_ERR_CRC`,
`RET_STATE_ERR_EMPTY`, `RET_STATE_ERR_BUSY`, `RET_STATE_ERR_OVERFLOW`,
`RET_STATE_ERR_TIMEOUT`.

**Panic / assertion:**

```c
PANIC();           // ErrorHandler → disable IRQ + breakpoint + infinite loop
ASSERT_CHECK(x);   // calls PANIC() if x == 0 (active when PANIC_CHECK_ENABLE defined)
```

Panic behavior is controlled by flags in `app/conf/dbg_cfg.h`:

- `PANIC_CHECK_ENABLE` — enables `PANIC()` (defaults to `0` even in dev builds)
- `APP_ASSERT_CHECK_ENABLE` — enables `ASSERT_CHECK(x)`
- `RTOS_ASSERT_CHECK_ENABLE` — enables FreeRTOS stack overflow checks
- `DEBUG_QUICK_ENABLE` — master switch: set to `1` to enable all three above at once

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

Stack sizes and priorities are defined in `app/conf/rtos_tasks_stack_and_prio.h`.
Use `configMINIMAL_STACK_SIZE` multiples for stack sizes.
Use `TASK_PRIORITY_01` … `TASK_PRIORITY_08` constants for priorities (`TASK_PRIORITY_08` is also aliased as `WATCHDOG_TASK_PRIORITY`).

Critical sections: `SYS_CRITICAL_ON()` / `SYS_CRITICAL_OFF()` (map to FreeRTOS critical section API).

---

## Key Module APIs

**Filter (first-order IIR low-pass):** *(planned — module not yet implemented)*

```c
TFilter f = { .T = 0.05f, .Calc = TFilter_Calc };  // T = Tsample / Tfilter
f.input = rawValue;
f.calc(&f);
filtValue = f.output;
```

**PID controller:** *(planned — module not yet implemented)*

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
RingBuff_Str_Search(&rb, DELAY_1_MILSEC, "token");       // blocking search
RingBuff_Str_SetSearch(&rb, timeout_ms, patterns, n, &matchIdx);
```

**Debug print** *(planned — not yet implemented; will route to USART via syscalls.c)*:

```c
DEBUG_PRINT("value: %d\n", val);
```

---

## Compile-time Flags

| Flag | Values | Effect |
|---|---|---|
| `DEBUG_ENABLE` | `0` / `1` | Enables debug UART, disables IWDG |
| `FW_OPT` | `0` / `1` | `-Og` / `-O1` |
| `PANIC_CHECK_ENABLE` | defined / not | Enables `PANIC()` / `ASSERT_CHECK()` macros (app) |
| `USE_FULL_LL_DRIVER` | always set | STM32 full LL driver |
| `HSE_VALUE` | `8000000U` | External crystal frequency |
| `STM32G431xx` | always set | STM32G431 device family selection |
| `USE_FULL_ASSERT` | always set | Enables STM32 assert_param checks |
| `FLASH_BOOT_ADDR` | `0x08000000` | Boot flash base address |
| `FLASH_BOOT_SIZE` | `0x4000` | Boot flash size (16 KB) |
| `FLASH_APP_ADDR` | `0x08000000` | App flash base address |
| `FLASH_APP_SIZE` | `0x18000` | App flash size (96 KB) |
