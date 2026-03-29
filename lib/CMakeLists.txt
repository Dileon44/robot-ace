# Portable data-structure libraries.
# Built as INTERFACE: headers include lib_cfg.h which is supplied by each
# consumer (app/conf/ or boot/conf/), so sources must compile in consumer context.
add_library(libs INTERFACE)

target_sources(libs INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/rand/rand.c
	${CMAKE_CURRENT_SOURCE_DIR}/ring_buff/ring_buff.c
	${CMAKE_CURRENT_SOURCE_DIR}/ring_list/ring_list.c
	${CMAKE_CURRENT_SOURCE_DIR}/shared_mutex/shared_mutex.c
)

target_include_directories(libs INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/rand
	${CMAKE_CURRENT_SOURCE_DIR}/ring_buff
	${CMAKE_CURRENT_SOURCE_DIR}/ring_list
	${CMAKE_CURRENT_SOURCE_DIR}/shared_mutex
)
