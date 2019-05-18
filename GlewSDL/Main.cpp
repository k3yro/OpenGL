#include <iostream>
#define GLEW_STATIC			//Glew DLLs statisch linken
#include <GL/glew.h>
#define SDL_MAIN_HANDLED	//Eigene main Funktion verwenden
#include <SDL.h>

// Linker - Eingabe - Zusaetzliche Abhaengigkeiten
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

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

	window = SDL_CreateWindow("First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL/*Typ - z.B. Vulcan usw.*/);
	
	//Fehlerkorrektur

	//1 Thread, 1 Context (mehrere moeglich)
	SDL_GLContext glContext = SDL_GL_CreateContext(window); // Speichert Render Status

	//Fehlerkorrektur

	//Laden aller OpenGL Erweitungen
	GLenum err = glewInit(); //benoetigt SDL_GLContext (oben)
	if (GLEW_OK != err)
	{
		std::cout << "Glew Error: " << glewGetErrorString(err) << std::endl;
		system("pause");
		return -1;
	}
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	bool close = false;

	while (!close) // GameLoop
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Loeschfarbe angeben
		glClear(GL_COLOR_BUFFER_BIT);			// Loeschen mit Loeschfarbe



		SDL_GL_SwapWindow(window); //2 Buffer (Monitorausgabe|Bildberechnung) -> Doppelpufferung

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				close = true;
			}
		}
	}
	return 0;
}