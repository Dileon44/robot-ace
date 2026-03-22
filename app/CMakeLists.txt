set(APP_FULL_NAME ${FW_VER_APP}.${PROJECT_NAME}.app.${FW_TYPE}.${FW_PLATFORM}.${FW_BSP}.${FW_GIT_HASH})

file(GLOB_RECURSE SOURCES_APP CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(app ${SOURCES_APP})

target_include_directories(app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(app PRIVATE
	platform
	shared
	libs
	wsh_shell
	freertos_kernel
)

target_link_options(app PRIVATE
	-u _printf_float
	-T${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_app_linker_script.ld
)

set_target_properties(app PROPERTIES
	OUTPUT_NAME ${APP_FULL_NAME}.elf
	VERSION     ${FW_VER_APP}
)

add_custom_command(TARGET app POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:app>
		${CMAKE_SOURCE_DIR}/${BUILD_DIR}/fw_${FW_PLATFORM_MCU}.elf
	COMMAND ${CMAKE_OBJCOPY} -O ihex  $<TARGET_FILE:app>
		${CMAKE_SOURCE_DIR}/${BUILD_DIR}/fw_${FW_PLATFORM_MCU}.hex
	COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:app>
		${CMAKE_SOURCE_DIR}/${BUILD_DIR}/fw_${FW_PLATFORM_MCU}.bin
)
