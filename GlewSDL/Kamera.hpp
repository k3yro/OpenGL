#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Kamera
{
public:
	Kamera(float fov, float width, float height) : fov(fov), width(width), height(height)
	{

		projection = glm::perspective(fov / 2.0f/*Halber Winkel*/, width / height, 0.1f, 1000.0f);
		view = glm::mat4(1.0f); // Einheitsmatrix
		ortho = false;
	}

	//Todo: Implement
	void switchProjection()
	{
		ortho = !ortho;
		if (ortho)
		{
			projection = glm::ortho(-2.26f, 2.26f, -1.7f, 1.7f, 0.1f, 1000.0f);
		}
		else
		{
			projection = glm::perspective(fov / 2.0f/*Halber Winkel*/, width / height, 0.1f, 1000.0f);
		}
	}

	// Vorberechnete Kameraposition zurueckgeben (Optimierung)
	glm::mat4 getViewProj()
	{
		return viewProj; 
	}

	// Kameraposition neu berechnen
	void update()
	{
		viewProj = projection * view;
	}

	// Alle Kamerabewegungen muessen invertiert werden
	void translate(glm::vec3 v /*Richtung, in die sich Kamera bewegt*/)
	{
		// view invertieren (view = in welche Richtung muessen sich andere
		// Objekte bewegen, damit sich Kamera im 0-Punkt befindet)
		view = glm::translate(view, v * -1.0f);
		// Bei Rotation muss auch invertiert werden
	}

	
	~Kamera() 
	{

	}

private:
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 viewProj;

	float fov, width, height;
	bool ortho;
};