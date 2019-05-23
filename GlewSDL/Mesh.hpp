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
	Mesh(std::vector<Vertex>& vertices, uint64_t numVertices, std::vector<uint32_t>& indices, uint64_t numIndices, Material material, Shader* shader)
	{
		this->material = material;
		this->shader = shader;
		this->numIndices = numIndices;
		

		vertexBuffer = new VertexBuffer(vertices.data(), numVertices);
		indexBuffer = new IndexBuffer(indices.data(), numIndices, sizeof(indices[0]));

		diffuseLocation = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_material.diffuse"));
		specularLocation = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_material.specular"));
		emissiveLocation = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_material.emissive"));
		shininessLocatin = GLCALL(glGetUniformLocation(shader->getShaderId(), "u_material.shininess"));
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
		glUniform3fv(diffuseLocation, 1, (float*)& material.diffuse.x);
		glUniform3fv(specularLocation, 1, (float*)& material.specular.x);
		glUniform3fv(emissiveLocation, 1, (float*)& material.emissive.x);
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

class Model
{
public:
	Model() {}

	void init(const char* filename, Shader* shader) 
	{
		uint64_t numMeshes = 0;
		std::ifstream input = std::ifstream(filename, std::ios::in | std::ios::binary);
		if (!input.is_open())
		{
			std::cout << "Model nicht gefunden" << std::endl;
			return;
		}

		input.read((char*)& numMeshes, sizeof(uint64_t));

		for (size_t i = 0; i < numMeshes; i++)
		{
			Material material = {};
			std::vector<Vertex> vertices;
			uint64_t numVertices = 0;
			std::vector<uint32_t> indices;
			uint64_t numIndices = 0;

			input.read((char*)& material, sizeof(Material));
			input.read((char*)& numVertices, sizeof(uint64_t));
			input.read((char*)& numIndices, sizeof(uint64_t));
			//Todo: Optimierung

			for (uint64_t i = 0; i < numVertices; i++)
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

			meshes.push_back(new Mesh(vertices, numVertices, indices, numIndices, material, shader));
		}
	}

	void render() 
	{
		for (Mesh* mesh : meshes)
		{
			mesh->render();
		}
	}

	~Model() 
	{
		for (Mesh* mesh : meshes)
		{
			delete mesh;
		}
	}

private:
	std::vector<Mesh*> meshes;
};
