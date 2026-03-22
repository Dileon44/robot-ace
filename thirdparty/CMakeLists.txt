add_subdirectory(freertos_kernel ${CMAKE_BINARY_DIR}/freertos_kernel)

# Provide MCU-specific compile flags to FreeRTOS kernel targets
target_link_libraries(freertos_kernel      PRIVATE cpu_flags)
target_link_libraries(freertos_kernel_port PRIVATE cpu_flags)

# --- wsh-shell ---------------------------------------------------
# wsh_shell_cfg.h is a per-project config (gitignored in the submodule).
# Generate it from the bundled defaults on first configure if absent.
set(_WSH_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wsh-shell/src)
if(NOT EXISTS "${_WSH_SRC_DIR}/wsh_shell_cfg.h")
    file(COPY_FILE
        "${_WSH_SRC_DIR}/wsh_shell_cfg_def.h"
        "${_WSH_SRC_DIR}/wsh_shell_cfg.h"
    )
    message(STATUS "wsh-shell: generated default wsh_shell_cfg.h")
endif()

file(GLOB WSH_SHELL_SOURCES CONFIGURE_DEPENDS ${_WSH_SRC_DIR}/*.c)

add_library(wsh_shell STATIC ${WSH_SHELL_SOURCES})
target_include_directories(wsh_shell PUBLIC ${_WSH_SRC_DIR})
target_link_libraries(wsh_shell PRIVATE cpu_flags)
