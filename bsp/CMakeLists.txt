# BSP — board support package.
# Built as INTERFACE: headers include bsp_cfg.h and main.h which are supplied
# by the consumer (app/conf/ and app/), so sources compile in consumer context.
add_library(bsp INTERFACE)

target_sources(bsp INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/bsp.c
	${CMAKE_CURRENT_SOURCE_DIR}/encoder_m/encoder_m.c
	${CMAKE_CURRENT_SOURCE_DIR}/encoder_m/as5600/encoder_m_as5600.c
	${CMAKE_CURRENT_SOURCE_DIR}/encoder_m/as5600/encoder_m_as5600__interface.c
)

target_include_directories(bsp INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/encoder_m
	${CMAKE_CURRENT_SOURCE_DIR}/encoder_m/as5600
)
