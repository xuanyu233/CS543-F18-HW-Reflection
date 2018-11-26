#pragma once

#include <vector>
#include <string>
#include "Angel.h"
#include "drawable.h"

class PLYModel : public Drawable {
public:
	PLYModel();
	~PLYModel();
	PLYModel(const char *file_path);
	virtual void Render() const override;
	GLuint GetVAO();

	float xMin, yMin, zMin;
	float xMax, yMax, zMax;

	float xDis, yDis, zDis;

private:
	void ReadFile(const char* file_path);
	void BufferSetup();

	void CalculateFaceNormals();
	void CalculateVertexNormals();
	std::string model_name;

	std::vector<vec3> points;
	std::vector<vec3> face_index;
	std::vector<vec3> face_normals;
	std::vector<vec3> vertex_normals;

	GLuint vao_mesh;
};
