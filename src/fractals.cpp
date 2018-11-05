/*

This code is a modified version of Shaker. The original can be found at

	https://github.com/CobaltXII/shaker

You need to link OpenGL and SDL2 in order to build Fractals. The following command should suffice 
for most systems and compilers.

	clang++ fractals.cpp -o fractals -O3 -lSDL2 -lGL

On Apple systems, you may need to use this command instead, based on your compiler vendor.

	clang++ fractals.cpp -o fractals -O3 -lSDL2 -framework OpenGL

Once you have compiled Fractals successfully, it is trivial to use it. Simply pass a filename as an
argument to Fractals. You can optionally pass a width and a height (you must specify both if you are
to specify any).

	./fractals mandelbrot.glsl

*/

#define SHAKER_DEBUG

#include <vector>
#include <utility>
#include <sstream>
#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>

/*

For some reason, Apple has deprecated OpenGL since macOS 10.14. However, the API still functions
perfectly, Apple has just dropped support for updating it.

*/

#define GL_SILENCE_DEPRECATION

#include <OpenGL/GL3.h>

/*

Vertex shader.

*/

const char gl_array_vertex_shader_source[] = 
{
    0x23, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x33, 0x33, 0x30,
    0x20, 0x63, 0x6f, 0x72, 0x65, 0x0a, 0x0a, 0x6c, 0x61, 0x79, 0x6f, 0x75,
    0x74, 0x20, 0x28, 0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20,
    0x3d, 0x20, 0x30, 0x29, 0x20, 0x69, 0x6e, 0x20, 0x76, 0x65, 0x63, 0x33,
    0x20, 0x5f, 0x5f, 0x70, 0x6f, 0x73, 0x3b, 0x0a, 0x0a, 0x76, 0x6f, 0x69,
    0x64, 0x20, 0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x0a, 0x7b, 0x0a, 0x09,
    0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x20,
    0x3d, 0x20, 0x76, 0x65, 0x63, 0x34, 0x28, 0x5f, 0x5f, 0x70, 0x6f, 0x73,
    0x2e, 0x78, 0x2c, 0x20, 0x5f, 0x5f, 0x70, 0x6f, 0x73, 0x2e, 0x79, 0x2c,
    0x20, 0x5f, 0x5f, 0x70, 0x6f, 0x73, 0x2e, 0x7a, 0x2c, 0x20, 0x31, 0x2e,
    0x30, 0x66, 0x29, 0x3b, 0x0a, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const char* gl_vertex_shader_source = (const char*)gl_array_vertex_shader_source;

/*

Load std::string from file.

*/

std::string loadfile(std::string path)
{
	std::ifstream _in_file(path);

	if (!_in_file.is_open())
	{
		std::cout << "Could not load \"" << path << "\"." << std::endl;

		exit(3);
	}

	std::stringstream _str_buffer;

	_str_buffer << _in_file.rdbuf() << "\0";

	return _str_buffer.str();
}

/*

Entry point.

*/

int main(int argc, char** argv)
{
	/*

	Parse command line input.

	*/

	int sdl_xres;
	int sdl_yres;

	const char* usr_fragment_path;

	if (argc == 2)
	{
		sdl_xres = 800;
		sdl_yres = 600;

		usr_fragment_path = argv[1];
	}
	else if (argc == 4)
	{
		sdl_xres = atoi(argv[2]);
		sdl_yres = atoi(argv[3]);

		usr_fragment_path = argv[1];
	}
	else
	{
		std::cout << "Usage: fractals <fractal-path> [width, height]" << std::endl;

		exit(7);
	}

	/*

	Initialize SDL2.

	*/

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Could not initialize SDL." << std::endl;

		exit(1);
	}

	SDL_Window* sdl_window = SDL_CreateWindow
	(
		(std::string("Fractals - ") + usr_fragment_path).c_str(),

		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,

		sdl_xres,
		sdl_yres,

		SDL_WINDOW_OPENGL |

		SDL_WINDOW_ALLOW_HIGHDPI
	);

	if (sdl_window == NULL)
	{
		std::cout << "Could not create a SDL_Window." << std::endl;

		exit(2);
	}

	/*

	Request OpenGL 3.2.

	*/

	int gl_success = 0;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/*

	Initialize OpenGL.

	*/

	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);

	if (gl_context == NULL)
	{
		std::cout << "Could not create a SDL_GLContext." << std::endl;

		exit(2);
	}

	int gl_w;
	int gl_h;

	SDL_GL_GetDrawableSize(sdl_window, &gl_w, &gl_h);

	#ifdef SHAKER_DEBUG

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "." << std::endl;

	std::cout << "OpenGL drawable size: " << gl_w << ", " << gl_h << "." << std::endl;

	#endif

	SDL_GL_SetSwapInterval(1);

	/*

	Initialize fractal driver.

	*/

	float fractal_min_re;
	float fractal_max_re;

	float fractal_min_im;
	float fractal_max_im;

	float fractal_factor_re;
	float fractal_factor_im;

	fractal_min_re = 0.0 - 2.0;
	fractal_max_re = 0.0 + 2.0;

	fractal_min_im = 0.0 - 1.2;

	fractal_max_im = fractal_min_im + (fractal_max_re - fractal_min_re) * gl_h / gl_w;

	double min_im_ = 0.0 - (fractal_max_im - fractal_min_im) / 2.0;
	double max_im_ = 0.0 + (fractal_max_im - fractal_min_im) / 2.0;

	fractal_min_im = min_im_;
	fractal_max_im = max_im_;

	fractal_factor_re = (fractal_max_re - fractal_min_re) / (gl_w - 1);
	fractal_factor_im = (fractal_max_im - fractal_min_im) / (gl_h - 1);

	double pan_re = 0.0;
	double pan_im = 0.0;

	double pan_min_re = fractal_min_re;
	double pan_min_im = fractal_min_im;

	double pan_max_re = fractal_max_re;
	double pan_max_im = fractal_max_im;

	/*

	Load and compile shaders.

	*/

	int gl_vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(gl_vertex_shader, 1, &gl_vertex_shader_source, NULL);

	glCompileShader(gl_vertex_shader);

	glGetShaderiv(gl_vertex_shader, GL_COMPILE_STATUS, &gl_success);

	if (!gl_success)
	{
		char gl_info[4096];

		glGetShaderInfoLog(gl_vertex_shader, 4096, NULL, gl_info);

		std::cout << "Could not compile vertex shader." << std::endl;

		std::cout << gl_info;

		exit(4);
	}

	std::string gl_fragment_shader_source = loadfile(usr_fragment_path);

	const GLchar* gl_char_fragment_shader_source = (const GLchar*)gl_fragment_shader_source.c_str();

	int gl_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(gl_fragment_shader, 1, &gl_char_fragment_shader_source, NULL);

	glCompileShader(gl_fragment_shader);

	glGetShaderiv(gl_fragment_shader, GL_COMPILE_STATUS, &gl_success);

	if (!gl_success)
	{
		char gl_info[4096];

		glGetShaderInfoLog(gl_fragment_shader, 4096, NULL, gl_info);

		std::cout << "Could not compile fragment shader." << std::endl;

		std::cout << gl_info;

		exit(5);
	}

	/*

	Link shaders.

	*/

	int gl_shader_program = glCreateProgram();

	glAttachShader(gl_shader_program, gl_vertex_shader);

	glAttachShader(gl_shader_program, gl_fragment_shader);

	glLinkProgram(gl_shader_program);

	glGetProgramiv(gl_shader_program, GL_LINK_STATUS, &gl_success);

	if (!gl_success)
	{
		char gl_info[4096];

		glGetProgramInfoLog(gl_shader_program, 4096, NULL, gl_info);

		std::cout << "Could not link vertex and fragment shaders." << std::endl;

		std::cout << gl_info;

		exit(6);
	}

	/*

	Delete shaders.

	*/

	glDeleteShader(gl_vertex_shader);

	glDeleteShader(gl_fragment_shader);

	/*

	Define quadrant vertices and indices.

	*/

	float gl_vertices[] =
	{
		0.0f + 1.0f, 0.0f + 1.0f, 0.0f + 0.0f,
		0.0f + 1.0f, 0.0f - 1.0f, 0.0f + 0.0f,
		0.0f - 1.0f, 0.0f - 1.0f, 0.0f + 0.0f,
		0.0f - 1.0f, 0.0f + 1.0f, 0.0f + 0.0f,
	};

	unsigned int gl_indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	/*

	Generate the vertex buffer object, vertex array object, and element buffer object.

	*/

	unsigned int gl_vbo;
	unsigned int gl_vao;
	unsigned int gl_ebo;

	glGenVertexArrays(1, &gl_vao);

	glGenBuffers(1, &gl_vbo);
	glGenBuffers(1, &gl_ebo);

	glBindVertexArray(gl_vao);

	glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(gl_vertices), gl_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ebo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gl_indices), gl_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(gl_shader_program);

	/*

	Enter main loop.

	*/

	Uint32 sdl_mouse_x = 0;
	Uint32 sdl_mouse_y = 0;

	SDL_bool sdl_mouse_l = SDL_FALSE;
	SDL_bool sdl_mouse_r = SDL_FALSE;

	SDL_bool sdl_mouse_ol = SDL_FALSE;
	SDL_bool sdl_mouse_or = SDL_FALSE;

	Uint32 sdl_iteration = 0;

	SDL_bool sdl_running = SDL_TRUE;

	while (sdl_running == SDL_TRUE)
	{
		Uint32 sdl_time_start = SDL_GetTicks();

		sdl_mouse_ol = sdl_mouse_l;
		sdl_mouse_or = sdl_mouse_r;

		/*

		Handle events.

		*/

		SDL_Event e;

		Sint32 sdl_wheel = 0;

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				sdl_running = SDL_FALSE;
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				sdl_mouse_x = e.motion.x;
				sdl_mouse_y = e.motion.y;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					sdl_mouse_l = SDL_TRUE;
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)
				{
					sdl_mouse_r = SDL_TRUE;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					sdl_mouse_l = SDL_FALSE;
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)
				{
					sdl_mouse_r = SDL_FALSE;
				}
			}
			else if (e.type == SDL_KEYDOWN)
			{
				SDL_Keycode sdl_key = e.key.keysym.sym;

				if (sdl_key == SDLK_ESCAPE)
				{
					sdl_running = SDL_FALSE;
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				if (e.wheel.y != 0)
				{
					sdl_wheel = e.wheel.y;
				}
			}
		}

		/*

		Handle zooming.

		*/

		if (sdl_wheel && !sdl_mouse_ol && !sdl_mouse_l)
		{
			double center_re = (pan_min_re + (pan_max_re - pan_min_re) / 2.0);
			double center_im = (pan_min_im + (pan_max_im - pan_min_im) / 2.0);

			double zoom = 1.05;

			if (sdl_wheel < 0)
			{
				pan_min_re = (pan_min_re - center_re) / zoom + center_re;
				pan_max_re = (pan_max_re - center_re) / zoom + center_re;

				pan_min_im = (pan_min_im - center_im) / zoom + center_im;
				pan_max_im = (pan_max_im - center_im) / zoom + center_im;
			}
			else
			{
				pan_min_re = (pan_min_re - center_re) * zoom + center_re;
				pan_max_re = (pan_max_re - center_re) * zoom + center_re;

				pan_min_im = (pan_min_im - center_im) * zoom + center_im;
				pan_max_im = (pan_max_im - center_im) * zoom + center_im;
			}

			pan_max_im = pan_min_im + (pan_max_re - pan_min_re) * gl_h / gl_w;

			fractal_min_re = pan_min_re;
			fractal_max_re = pan_max_re;

			fractal_min_im = pan_min_im;
			fractal_max_im = pan_max_im;

			fractal_factor_re = (fractal_max_re - fractal_min_re) / (gl_w - 1);
			fractal_factor_im = (fractal_max_im - fractal_min_im) / (gl_h - 1);
		}

		/*

		Handle panning.

		*/

		double c_re = pan_min_re + sdl_mouse_x * 2 * fractal_factor_re;
		double c_im = pan_min_im + sdl_mouse_y * 2 * fractal_factor_im;

		if (!sdl_mouse_ol && sdl_mouse_l)
		{
			pan_re = c_re;
			pan_im = c_im;

			pan_min_re = fractal_min_re;
			pan_max_re = fractal_max_re;

			pan_min_im = fractal_min_im;
			pan_max_im = fractal_max_im;
		}
		else if (sdl_mouse_ol && sdl_mouse_l)
		{
			double inc_re = c_re - pan_re;
			double inc_im = c_im - pan_im;

			fractal_min_re = pan_min_re - inc_re;
			fractal_max_re = pan_max_re - inc_re;

			fractal_min_im = pan_min_im - inc_im;
			fractal_max_im = pan_max_im - inc_im;
		}
		else if (sdl_mouse_ol && !sdl_mouse_l)
		{
			pan_min_re = fractal_min_re;
			pan_max_re = fractal_max_re;

			pan_min_im = fractal_min_im;
			pan_max_im = fractal_max_im;
		}

		/*

		Set up variables.

		*/

		int gl_i_resolution_location = glGetUniformLocation(gl_shader_program, "glx_Resolution");

		glUniform2f(gl_i_resolution_location, (float)gl_w, (float)gl_h);

		int gl_i_real_location = glGetUniformLocation(gl_shader_program, "glx_Real");

		glUniform2f(gl_i_real_location, fractal_min_re, fractal_max_re);

		int gl_i_imag_location = glGetUniformLocation(gl_shader_program, "glx_Imag");

		glUniform2f(gl_i_imag_location, fractal_min_im, fractal_max_im);

		int gl_i_factor_location = glGetUniformLocation(gl_shader_program, "glx_Factor");

		glUniform2f(gl_i_factor_location, fractal_factor_re, fractal_factor_im);

		/*

		Draw quadrant.

		*/

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(sdl_window);

		/*

		Cap framerate.

		*/

		Uint32 sdl_time_elapsed = SDL_GetTicks() - sdl_time_start;

		#ifdef SHAKER_DEBUG

		if (sdl_iteration % 1 == 0)
		{
			std::cout << "Framerate: " << (1000.0 / sdl_time_elapsed) << std::string(16, ' ') << '\r' << std::flush;
		}
		
		#endif

		if (sdl_time_elapsed < 1000.0 / 60.0)
		{
			SDL_Delay((Uint32)((Uint32)(1000.0 / 60.0) - sdl_time_elapsed));
		}

		sdl_iteration++;
	}

	/*

	Clean up SDL2 and OpenGL.

	*/

	glDeleteVertexArrays(1, &gl_vao);

	glDeleteBuffers(1, &gl_vbo);
	glDeleteBuffers(1, &gl_ebo);

	SDL_GL_DeleteContext(gl_context);

	SDL_DestroyWindow(sdl_window);

	SDL_Quit();

	/*

	Exit cleanly.

	*/

	return 0;
}