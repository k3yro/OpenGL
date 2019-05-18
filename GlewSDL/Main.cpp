#include <iostream>
#define GLEW_STATIC			//Glew DLLs statisch linken
#include <GL/glew.h>
#define SDL_MAIN_HANDLED	//Eigene main Funktion verwenden
#include <SDL.h>

// Linker - Eingabe - Zusaetzliche Abhaengigkeiten
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

int main(int argc, char** argv) 
{
	SDL_Window* window;
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL/*Typ - z.B. Vulcan usw.*/);
	
	//Fehlerkorrektur

	//1 Thread, 1 Context (mehrere moeglich)
	SDL_GLContext glContext = SDL_GL_CreateContext(window);

	//Fehlerkorrektur

	bool close = false;

	while (!close)
	{
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