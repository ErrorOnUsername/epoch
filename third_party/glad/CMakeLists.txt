cmake_minimum_required( VERSION 3.4 )
project( glad )

set( SOURCES
    src/glad.c
)

add_library( glad ${SOURCES} )
