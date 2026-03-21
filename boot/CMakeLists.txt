set(BOOT_FULL_NAME ${FW_VER_BOOT}.${PROJECT_NAME}.boot.${FW_TYPE}.${FW_PLATFORM}.${FW_BSP}.${FW_GIT_HASH})

file(GLOB_RECURSE SOURCES_BOOT CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(boot.elf ${SOURCES_BOOT})

target_include_directories(boot.elf PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(boot.elf PRIVATE
	platform
	shared
)

target_link_options(boot.elf PRIVATE
	-T${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${FW_PLATFORM}_boot_linker_script.ld
)

set_target_properties(boot.elf PROPERTIES
	OUTPUT_NAME ${BOOT_FULL_NAME}.elf
	VERSION     ${FW_VER_BOOT}
)
