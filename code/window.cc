#include "window.hh"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#define GL_VERSION_MAJOR 4
#define GL_VERSION_MINOR 0


static GLFWwindow* s_window = nullptr;
static int         s_width  = 0;
static int         s_height = 0;


WindowResult window_init( int width, int height, char const* title )
{
	if ( !glfwInit() )
	{
		return WindowResult_GLFWInitFailed;
	}

#ifdef __APPLE__
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#endif

	s_window = glfwCreateWindow( width, height, title, nullptr, nullptr );
	if ( !s_window )
	{
		return WindowResult_WindowCreateFailed;
	}

	s_width  = width;
	s_height = height;

	glfwMakeContextCurrent( s_window );

	int glad_init_result = gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress );
	if ( !glad_init_result )
	{
		return WindowResult_ContextCreateFailed;
	}

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_DEPTH_TEST );

	glfwSwapInterval( 1 );
	glViewport( 0, 0, width, height );

	return WindowResult_Success;
}


void window_deinit()
{
	glfwDestroyWindow( s_window );
	glfwTerminate();
}


void window_poll_events()
{
	glfwPollEvents();
}


void window_swap_buffers()
{
	glfwSwapBuffers( s_window );
}


bool window_should_close()
{
	return glfwWindowShouldClose( s_window );
}


void window_set_title( char const* title )
{
	glfwSetWindowTitle( s_window, title );
}


float window_get_width()
{
	int width = 1;
	glfwGetFramebufferSize( s_window, &width, nullptr );

	return (float)width;
}

float window_get_height()
{
	int height = 1;
	glfwGetFramebufferSize( s_window, nullptr, &height );

	return (float)height;
}


Vec2 window_get_scale()
{
	float xscale, yscale;
	glfwGetWindowContentScale( s_window, &xscale, &yscale );

	return { xscale, yscale };
}


static_assert( WindowResult_Count == 4 );
static char const* s_window_result_strings[WindowResult_Count] =
{
	"Success",
	"glfwInit() Failed",
	"Could not create the window",
	"GL context creation failed",
};


char const* window_result_as_str( WindowResult result )
{
	return s_window_result_strings[result];
}
