#include <iostream>
#define GLEW_STATIC			//Glew DLLs statisch linken
#include <GL/glew.h>
#define SDL_MAIN_HANDLED	//Eigene Main Funktion verwenden
#include <SDL.h>
#include <Windows.h>
#include <sstream>

// Linker - Eingabe - Zusaetzliche Abhaengigkeiten
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

#include "vertex_buffer.h"
#include "index_buffer.h"
#include "defines.h"
#include "shader.h"

// Neue OpenGl Debug Variante:
std::string lastErrorMessage = "";
void APIENTRY openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	//if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
	//{
	if (lastErrorMessage != (std::string)message)
	{
		lastErrorMessage = (std::string)message;
		std::cout << "[OpenGL_New Error] " << message << std::endl;
	}
	//}	
}

// Alte OpenGl Debug Variante (GLCALL):
#ifdef _DEBUG
std::string lastErrorFile = "";
void _GLGetError(const char* file, int line, const char* call) {
	std::stringstream ss;
	ss << file << line;
	if (ss.str() != lastErrorFile)
	{
		lastErrorFile = ss.str();
		while (GLenum error = glGetError()) {
			std::cout << "[OpenGL_Old Error] " << glewGetErrorString(error) << " in " << file << ":" << line << " Call: " << call << std::endl;
		}

	}
	ss.str(std::string());
}
#define GLCALL(call) call; _GLGetError(__FILE__, __LINE__, #call)
#else
#define GLCALL(call) call
#endif

// OpenGL Befehle nachschlagen: http://docs.gl
//TODO: Debug -> mit Konsole | Release -> nur Fenster
int main(int argc, char** argv) 
{
	SDL_Window* window;
	SDL_Init(SDL_INIT_EVERYTHING);

	//Aufbau FrameBuffer(4x8 -> 1Pixel = 32bit):
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32/*bit*/);	// Optional (minimale Buffergroesse)
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1/*An*/);	// Doppelpufferung einschalten

#ifdef _DEBUG
	// Spezieller Debug Mode (Verlangsamung)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	//uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP;
	//uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP;
	uint32_t flags = SDL_WINDOW_OPENGL;

	window = SDL_CreateWindow("First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
	
	//Todo: Fehlerkorrektur window

	//1 Thread, 1 Context (mehrere moeglich)
	SDL_GLContext glContext = SDL_GL_CreateContext(window); // Speichert Render Status

	//Todo: Fehlerkorrektur glContext

	//Laden aller OpenGL Erweitungen
	GLenum err = glewInit(); //benoetigt SDL_GLContext (oben)
	if (GLEW_OK != err)
	{
		std::cout << "Glew Error: " << glewGetErrorString(err) << std::endl;
		system("pause");
		return -1;
	}
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLDebugCallback, 0);
#endif

	//Daten fuer Dreieck:
	Vertex verticies[] = {
		Vertex{-0.5f, -0.5f, 0.0f,
				1.0f, 0.0f, 0.0f, 1.0f},
		Vertex{-0.5f, 0.5f, 0.0f,
				0.0f, 1.0f, 0.0f, 1.0f},
		Vertex{0.5f, -0.5f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f},
		Vertex{0.5f, 0.5f, 0.0f,
				1.0f, 0.0f, 0.0f, 1.0f}
	};
	uint32_t countVerticies = 4; // Anzahl Dreiecke in verticies Array

	//Index fuer komplexere Formen
	uint32_t indices[] = {
		0, 1, 2,
		1, 2, 3
	};
	uint32_t numIndices = 6;

	IndexBuffer indexBuffer(indices, numIndices, sizeof(indices[0]));

	VertexBuffer vertexBuffer(verticies, countVerticies);
	vertexBuffer.Unbind();

	Shader shader("basic.vs.txt", "basic.fs.txt");
	shader.bind();

	//Zeit messen:
	uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64_t lastCounter = SDL_GetPerformanceCounter();
	float delta = 0.0f;

	//Wireframe Modus
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal

	bool close = false;
	while (!close) // GameLoop
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Loeschfarbe angeben
		glClear(GL_COLOR_BUFFER_BIT);			// Loeschen mit Loeschfarbe

		
		vertexBuffer.Bind();
		indexBuffer.bind();
		GLCALL(glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0));
		indexBuffer.unbind();
		vertexBuffer.Unbind();

		SDL_GL_SwapWindow(window); //2 Buffer (Monitorausgabe|Bildberechnung) -> Doppelpufferung

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				close = true;
			}
		}

		//Zeit messen:
		uint64_t endCounter = SDL_GetPerformanceCounter();
		uint64_t counterElapsed = endCounter - lastCounter;
		delta = ((float)counterElapsed) / (float)perfCounterFrequency;
		uint32_t FPS = (uint32_t)((float)perfCounterFrequency / (float)counterElapsed);
		//std::cout << FPS << std::endl;
		lastCounter = endCounter;
	}
	return 0;
}