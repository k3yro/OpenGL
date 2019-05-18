#include <iostream>
#define GLEW_STATIC			//Glew DLLs statisch linken
#include <GL/glew.h>
#define SDL_MAIN_HANDLED	//Eigene Main Funktion verwenden
#include <SDL.h>

// Linker - Eingabe - Zusaetzliche Abhaengigkeiten
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

struct Vertex // Punkt im Raum
{
	//Position:
	float x;
	float y;
	float z;
	//Reihenfolge x,y,z nie aendern
};

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

	window = SDL_CreateWindow("First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL/*Typ - z.B. Vulcan usw.*/);
	
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
		Vertex{-0.5f, -0.5f, 0.0f},
		Vertex{0.0f, 0.5f, 0.0f},
		Vertex{0.5f, -0.5f, 0.0f}
	};
	uint32_t countVerticies = 3; // Anzahl Dreiecke in verticies Array

	//Buffer, um das Dreieck in Grafikkartenspeicher zu legen
	GLuint vertexBuffer; // Index/ID (Zeigerersatz)
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer/*ID*/); // Buffer als Array betrachten
	//Buffer ist jetzt gebunden

	//Daten an Buffer senden 
	glBufferData(GL_ARRAY_BUFFER/*Typ*/, countVerticies * sizeof(Vertex)/*Groesse*/, verticies, GL_STATIC_DRAW);

	//Daten erlaeutern (bleibt wie glBindBuffer gesetzt bis man es aendert):
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,							// Index
		3,							// Anzahl (x,y,z)
		GL_FLOAT,					// Typ
		GL_FALSE,					// Normalisierung z.B. 255 in 1.0f umwandeln
		sizeof(Vertex),				// Groesse
		0//offsetof(struct Vertex,x)// Wo beginnt erste Koordinate
	);

	bool close = false;
	while (!close) // GameLoop
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Loeschfarbe angeben
		glClear(GL_COLOR_BUFFER_BIT);			// Loeschen mit Loeschfarbe

		/* Veraltet:
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.0f, 0.5f);
		glVertex2f(0.5f, -0.5f);
		glEnd();
		*/

		//Gebundener Buffer (glBindBuffer) wird gezeichnet
		glDrawArrays(GL_TRIANGLES, 0/*Kompletten Buffer zeichnen*/, countVerticies);

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