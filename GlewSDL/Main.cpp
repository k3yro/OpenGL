#include <iostream>
#define GLEW_STATIC			//Glew DLLs statisch linken
#include <GL/glew.h>
#define SDL_MAIN_HANDLED	//Eigene Main Funktion verwenden
#include <SDL.h>

// Linker - Eingabe - Zusaetzliche Abhaengigkeiten
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

#include "vertex_buffer.h"
#include "defines.h"
#include "shader.h"

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
		glDrawArrays(GL_TRIANGLE_STRIP, 0, countVerticies);
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