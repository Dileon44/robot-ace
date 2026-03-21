# Portable data-structure libraries (ring_buff, ring_list).
# Sources depend on wsh_emblib.h which is not yet in the repository —
# convert to STATIC and add cpu_flags when that dependency is resolved.
add_library(libs INTERFACE)

target_include_directories(libs INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)
