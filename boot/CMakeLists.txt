set(BOOT_FULL_NAME ${FW_VER_BOOT}.${PROJECT_NAME}.boot.${FW_TYPE}.${FW_PLATFORM}.${FW_BSP}.${FW_GIT_HASH})

file(GLOB_RECURSE SOURCES_BOOT CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(boot ${SOURCES_BOOT})

target_include_directories(boot PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(boot PRIVATE
	platform
	shared
)

target_link_options(boot PRIVATE
	-T${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_boot_linker_script.ld
)

add_custom_command(TARGET boot POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:boot>
		${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${BOOT_FULL_NAME}.elf
	COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:boot>
		${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${BOOT_FULL_NAME}.hex
	COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:boot>
		${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${BOOT_FULL_NAME}.bin
)
