add_subdirectory(freertos_kernel ${CMAKE_BINARY_DIR}/freertos_kernel)

# Provide MCU-specific compile flags to FreeRTOS kernel targets
target_link_libraries(freertos_kernel      PRIVATE cpu_flags)
target_link_libraries(freertos_kernel_port PRIVATE cpu_flags)

# --- wsh-shell ---------------------------------------------------
# wsh_shell_cfg.h is a per-project config (gitignored in the submodule).
# Always sync from app/features/shell/wsh_shell_cfg.h so the library
# and app code share identical struct layout and buffer sizes.
set(_WSH_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wsh-shell/src)
configure_file(
    "${CMAKE_SOURCE_DIR}/app/features/shell/wsh_shell_cfg.h"
    "${_WSH_SRC_DIR}/wsh_shell_cfg.h"
    COPYONLY
)

file(GLOB WSH_SHELL_SOURCES CONFIGURE_DEPENDS ${_WSH_SRC_DIR}/*.c)

add_library(wsh_shell STATIC ${WSH_SHELL_SOURCES})
target_include_directories(wsh_shell PUBLIC ${_WSH_SRC_DIR})

# This is necessary to exclude the addition of wsh_wshell_cfg.h.
target_include_directories(wsh_shell PRIVATE
    ${CMAKE_SOURCE_DIR}/app/conf
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/platform/${FW_PLATFORM_MCU}/core/CMSIS
)

target_link_libraries(wsh_shell PRIVATE cpu_flags)
