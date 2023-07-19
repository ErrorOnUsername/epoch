cmake_minimum_required( VERSION 3.5 )
project( epoch )

set( ED_SRCS
	code/draw_gl.cc
	code/main.cc
	code/window.cc
)


add_executable( ${CMAKE_PROJECT_NAME} ${ED_SRCS} )

add_custom_target( run
	COMMAND ./build/epoch
	VERBATIM
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS epoch
)

