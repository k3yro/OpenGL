#include <iostream>
#define GLEW_STATIC			// Glew DLLs statisch linken
#include <GL/glew.h>
#define SDL_MAIN_HANDLED	// Eigene Main Funktion verwenden

#include <glm/glm.hpp>		// Mathe... -.-
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL.h>
#include <Windows.h>
#include <sstream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

	// Aufbau FrameBuffer(4x8 -> 1Pixel = 32bit):
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8/*bit*/);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32/*bit*/);	// Optional (minimale Buffergroesse)
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1/*An*/);	// Doppelpufferung einschalten
	
	// Vsync: 0 = Aus (Screen Tearing Gefahr), 
	// 1 = Ein (Grafikkarte wartet bis Frame auf Monitor fertig gezeichnet ist), 
	// -1 = AdaptivSync (Monitor(g-sync/freesync faehig) wartet auf Grafikkarte)
	SDL_GL_SetSwapInterval(1); // Hardwarevoraussetzungen sind mit SDL-Befehlen abfragbar

#ifdef _DEBUG
	// Spezieller Debug Mode (Verlangsamung)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	//uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP;
	//uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP;
	uint32_t flags = SDL_WINDOW_OPENGL;

	window = SDL_CreateWindow("First Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);

	// 1 Thread, 1 Context (mehrere moeglich)
	SDL_GLContext glContext = SDL_GL_CreateContext(window); // Speichert Render Status

	// Laden aller OpenGL Erweitungen
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

	// Daten fuer Dreieck:
	Vertex verticies[] = {
		Vertex{-0.5f, -0.5f, 0.0f,
				0.0f, 0.0f,
				1.0f, 0.0f, 0.0f, 1.0f},
		Vertex{-0.5f, 0.5f, 0.0f,
				0.0f, 1.0f,
				0.0f, 1.0f, 0.0f, 1.0f},
		Vertex{0.5f, -0.5f, 0.0f,
				1.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f},
		Vertex{0.5f, 0.5f, 0.0f,
				1.0f, 1.0f,
				1.0f, 0.0f, 0.0f, 1.0f}
	};
	uint32_t countVerticies = 4; // Anzahl Dreiecke in verticies Array

	// Index fuer komplexere Formen
	uint32_t indices[] = {
		0, 1, 2,
		1, 2, 3
	};
	uint32_t numIndices = 6;

	IndexBuffer indexBuffer(indices, numIndices, sizeof(indices[0]));

	VertexBuffer vertexBuffer(verticies, countVerticies);
	vertexBuffer.Unbind();

	// Textur
	int32_t textureWidth = 0;
	int32_t terxtureHeight = 0;
	int32_t bitsPerPixel = 0;
	stbi_set_flip_vertically_on_load(true); // sonst Textur auf den Kopf
	auto texturBuffer = stbi_load("Texturen/Textur.png", &textureWidth, &terxtureHeight, &bitsPerPixel, 4/*Kanaele*/);

	GLuint textureId;
	GLCALL(glGenTextures(1, &textureId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, textureId));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER/*z.b. bei Kamerabewegung*/, GL_LINEAR)); // GL_MIPMAP = nutze verschieden Aufloesungen
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER/*nah dran*/, GL_LINEAR/*verschwommen/verwaschen*/)); // GL_NEAREST = Minecraft / PS2
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE/*Am Rand der Textur abschneiden (statt z.B. kacheln)*/));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0/*level(bitmaps)*/, GL_RGBA8/*Format*/, textureWidth, terxtureHeight, 0/*boarder?*/, GL_RGBA/*nochmal Format, aber ohne 8bit!?*/, GL_UNSIGNED_BYTE/*Typ Farbkanaele*/, texturBuffer));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

	if (texturBuffer) {
		stbi_image_free(texturBuffer);
	}


	Shader shader("basic.vs.txt", "basic.fs.txt");
	shader.bind();

	// Zeit messen:
	uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64_t lastCounter = SDL_GetPerformanceCounter();
	float delta = 0.0f;

	// Uniform - Shader muss geladen sein
	int colorUniformLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_color"));
	if (!colorUniformLocation != -1) {
		GLCALL(glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f));
	}

	// Textur:
	int textureUniformLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_texture"));
	if (!textureUniformLocation != -1) {
		GLCALL(glUniform1i(textureUniformLocation, 0));
	}

	// Pinguin drehen:
	glm::mat4 model = glm::mat4(1.0f); // Einheitsmatrix (nichts passiert)
	model = glm::scale(model, glm::vec3(1.0f/*x*/, 1.0f/*y*/, 1.0f/*z*/)); // Skalieren

	bool ortho = false; // Ingame Perspektive wechseln

	// Orthogonale Projektion
	glm::mat4 projectionOrtho = glm::ortho(-2.26f, 2.26f,/*4zu3*/ -1.7f, 1.7f, -10.0f, 100.0f); // Auflösungsabhaengig

	// Perspektivische Projektion
	glm::mat4 projectionPersp = glm::perspective(glm::radians(45.0f)/*Aufnahme-Winkel*/, 4.0f / 3.0f/*hier 4zu3, sonst Aufloesungsabhaengig*/, 0.1f/*Near Sichtweite*/, 100.0f/*Far Sichtweite (richtiges Spiel 1000 oder mehr)*/);

	// Initial Perspektive
	glm::mat4 projection = projectionPersp;

	// Kamera verschieben
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f/*n Einheiten entfernt*/));

	// Projektion berechnen
	glm::mat4 modelViewProj = projection * view * model;

	int modelMatrixLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_modelViewProj"));

	// Wireframe Modus
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal

	float time = 0.0f;
	bool close = false;
	while (!close) // GameLoop
	{
		// Events abfragen
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// Beenden
			if (event.type == SDL_QUIT)
			{
				close = true;
			}
			// Perspektive wechseln
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_p/*Taste P*/ && event.key.keysym.mod & KMOD_LCTRL/*sowie gedrueckte linke Steuerungstaste*/)
				{
					ortho = !ortho;
					if (ortho)
					{
						projection = projectionOrtho;
						std::cout << "Kamerawechsel zu Orthogonal!" << std::endl;
					}
					else
					{
						projection = projectionPersp;
						std::cout << "Kamerawechsel zu Perspektive!" << std::endl;
					}
				}
			}
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Loeschfarbe angeben
		glClear(GL_COLOR_BUFFER_BIT);			// Loeschen mit Loeschfarbe
		time += delta; // delta ist die Zeit, die seit dem letzten Frame vergangen ist!

		// Echte Rotation:
		model = glm::rotate(model, 1.0f * delta, glm::vec3(0, 1/*y*/, 0)/*Achse um die rotiert werden soll*/);
		modelViewProj = projection * view * model;

		// Fake Rotation (mit scale):
		//model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(sinf(time), 1, 1));

		// Pulsierende Farbe
		if (!colorUniformLocation != -1) {
			GLCALL(glUniform4f(colorUniformLocation, 1.0f, 1.0f, sinf(time)* sinf(time), 1.0f));
		}

		vertexBuffer.Bind();
		indexBuffer.bind();
		GLCALL(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]));
		GLCALL(glActiveTexture(GL_TEXTURE0));
		GLCALL(glBindTexture(GL_TEXTURE_2D, textureId));
		GLCALL(glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0));
		indexBuffer.unbind();
		vertexBuffer.Unbind();

		SDL_GL_SwapWindow(window); // 2 Buffer (Monitorausgabe|Bildberechnung) -> Doppelpufferung

		// Zeit messen:
		uint64_t endCounter = SDL_GetPerformanceCounter();
		uint64_t counterElapsed = endCounter - lastCounter;
		delta = ((float)counterElapsed) / (float)perfCounterFrequency;
		uint32_t FPS = (uint32_t)((float)perfCounterFrequency / (float)counterElapsed);
		//std::cout << FPS << std::endl;
		lastCounter = endCounter;

	}
	GLCALL(glDeleteTextures(1, &textureId));
	return 0;
}