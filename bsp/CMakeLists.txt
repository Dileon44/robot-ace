# BSP — board support package.
# Built as INTERFACE: headers include bsp_cfg.h and main.h which are supplied
# by the consumer (app/conf/ and app/), so sources compile in consumer context.
add_library(bsp INTERFACE)

target_sources(bsp INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/bsp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/bsp_module.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/encoder/encoder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/encoder/as5600/as5600.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/encoder/as5600/as5600_bus.cpp
)

target_include_directories(bsp INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/encoder
	${CMAKE_CURRENT_SOURCE_DIR}/encoder/as5600
)
