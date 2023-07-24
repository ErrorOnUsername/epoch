cmake_minimum_required( VERSION 3.5 )
project( epoch )

include_directories( SYSTEM third_party/freetype/include )
include_directories( SYSTEM third_party/glad/include )
include_directories( SYSTEM third_party/glfw/include )
include_directories( SYSTEM third_party/glm )
include_directories( SYSTEM third_party/stb_image )

add_subdirectory( third_party )

set( CMAKE_EXPORT_COMPILE_COMMANDS On )
set( CMAKE_CXX_STANDARD 20 )

set( ED_SRCS
	code/buffer.cc
	code/draw_gl.cc
	code/editor.cc
	code/font.cc
	code/main.cc
	code/util.cc
	code/window.cc
)


add_executable( ${CMAKE_PROJECT_NAME} ${ED_SRCS} )
target_link_libraries( ${CMAKE_PROJECT_NAME} glfw glad stb_image freetype )

add_custom_target( run
	COMMAND ./build/epoch
	VERBATIM
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS epoch
)

