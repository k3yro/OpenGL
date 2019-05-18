#pragma once
#include <GL/glew.h>

#include "defines.h"

struct VertexBuffer
{
	VertexBuffer(void* data, uint32_t numVertices)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &bufferId);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), data, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0/*Vorsich: x Koord an erster Stelle!*/);

		glBindVertexArray(0);
	}

	virtual ~VertexBuffer()
	{
		glDeleteBuffers(1, &bufferId);
	}
	void Bind()
	{
		glBindVertexArray(vao);
	}

	void Unbind()
	{
		glBindVertexArray(0);
	}

private:
	GLuint bufferId;
	GLuint vao; //VertexArrayObject...
};