#pragma once

#include <glm/glm.hpp>
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "shader.h"
#include <vector>
#include <fstream>

struct Material
{
	//std::vector<float> diffuse;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 emissive;
	float shininess;
};

class Mesh
{
public:
	Mesh(const char* filename, Material material, Shader* shader)
	{
		this->material = material;
		this->shader = shader;
		//Todo: auslagern in eine Initial -Methode, wegen Fehlerrueckgabewert von fstream
		std::vector<Vertex> vertices;
		uint64_t countVerticies = 0; // Anzahl Dreiecke in verticies Array

		// Index fuer komplexere Formen
		std::vector<uint32_t> indices;
		numIndices = 0;

		std::ifstream input = std::ifstream(filename, std::ios::in | std::ios::binary);
		//Todo:Throw
		input.read((char*)& countVerticies, sizeof(uint64_t));
		input.read((char*)& numIndices, sizeof(uint64_t));
		//Todo: Optimierung

		for (uint64_t i = 0; i < countVerticies; i++)
		{
			Vertex vertex;
			input.read((char*)& vertex.position.x, sizeof(float));
			input.read((char*)& vertex.position.y, sizeof(float));
			input.read((char*)& vertex.position.z, sizeof(float));

			input.read((char*)& vertex.normal.x, sizeof(float));
			input.read((char*)& vertex.normal.y, sizeof(float));
			input.read((char*)& vertex.normal.z, sizeof(float));
			vertices.push_back(vertex);
		}

		for (uint64_t i = 0; i < numIndices; i++)
		{
			uint32_t index;
			input.read((char*)& index, sizeof(uint32_t));
			indices.push_back(index);
		}

		vertexBuffer = new VertexBuffer(vertices.data(), countVerticies);
		indexBuffer = new IndexBuffer(indices.data(), numIndices, sizeof(indices[0]));

		diffuseLocation = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_diffuse"));
		specularLocation = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_specular"));
		emissiveLocation = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_emissive"));
		shininessLocatin = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_shininess"));
	}

	~Mesh()
	{
		delete vertexBuffer;
		delete indexBuffer;
	}

	inline void render()
	{
		vertexBuffer->Bind();
		indexBuffer->bind();
		glUniform3fv(diffuseLocation, 1, (float*)& material.diffuse.data);
		glUniform3fv(specularLocation, 1, (float*)& material.specular.data);
		glUniform3fv(emissiveLocation, 1, (float*)& material.emissive.data);
		glUniform1f(shininessLocatin, material.shininess);
		GLCALL(glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0));
	}

	

private:
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	Shader* shader;
	Material material;
	uint64_t numIndices = 0;
	int diffuseLocation;
	int specularLocation;
	int emissiveLocation;
	int shininessLocatin;
};
