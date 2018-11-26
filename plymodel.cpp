#include <fstream>
#include "plymodel.h"


PLYModel::PLYModel() = default;
PLYModel::~PLYModel() = default;

PLYModel::PLYModel(const char *file_path) {
	model_name = std::string(file_path);
	ReadFile(file_path);

	CalculateFaceNormals();
	CalculateFaceNormals();

	BufferSetup();

	xDis = xMax - xMin;
	yDis = yMax - yMin;
	zDis = zMax - zMin;
}

void PLYModel::ReadFile(const char *file_path) {
	xMin = std::numeric_limits<float>::max();
	yMin = std::numeric_limits<float>::max();
	zMin = std::numeric_limits<float>::max();

	xMax = std::numeric_limits<float>::min();
	yMax = std::numeric_limits<float>::min();
	zMax = std::numeric_limits<float>::min();

	std::cout << "PLYModel::ReadFile::" << file_path << std::endl;
	std::ifstream input_file;
	input_file.open(file_path);

	std::string content;
	int num_vertices;
	int num_faces;
	while (content != "vertex") {
		input_file >> content;
	}
	input_file >> num_vertices;
	while (content != "face") {
		input_file >> content;
	}
	input_file >> num_faces;
	while (content != "end_header") {
		input_file >> content;
	}

	// read vertices
	for (size_t i = 0; i < num_vertices; i++) {
		float x, y, z;
		input_file >> x;
		input_file >> y;
		input_file >> z;

		xMin = std::fmin(xMin, x);
		yMin = std::fmin(yMin, y);
		zMin = std::fmin(zMin, z);
		xMax = std::fmax(xMax, x);
		yMax = std::fmax(yMax, y);
		zMax = std::fmax(zMax, z);

		points.push_back(vec3(x, y, z));
	}

	for (size_t i = 0; i < num_faces; i++) {
		int n, v1, v2, v3;
		input_file >> n;
		input_file >> v1;
		input_file >> v2;
		input_file >> v3;
		face_index.push_back(vec3(v1, v2, v3));
	}

	input_file.close();

	std::cout << "PLYModel::" << file_path << "\n"
		<< "number vertices: " << num_vertices << " " << points.size() << "\n"
		<< "number faces: " << num_faces << " " << face_index.size() << std::endl;

	std::cout << "PLYModel::Boundary::" << "\n"
		<< "xMin -- xMax:  " << xMin << "  --  " << xMax << "\n"
		<< "yMin -- yMax:  " << yMin << "  --  " << yMax << "\n"
		<< "zMin -- zMax:  " << zMin << "  --  " << zMax << std::endl;

}

void PLYModel::CalculateFaceNormals() {
	//int counter = 0;
	for (size_t i = 0; i < face_index.size(); i++) {
		vec3 face_normal = vec3(0,0,0);
		vec3 curr_face = face_index[i];
		for (int i = 0; i < 3; i++) {
			vec3 curr_vertex = points[(int)curr_face[i]];
			vec3 next_vertex = points[(int)curr_face[(i + 1) % 3]];

			face_normal.x = face_normal.x + (curr_vertex.y - next_vertex.y) * (curr_vertex.z + next_vertex.z);
			face_normal.y = face_normal.y + (curr_vertex.z - next_vertex.z) * (curr_vertex.x + next_vertex.x);
			face_normal.z = face_normal.z + (curr_vertex.x - next_vertex.x) * (curr_vertex.y + next_vertex.y);
		}
		face_normal = Angel::normalize(face_normal);
		face_normals.push_back(Angel::normalize(face_normal));
		//counter++;
	}
	std::cout << "CalculateFaceNormals:: number face normals: " << face_normals.size() << std::endl;
}

void PLYModel::CalculateVertexNormals() {
	for (int i = 0; i < points.size(); i++) {
		vertex_normals.push_back(vec3(0, 0, 0));
	}
	for (int i = 0; i < face_index.size(); i++) {
		vec3 face_normal = face_normals[i];
		vec3 curr_face = face_index[i];

		vertex_normals[(int)curr_face.x] += face_normal;
		vertex_normals[(int)curr_face.y] += face_normal;
		vertex_normals[(int)curr_face.z] += face_normal;
	}
}

void PLYModel::BufferSetup() {

	auto *v = new vec3[points.size()];
	auto *n = new vec3[vertex_normals.size()];
	auto *e = new int[face_index.size() * 3]; // ebo index

	for (int i = 0; i < points.size(); i++) {
		v[i] = points[i];
	}

	for (int i = 0; i < vertex_normals.size(); i++) {
		n[i] = vertex_normals[i];
	}

	for (int i = 0; i < face_index.size(); i++) {
		vec3 curr_face = face_index[i];
		e[3 * i + 0] = (int)curr_face.x;
		e[3 * i + 1] = (int)curr_face.y;
		e[3 * i + 2] = (int)curr_face.z;
	}

	GLuint VBO[2];
	GLuint EBO;
	glGenVertexArrays(1, &vao_mesh);
	glBindVertexArray(vao_mesh);

	glGenBuffers(2, VBO);

	// for points
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*points.size(), v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	// for vertex normals
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*points.size(), n, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	// for EBO
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*face_index.size() * sizeof(int), e, GL_STATIC_DRAW);

	glBindVertexArray(0);

	delete[] v;
	delete[] n;
	delete[] e;

}


GLuint PLYModel::GetVAO() {
	return vao_mesh;
}

void PLYModel::Render() const {

	glBindVertexArray(vao_mesh);
	glDrawElements(GL_TRIANGLES, 3 * face_index.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
