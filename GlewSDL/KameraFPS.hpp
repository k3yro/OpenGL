#pragma once

#include "Kamera.hpp"

class KameraFPS: public Kamera
{
public:
	KameraFPS(float fov, float width, float height) : Kamera(fov, width, height)
	{
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw = -90.0f;
		pitch = 0.0f;
		onMouseMoved(0.0f, 0.0f);
		update();
	}

	void onMouseMoved(float xRel, float yRel)
	{
		yaw += xRel * mouseSensitivity;
		pitch -= yRel * mouseSensitivity;
		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}
		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		lookAt = glm::normalize(front);
		update();
	}

	void update() override 
	{
		view = glm::lookAt(position, position + lookAt, up);
		viewProj = projection * view;
	}

	void moveFront(float amount)
	{
		translate(glm::normalize(glm::vec3(1.0f, 0.0f/*Bewegung in y-Richtung sperren*/, 1.0f) * lookAt) * amount);
		update();
	}

	void moveSideway(float amount)
	{
		// cross = Kreuzprodukt von lookAt und up
		translate(glm::normalize(glm::cross(lookAt, up))* amount);
		update();
	}

protected:
	
	float yaw;		// Rotation um die y-Achse (nach links und rechts schauen)
	float pitch;	// Rotation um die x-Achse (nach oben und unten schauen)
	// FYI: roll = Rotation um z-Achse ...

	glm::vec3 lookAt;
	const float mouseSensitivity = 0.1f;
	glm::vec3 up;
};

