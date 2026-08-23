# Portable data-structure libraries + the log module.
# Built as INTERFACE: headers include lib_cfg.h / log_cfg.h which are supplied by
# each consumer (app/conf/ or boot/conf/), so sources must compile in consumer
# context. Only app links this target — boot has no log transport.
add_library(libs INTERFACE)

target_sources(libs INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/log/log.c
	${CMAKE_CURRENT_SOURCE_DIR}/log/log_io.c
	${CMAKE_CURRENT_SOURCE_DIR}/log/log_root.c
	${CMAKE_CURRENT_SOURCE_DIR}/rand/rand.c
	${CMAKE_CURRENT_SOURCE_DIR}/ring_buff/ring_buff.c
	${CMAKE_CURRENT_SOURCE_DIR}/ring_list/ring_list.c
	${CMAKE_CURRENT_SOURCE_DIR}/shared_mutex/shared_mutex.c
)

target_include_directories(libs INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/log
	${CMAKE_CURRENT_SOURCE_DIR}/rand
	${CMAKE_CURRENT_SOURCE_DIR}/ring_buff
	${CMAKE_CURRENT_SOURCE_DIR}/ring_list
	${CMAKE_CURRENT_SOURCE_DIR}/shared_mutex
)
