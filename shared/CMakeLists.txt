# INTERFACE library: sources are compiled in the context of each consuming target.
# Required because syscalls.c includes "main.h" (app/main.h or boot/main.h)
# and delay.c includes "platform.h" — both depend on the consumer's include paths.
add_library(shared INTERFACE)

target_include_directories(shared INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(shared INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/def_types.c
	${CMAKE_CURRENT_SOURCE_DIR}/delay.c
	${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
)
