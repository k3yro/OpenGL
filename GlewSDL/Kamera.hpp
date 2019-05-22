#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class Kamera
{
public:
	Kamera(float fov, float width, float height)
	{
		projection = glm::perspective(fov / 2.0f/*Halber Winkel*/, width / height, 0.1f, 1000.0f);
		view = glm::mat4(1.0f); // Einheitsmatrix
		position = glm::vec3(0.0f);
		update();
	}

	// Vorberechnete Kameraposition zurueckgeben (Optimierung)
	glm::mat4 getViewProj()
	{
		return viewProj; 
	}

	glm::mat4 getView()
	{
		return view;
	}

	// Kameraposition neu berechnen
	virtual void update()
	{
		viewProj = projection * view;
	}

	// Alle Kamerabewegungen muessen invertiert werden
	virtual void translate(glm::vec3 v /*Richtung, in die sich Kamera bewegt*/)
	{
		position += v;
		// view invertieren (view = in welche Richtung muessen sich andere
		// Objekte bewegen, damit sich Kamera im 0-Punkt befindet)
		view = glm::translate(view, v * -1.0f);
		// Bei Rotation muss auch invertiert werden
	}


protected:
	glm::vec3 position; // Wo befindet sich Kamera gerade?
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 viewProj;
};