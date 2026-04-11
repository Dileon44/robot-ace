cmake_minimum_required(VERSION 3.30)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(ace VERSION 1.0.0)
enable_language(C ASM)
set(CMAKE_C_STANDARD 17)

set(BUILD_DIR build)

set(FW_VER_APP  ${PROJECT_VERSION})
set(FW_VER_BOOT 1.0.0)

set(FW_PROJECT "robot-ace")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" FW_VER_MAJOR "${PROJECT_VERSION}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" FW_VER_MINOR "${PROJECT_VERSION}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" FW_VER_BUILD "${PROJECT_VERSION}")
set(FW_BSP 0)

execute_process(
    COMMAND git rev-parse --short=6 HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(FW_GIT_HASH    "${GIT_COMMIT_HASH}")
set(FW_GIT_BRANCH  "${GIT_BRANCH_NAME}")

# Optimization level: set via -DFW_OPT=0|1 (fw_builder.py), default 0
if(NOT DEFINED FW_OPT)
	set(FW_OPT 0)
endif()
if(FW_OPT EQUAL 1)
	set(OPT_FLAG -O1)
else()
	set(OPT_FLAG -Og)
endif()

# Debug mode: tag=dev → DEBUG_ENABLE=1; tag=prd → macro not defined
if(NOT DEFINED FW_TAG)
	set(FW_TAG prd)
endif()
if(FW_TAG STREQUAL "dev")
	set(DEBUG_ENABLE_DEF
		DEBUG_ENABLE=1
		PANIC_CHECK_ENABLE=1
		APP_ASSERT_CHECK_ENABLE=1
		RTOS_ASSERT_CHECK_ENABLE=1
	)
else()
	set(DEBUG_ENABLE_DEF "")
endif()

# LTO: enabled for opt=1 builds (both dev and prd)
if(FW_OPT EQUAL 1)
	set(LTO_FLAGS -flto)
else()
	set(LTO_FLAGS "")
endif()

add_compile_options(
	${OPT_FLAG}
	${LTO_FLAGS}
	-g
	-specs=nosys.specs
	-ffunction-sections
	-fdata-sections
	-fstack-usage
	-Wall
	-Wno-unused-function
	-Wno-switch
	-Wno-format
	-Wno-comment
	-MMD
)

add_link_options(
	${LTO_FLAGS}
	-specs=nano.specs
	-specs=nosys.specs
	-static
	-Wl,--start-group -lc -lm -Wl,--end-group
	-Wl,--gc-sections
	-Wl,--print-memory-usage
	-Wl,--no-warn-rwx-segments
)

add_compile_definitions(
	FW_OPT=${FW_OPT}
	${DEBUG_ENABLE_DEF}
)

# Platform — defines cpu_flags and platform targets; sets FLASH_* cache vars
add_subdirectory(platform/${FW_PLATFORM_MCU})

# Linker script generation (uses FLASH_* set by platform/${FW_PLATFORM_MCU})
set(LD_FLASH_START ${FLASH_BOOT_ADDR})
set(LD_FLASH_SIZE  ${FLASH_BOOT_SIZE})
configure_file(
	${CMAKE_SOURCE_DIR}/platform/${FW_PLATFORM_MCU}/linker_script.ld
	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_boot_linker_script.ld
)
unset(LD_FLASH_START)
unset(LD_FLASH_SIZE)

set(LD_FLASH_START ${FLASH_APP_ADDR})
set(LD_FLASH_SIZE  ${FLASH_APP_SIZE})
configure_file(
	${CMAKE_SOURCE_DIR}/platform/${FW_PLATFORM_MCU}/linker_script.ld
	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_app_linker_script.ld
)
unset(LD_FLASH_START)
unset(LD_FLASH_SIZE)

# FreeRTOS config (must be defined before add_subdirectory(thirdparty))
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
	${CMAKE_SOURCE_DIR}/app
	${CMAKE_SOURCE_DIR}/app/conf
	${CMAKE_SOURCE_DIR}/app/motor
)
target_compile_definitions(freertos_config INTERFACE
	projCOVERAGE_TEST=0
)
set(FREERTOS_HEAP "4" CACHE STRING "FreeRTOS Heap Model" FORCE)
set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "FreeRTOS Cross Compile Port" FORCE)

add_subdirectory(thirdparty)
add_subdirectory(shared)
add_subdirectory(lib)
add_subdirectory(bsp)
add_subdirectory(app)
add_subdirectory(boot)
