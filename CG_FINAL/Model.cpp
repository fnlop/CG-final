#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include "../GL/glew.h"
#include "../GL/glut.h"
#include "glm/glm/glm.hpp"
#include "Model.h"

Model::Model() {
	model = NULL;
	vertices = NULL;
	radius = 1.0;
}
Model::Model(char *path) : Model() {
	loadModel(path);
}
Model::Model(Model&& other) {
	model = other.model;
	vertices = other.vertices;
	ymin = other.ymin;
	ymax = other.ymax;
	radius = other.radius;
	other.model = NULL;
	other.vertices = NULL;
}
Model& Model::operator=(Model&& other) {
	model = other.model;
	vertices = other.vertices;
	ymin = other.ymin;
	ymax = other.ymax;
	radius = other.radius;
	other.model = NULL;
	other.vertices = NULL;
	return *this;
}

Model::~Model() {
	if (model != NULL) {
		glmDelete(model);
	}
	if (vertices != NULL) {
		delete[] vertices;
	}
}

void Model::loadModel(char * path) {
	// load model obj file
	model = glmReadOBJ(path);			//object file
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	//print_model_info(model);

//GLMtriangle: Structure that defines a triangle in a model.
//	/	typedef struct _GLMtriangle {
//		GLuint vindices[3];           /* array of triangle vertex indices */
//		GLuint nindices[3];           /* array of triangle normal indices */
//		GLuint tindices[3];           /* array of triangle texcoord indices*/
//		GLuint findex;                /* index of triangle facet normal */

	ymin = INFINITY;
	ymax = -INFINITY;
	// create vertices for VBO and VAO
	int index;
	vertices = new Vertex[3 * model->numtriangles];			//total vertices
	for (unsigned int i = 0; i < model->numtriangles; i++) {		//all triangles
		GLfloat tpos[3] = { 0, 0, 0 };
		for (int j = 0; j < 3; j++) {						//vertices in a triangle
			//position
			index = model->triangles[i].vindices[j];		//specific vertex
			int k;
			for (k = 0; k < 3; k++) {
				vertices[3 * i + j].position[k] = model->vertices[3 * index + k];
				tpos[k] += model->vertices[3 * index + k];
			}
			//normal
			index = model->triangles[i].nindices[j];
			for (k = 0; k < 3; k++) {
				vertices[3 * i + j].normal[k] = model->normals[3 * index + k];
			}
			//texture
			index = model->triangles[i].tindices[j];
			for (k = 0; k < 2; k++) {
				vertices[3 * i + j].texcoord[k] = model->texcoords[2 * index + k];
			}
			
		}
		// set triangle position to all the 3 vertices
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				vertices[3 * i + j].trianglePosition[k] = tpos[k] / 3;
			}
		}
		if (vertices[3 * i + 0].trianglePosition[1] > ymax)
			ymax = vertices[3 * i + 0].trianglePosition[1];
		if (vertices[3 * i + 0].trianglePosition[1] < ymin)
			ymin = vertices[3 * i + 0].trianglePosition[1];
	}
}

void Model::constructVO(GLuint vboid, GLuint vaoid) {
	// ---- generate VBO ----
	glBindBuffer(GL_ARRAY_BUFFER, vboid); //bind with the buffer , GL_ARRAY_BUFFER is for vertex type
	// use vertices to construct VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3 * model->numtriangles, vertices, GL_STATIC_DRAW);

	// ---- generate VAO ----
	glBindVertexArray(vaoid);
	glEnableVertexAttribArray(0);	// VAO[0] for position
	glEnableVertexAttribArray(1);	// VAO[1] for normal
	glEnableVertexAttribArray(2);	// VAO[2] for textures
	glEnableVertexAttribArray(3);	// VAO[3] for traingle position
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	//normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	//texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texcoord)));
	// triangle position
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, trianglePosition)));

	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int Model::getVertexNum(void) {
	return 3 * model->numtriangles;
}

GLMmodel* Model::getGLMmodel(void) {
	return model;
}

float Model::getBoundingRadius(void) {
	return radius;
}

std::pair<float, float> Model::getBoundingY() {
	return std::pair<float, float>(ymin, ymax);
}
