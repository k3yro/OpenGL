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
#include <vector>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Linker - Eingabe - Zusaetzliche Abhaengigkeiten
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

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

#include "vertex_buffer.h"
#include "index_buffer.h"
#include "defines.h"
#include "shader.h"
#include "Kamera.hpp"
#include "KameraFPS.hpp"
#include "KameraFloating.hpp"
#include "Mesh.hpp"

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

	SDL_SetRelativeMouseMode(SDL_TRUE); // Maus fangen/verbergen

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

	Shader shader("basic.vs.txt", "basic.fs.txt");
	shader.bind();

	int directionLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_directional_light.direction"));
	glm::vec3 sunColor = glm::vec3(0.8f);
	glm::vec3 sunDirection = glm::vec3(-1.0f);
	GLCALL(glUniform3fv(glGetUniformLocation(shader.getShaderId(), "u_directional_light.diffuse"), 1, (float*)& sunColor.x));
	GLCALL(glUniform3fv(glGetUniformLocation(shader.getShaderId(), "u_directional_light.specular"), 1, (float*)& sunColor.x));
	sunColor *= 0.4f;
	GLCALL(glUniform3fv(glGetUniformLocation(shader.getShaderId(), "u_directional_light.ambient"), 1, (float*)& sunColor.x));

	//Model monkey;
	//monkey.init("Models/tree.bmf", &shader);
	Model tree01;
	tree01.init("Models/hubschrauber.bmf", &shader);

	Model tree02;
	tree02.init("Models/tree02.bmf", &shader);
	

	// Zeit messen:
	uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64_t lastCounter = SDL_GetPerformanceCounter();
	float delta = 0.0f;

	// Pinguin drehen:
	glm::mat4 model = glm::mat4(1.0f); // Einheitsmatrix (nichts passiert)
	model = glm::scale(model, glm::vec3(1.0f)); // Skalieren

	//TODO: Echte Aufloesung mit SDL abfragen
	//KameraFPS camera(90.0f/*Grad*/, 800.0f, 600.0f);
	KameraFloating camera(90.0f/*Grad*/, 800.0f, 600.0f);

	// Kamera bewegen:
	camera.translate(glm::vec3(0.0f, 0.0f, 5.0f));

	// ViewProjektionMatrix updaten
	camera.update();

	// Projektion berechnen
	glm::mat4 modelViewProj = camera.getViewProj() * model;

	int modelMatrixLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_modelViewProj"));
	int modelViewLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_modelView"));
	int invModelViewLocation = GLCALL(glGetUniformLocation(shader.getShaderId(), "u_invModelView"));


	// Wireframe Modus
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal

	// WASD
	bool buttonW = false;
	bool buttonA = false;
	bool buttonS = false;
	bool buttonD = false;
	bool buttonSpace = false;
	bool buttonShift = false;

	float cameraSpeed = 6.0f; // 5-6 m/s (ueblicher Wert zum Laufen)
	float time = 0.0f;
	bool close = false;

	// Nur Vorderseite der Dreiecke zeichnen (Culling)
	//GLCALL(glEnable(GL_CULL_FACE)); // glEnable - OpenGL Funktionen anschalten

	// Dreiecks Vorder/Rückseite drehen
	//GLCALL(glFrontFace(GL_CW)); // GL_CCW -> Dreiecke gege Urzeigersinn zeichnen (GL_CW -> gegen Uhrzeigersinn)

	// Deep Buffer anschalten
	GLCALL(glEnable(GL_DEPTH_TEST)); // Verdeckte Pixel nicht zeichnen (unbedingt clear Buffer in GameLoop aufrufen) 

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
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					buttonW = true;
					break;
				case SDLK_a:
					buttonA = true;
					break;
				case SDLK_s:
					buttonS = true;
					break;
				case SDLK_d:
					buttonD = true;
					break;
				case SDLK_SPACE:
					buttonSpace = true;
					break;
				case SDLK_LSHIFT:
					buttonShift = true;
					break;
				case SDLK_ESCAPE:
					SDL_SetRelativeMouseMode(SDL_FALSE);
					break;
					
				case SDLK_p:
					if (event.key.keysym.mod & KMOD_LCTRL)
					{
						//Pause ?
					}
					break;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					buttonW = false;
					break;
				case SDLK_a:
					buttonA = false;
					break;
				case SDLK_s:
					buttonS = false;
					break;
				case SDLK_d:
					buttonD = false;
					break;
				case SDLK_SPACE:
					buttonSpace = false;
					break;
				case SDLK_LSHIFT:
					buttonShift = false;
					break;
				}
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				if (SDL_GetRelativeMouseMode())
				{
					camera.onMouseMoved(event.motion.xrel, event.motion.yrel);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
			}
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Loeschfarbe angeben
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// Loeschen mit Loeschfarbe
		time += delta; // delta ist die Zeit, die seit dem letzten Frame vergangen ist!

		// Kamera bewegen:
		if (buttonW)
		{
			camera.moveFront(delta* cameraSpeed);
		}
		if (buttonA)
		{
			camera.moveSideway(-delta* cameraSpeed);
		}
		if (buttonS)
		{
			camera.moveFront(-delta* cameraSpeed);
		}
		if (buttonD)
		{
			camera.moveSideway(delta* cameraSpeed);
		}
		if (buttonSpace)
		{
			camera.moveUp(delta* cameraSpeed);
		}
		if (buttonShift)
		{
			camera.moveUp(-delta* cameraSpeed);
		}


		camera.update();
		

		// Echte Rotation:
		//model = glm::rotate(model, 1.0f * delta, glm::vec3(0, 1/*y*/, 0)/*Achse um die rotiert werden soll*/);

		modelViewProj = camera.getViewProj() * model;
		glm::mat4 modelView = camera.getView() * model;
		glm::mat4 invModelView = glm::transpose(glm::inverse(modelView));

		// Sonne
		glm::vec4 transformedSunDirection = glm::transpose(glm::inverse(camera.getView())) * glm::vec4(sunDirection, 1.0f);
		glUniform3fv(directionLocation, 1, (float*)& transformedSunDirection.data);

		// Fake Rotation (mit scale):
		//model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(sinf(time), 1, 1));


		// ModelView Matrizen
		GLCALL(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]));
		GLCALL(glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, &modelView[0][0]));
		GLCALL(glUniformMatrix4fv(invModelViewLocation, 1, GL_FALSE, &invModelView[0][0]));


		tree01.render();
		tree02.render();
		


		SDL_GL_SwapWindow(window); // 2 Buffer (Monitorausgabe|Bildberechnung) -> Doppelpufferung

		// Zeit messen:
		uint64_t endCounter = SDL_GetPerformanceCounter();
		uint64_t counterElapsed = endCounter - lastCounter;
		delta = ((float)counterElapsed) / (float)perfCounterFrequency;
		uint32_t FPS = (uint32_t)((float)perfCounterFrequency / (float)counterElapsed);
		//std::cout << FPS << std::endl;
		lastCounter = endCounter;

	}
	return 0;
}