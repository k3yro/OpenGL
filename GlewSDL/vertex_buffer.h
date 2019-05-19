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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(struct Vertex, x));
		glEnableVertexAttribArray(1/*Index*/);
		glVertexAttribPointer(1/*Index*/, 4/*rot bis alpha*/, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(struct Vertex, r)/*offset*/);

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