add_subdirectory(freertos_kernel ${CMAKE_BINARY_DIR}/freertos_kernel)

# Provide MCU-specific compile flags to FreeRTOS kernel targets
target_link_libraries(freertos_kernel      PRIVATE cpu_flags)
target_link_libraries(freertos_kernel_port PRIVATE cpu_flags)
