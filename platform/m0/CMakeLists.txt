# Flash memory map — CACHE INTERNAL so they're visible across all subdirectories
set(FLASH_BOOT_ADDR "0x08000000" CACHE INTERNAL "Boot flash base address")
set(FLASH_BOOT_SIZE "0x4000"     CACHE INTERNAL "Boot flash size (16 KB)")
set(FLASH_APP_ADDR  "0x08000000" CACHE INTERNAL "App flash base address")
set(FLASH_APP_SIZE  "0x18000"    CACHE INTERNAL "App flash size (96 KB)")

# CPU-specific flags — consumed by all targets via target_link_libraries(... cpu_flags)
add_library(cpu_flags INTERFACE)

target_compile_options(cpu_flags INTERFACE
	-mcpu=cortex-m4
	-mfpu=fpv4-sp-d16
	-mfloat-abi=hard
	-mthumb
)

target_link_options(cpu_flags INTERFACE
	-mcpu=cortex-m4
	-mfloat-abi=hard
	-mfpu=fpv4-sp-d16
	-Wl,-cref
	-Wl,-u,Reset_Handler
	-Wl,--defsym=malloc_getpagesize_P=0x80
	-Wl,-Map=${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}.map
)

# Platform sources — INTERFACE: compiled in the context of each consuming target.
# Required because platform.h and c0 headers include "main.h", which differs
# between app (app/main.h with FreeRTOS) and boot (boot/main.h, bare-metal).
file(GLOB         SOURCES_C0   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/c0/*.c")
file(GLOB_RECURSE SOURCES_CORE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/*.c")

add_library(platform INTERFACE)

target_sources(platform INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/platform.c
	${CMAKE_CURRENT_SOURCE_DIR}/startup.s
	${SOURCES_C0}
	${SOURCES_CORE}
)

target_include_directories(platform INTERFACE
	${CMAKE_SOURCE_DIR}/platform
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/c0
	${CMAKE_CURRENT_SOURCE_DIR}/core
	${CMAKE_CURRENT_SOURCE_DIR}/core/CMSIS
	${CMAKE_CURRENT_SOURCE_DIR}/core/HAL/inc
	${CMAKE_CURRENT_SOURCE_DIR}/core/HAL/inc/Legacy
	${CMAKE_CURRENT_SOURCE_DIR}/core/LL/inc
)

target_compile_definitions(platform INTERFACE
	USE_FULL_ASSERT
	HSE_VALUE=8000000U
	STM32G431xx
	USE_FULL_LL_DRIVER
	FLASH_BOOT_ADDR=${FLASH_BOOT_ADDR}
	FLASH_BOOT_SIZE=${FLASH_BOOT_SIZE}
	FLASH_APP_ADDR=${FLASH_APP_ADDR}
	FLASH_APP_SIZE=${FLASH_APP_SIZE}
)

target_link_libraries(platform INTERFACE cpu_flags)
