#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Position {
	float x, y, z;
};

struct Material {
	Position diffuse;
	Position specular;
	Position emissive;
	float shininess;
};

struct Mesh {
	std::vector<Position> positions;
	std::vector<Position> normals;
	std::vector<uint32_t> indices;
	Material material;
};

std::vector<Mesh> meshes;
std::vector<Material> materials;

void processMesh(aiMesh* mesh, const aiScene* scene) {
	Mesh m;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Position position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		m.positions.push_back(position);

		Position normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		m.normals.push_back(normal);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			m.indices.push_back(face.mIndices[j]);
		}
	}

	m.material = materials[mesh->mMaterialIndex];
	meshes.push_back(m);
}

void processNode(aiNode* node, const aiScene* scene) {

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

char* getFilename(char* filename) {
	int len = strlen(filename);
	char* lastSlash = filename;
	for (int i = 0; i < len; i++) {
		if (filename[i] == '/' || filename[i] == '\\') {
			lastSlash = filename + i + 1;
		}
	}
	return lastSlash;
}

void processMaterials(const aiScene* scene) {
	for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
		Material mat;
		aiMaterial* material = scene->mMaterials[i];

		aiColor3D diffuse(0.0f, 0.0f, 0.0f);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse)) {
			// No diffuse color
		}
		mat.diffuse = { diffuse.r, diffuse.g, diffuse.b };

		aiColor3D specular(0.0f, 0.0f, 0.0f);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_SPECULAR, specular)) {
			// No specular color
		}
		mat.specular = { specular.r, specular.g, specular.b };

		aiColor3D emissive(0.0f, 0.0f, 0.0f);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive)) {
			// No emissive color
		}
		mat.emissive = { emissive.r, emissive.g, emissive.b };

		float shininess = 0.0f;
		if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS, shininess)) {
			// No shininess
		}
		mat.shininess = shininess;

		float shininessStrength = 1.0f;
		if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength)) {
			// No shininessStrength
		}
		mat.specular.x *= shininessStrength;
		mat.specular.y *= shininessStrength;
		mat.specular.z *= shininessStrength;

		materials.push_back(mat);
	}
}

bool ideMode;
std::string inputFilename = "";
std::string pathToModelFolder = "";

int main(int argc, char** argv) {

	//############ CONFIGURE ##################
	ideMode = true;
	if (ideMode)
	{
		inputFilename = "world.obj";
		pathToModelFolder = "..//GlewSDL//Models//"; // End with "//"
	}
	//#########################################

	//Todo: Hotfolder for Models

	if (!ideMode)
	{
		if (argc <= 0) {
			return 1;
		}
		if (argc < 2) {
			std::cout << "Usage: " << argv[0] << " <modelfilename>" << std::endl;
		return 1;
		}
		inputFilename = argv[argc - 1];
	}

	unsigned int pFlags = 0;
	pFlags ^= aiProcess_Triangulate;
	pFlags ^= aiProcess_GenNormals;
	pFlags ^= aiProcess_OptimizeMeshes;
	pFlags ^= aiProcess_OptimizeGraph;
	pFlags ^= aiProcess_JoinIdenticalVertices;
	pFlags ^= aiProcess_ImproveCacheLocality;

	if (false)
	{
		pFlags ^= aiProcess_PreTransformVertices;
	}
	
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inputFilename, pFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE, !scene->mRootNode) {
		std::cout << "Error while loading model with assimp: " << importer.GetErrorString() << std::endl;
		return 1;
	}

	processMaterials(scene);
	processNode(scene->mRootNode, scene);

	char* cInputFileName = new char[inputFilename.length() + 1];
#pragma warning(disable:4996)
	strcpy(cInputFileName, inputFilename.c_str());
#pragma warning(enable:4996)
	std::string filename = std::string(getFilename(cInputFileName));

	delete[] cInputFileName;

	std::string filenameWithoutExtension = filename.substr(0, filename.find_last_of('.'));
	std::string outputFilename = pathToModelFolder + filenameWithoutExtension + ".bmf";

	std::ofstream output(outputFilename, std::ios::out | std::ios::binary);
	std::cout << "Writing bmf file..." << std::endl;
	uint64_t numMeshes = meshes.size();
	output.write((char*)& numMeshes, sizeof(uint64_t));
	for (Mesh& mesh : meshes) {
		uint64_t numVertices = mesh.positions.size();
		uint64_t numIndices = mesh.indices.size();
		output.write((char*)& mesh.material, sizeof(Material));

		output.write((char*)& numVertices, sizeof(uint64_t));
		output.write((char*)& numIndices, sizeof(uint64_t));
		for (uint64_t i = 0; i < numVertices; i++) {
			output.write((char*)& mesh.positions[i].x, sizeof(float));
			output.write((char*)& mesh.positions[i].y, sizeof(float));
			output.write((char*)& mesh.positions[i].z, sizeof(float));

			output.write((char*)& mesh.normals[i].x, sizeof(float));
			output.write((char*)& mesh.normals[i].y, sizeof(float));
			output.write((char*)& mesh.normals[i].z, sizeof(float));
		}
		for (uint64_t i = 0; i < numIndices; i++) {
			output.write((char*)& mesh.indices[i], sizeof(uint32_t));
		}
	}

	output.close();
	if (ideMode)
	{
		system("pause");
	}
}