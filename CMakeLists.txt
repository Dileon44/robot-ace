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

project(ecu VERSION 1.0.0)
enable_language(C ASM)
SET(CMAKE_C_STANDARD 17)

set(BUILD_DIR build)

set(FW_VER_APP ${PROJECT_VERSION})
set(FW_VER_BOOT 1.0.0)

# set(PLATFORM m3)
# set(FW_PLATFORM m3r0c0)
set(FW_BSP 0)

execute_process(
	COMMAND git rev-parse --short=6 HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(FW_GIT_HASH ${GIT_COMMIT_HASH})

file(GLOB_RECURSE SOURCES
	"shared/*.c"
	"platform/${FW_PLATFORM_MCU}/${FW_PLATFORM_CFG}/*.c"
	"platform/${FW_PLATFORM_MCU}/core/*.c"
	"platform/${FW_PLATFORM_MCU}/platform.c"
	"platform/${FW_PLATFORM_MCU}/*.s"
)

include_directories(
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_SOURCE_DIR}/shared
	${CMAKE_SOURCE_DIR}/platform
	${CMAKE_SOURCE_DIR}/bsp
)

# Optimization level: set via -DFW_OPT=0|1 (fw_builder.py), default 0
if(NOT DEFINED FW_OPT)
	set(FW_OPT 0)
endif()
if(FW_OPT EQUAL 1)
	set(OPT_FLAG -O1)
else()
	set(OPT_FLAG -O0)
endif()

# Debug mode: tag=dev → DEBUG_ENABLE=1; tag=prd → macro not defined
if(NOT DEFINED FW_TYPE)
	set(FW_TYPE prd)
endif()
if(FW_TYPE STREQUAL "dev")
	set(DEBUG_ENABLE_DEF DEBUG_ENABLE=1)
else()
	set(DEBUG_ENABLE_DEF "")
endif()

add_compile_options(
	${OPT_FLAG}
	-g
	-specs=nosys.specs
	-ffunction-sections
	-fdata-sections
	-fstack-usage
	# -std=gnu11
	-Wall
	-Wno-unused-function
	-Wno-switch
	-Wno-format
	-Wno-comment
	-MMD
)

add_link_options(
	-specs=nano.specs
	-specs=nosys.specs
	# "LINKER:-u _printf_float" # removed de-duplication for these options
	# "LINKER:-u _scanf_float"
	-static
	-Wl,--start-group -lc -lm -Wl,--end-group
	-Wl,--gc-sections
	-Wl,--print-memory-usage
	-Wl,--no-warn-rwx-segments
)

# PLATFORM SPECIFIC
# add_subdirectory(platform/m3)
include(platform/${FW_PLATFORM_MCU}/CMakeLists.txt)

add_compile_definitions(
	FLASH_BOOT_ADDR=${FLASH_BOOT_ADDR}
	FLASH_BOOT_SIZE=${FLASH_BOOT_SIZE}
	FLASH_APP_ADDR=${FLASH_APP_ADDR}
	FLASH_APP_SIZE=${FLASH_APP_SIZE}
	FW_OPT=${FW_OPT}
	${DEBUG_ENABLE_DEF}
)
# PLATFORM SPECIFIC

# FREERTOS SETTINGS
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
	INTERFACE
	${CMAKE_SOURCE_DIR}/app
	${CMAKE_SOURCE_DIR}/app/conf
	${CMAKE_SOURCE_DIR}/app/motor
)

target_compile_definitions(freertos_config
	INTERFACE
	projCOVERAGE_TEST=0
)

set(FREERTOS_HEAP "4" CACHE STRING "FreeRTOS Heap Model" FORCE)
set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "FreeRTOS Cross Compile Port" FORCE)

set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/thirdparty/freertos_kernel)
add_subdirectory(${FREERTOS_KERNEL_PATH} ${CMAKE_BINARY_DIR}/freertos_kernel)
# FREERTOS SETTINGS

# APP
set(TARGET_APP app)
set(APP_FULL_NAME ${FW_VER_APP}.${PROJECT_NAME}.app.${FW_TYPE}.${FW_PLATFORM}.${FW_BSP}.${FW_GIT_HASH})
add_executable(${TARGET_APP}.elf ${SOURCES})
set_target_properties(${TARGET_APP}.elf PROPERTIES VERSION ${FW_VER_APP})
target_link_libraries(${TARGET_APP}.elf freertos_kernel)
file(GLOB SOURCES_LIB "lib/*.c")
file(GLOB_RECURSE SOURCES_APP
	"app/*.c"
)
list(APPEND SOURCES_APP ${SOURCES_LIB})
target_sources(${TARGET_APP}.elf PRIVATE ${SOURCES_APP})
target_include_directories(${TARGET_APP}.elf PRIVATE
	${CMAKE_SOURCE_DIR}/app
)
target_link_options(${TARGET_APP}.elf PRIVATE
	-u _printf_float
	-T${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_app_linker_script.ld
)
set_target_properties(${TARGET_APP}.elf
	PROPERTIES
		OUTPUT_NAME ${APP_FULL_NAME}.elf
)
add_custom_command(TARGET ${TARGET_APP}.elf POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}/${APP_FULL_NAME}.elf
	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/fw_${FW_PLATFORM_MCU}.elf
)
# APP

# BOOT
set(TARGET_BOOT boot)
set(BOOT_FULL_NAME ${FW_VER_BOOT}.${PROJECT_NAME}.boot.${FW_TYPE}.${FW_PLATFORM}.${FW_BSP}.${FW_GIT_HASH})
add_executable(${TARGET_BOOT}.elf ${SOURCES})
set_target_properties(${TARGET_APP}.elf PROPERTIES VERSION ${FW_VER_BOOT})
file(GLOB_RECURSE SOURCES_BOOT
	"boot/*.c"
)
target_sources(${TARGET_BOOT}.elf PRIVATE ${SOURCES_BOOT})
target_include_directories(${TARGET_BOOT}.elf PRIVATE
	${CMAKE_SOURCE_DIR}/boot
)
target_link_options(${TARGET_BOOT}.elf PRIVATE
	-T${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_boot_linker_script.ld
)
set_target_properties(${TARGET_BOOT}.elf
	PROPERTIES
		OUTPUT_NAME ${BOOT_FULL_NAME}.elf
)
# add_custom_command(TARGET ${TARGET_BOOT}.elf POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy
# 	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${BOOT_FULL_NAME}.elf
# 	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/fw_${FW_PLATFORM_MCU}.elf
# )
# BOOT

# LINKER SCRIPTS GEN
set(LD_FLASH_START ${FLASH_BOOT_ADDR})
set(LD_FLASH_SIZE ${FLASH_BOOT_SIZE})
configure_file(
	${CMAKE_SOURCE_DIR}/platform/${FW_PLATFORM_MCU}/linker_script.ld
	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_boot_linker_script.ld
)
unset(LD_FLASH_START)
unset(LD_FLASH_SIZE)

set(LD_FLASH_START ${FLASH_APP_ADDR})
set(LD_FLASH_SIZE ${FLASH_APP_SIZE})
configure_file(
	${CMAKE_SOURCE_DIR}/platform/${FW_PLATFORM_MCU}/linker_script.ld
	${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_app_linker_script.ld
)
unset(LD_FLASH_START)
unset(LD_FLASH_SIZE)
# LINKER SCRIPTS GEN
