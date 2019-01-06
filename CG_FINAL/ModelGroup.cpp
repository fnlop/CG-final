#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include "../GL/glew.h"
#include "../GL/glut.h"
#include "glm/glm/glm.hpp"
#include "ModelGroup.h"

ModelGroup::ModelGroup() : radius(1.0){
	model = NULL;
	vertices = NULL;
	mesh = NULL;
	vboId = vaoId = vboLineId = vaoLineId = 0;
}
ModelGroup::ModelGroup(char *path) : ModelGroup() {
	loadModel(path);
}
ModelGroup::ModelGroup(ModelGroup&& other) {
	*this = std::move(other);
}
ModelGroup& ModelGroup::operator=(ModelGroup&& other) {
	*this = other;
	other.model = NULL;
	other.vertices = NULL;
	other.mesh = NULL;
	return *this;
}

ModelGroup::~ModelGroup() {
	if (model != NULL) {
		glmDelete(model);
	}
	if (vertices != NULL) {
		delete[] vertices;
	}
	if (mesh != NULL) {
		delete[] mesh;
	}
}

void ModelGroup::loadModel(char * path) {
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

	// create mesh vertices
	Point *mesh = new Point[6 * model->numtriangles];
	for (unsigned int i = 0; i < model->numtriangles; i += 1) {
		index = model->triangles[i].vindices[0];
		int j;

		GLfloat t[3] = { 0 };
		index = model->triangles[i].vindices[0];
		for (j = 0; j < 3; j++) {
			t[j] += model->vertices[3 * index + j];
		}
		index = model->triangles[i].vindices[1];
		for (j = 0; j < 3; j++) {
			t[j] += model->vertices[3 * index + j];
		}
		index = model->triangles[i].vindices[2];
		for (j = 0; j < 3; j++) {
			t[j] += model->vertices[3 * index + j];
		}
		for (j = 0; j < 3; j++)
			t[j] /= 3;

		int scale = 0;
		index = model->triangles[i].vindices[0];
		for (j = 0; j < 3; j++) {
			GLfloat tmp = t[j] - model->vertices[3 * index + j];
			mesh[6 * i + 0].position[j] = model->vertices[3 * index + j] + scale * tmp;
		}

		index = model->triangles[i].vindices[1];
		for (j = 0; j < 3; j++) {
			GLfloat tmp = t[j] - model->vertices[3 * index + j];
			mesh[6 * i + 1].position[j] = model->vertices[3 * index + j] + scale * tmp;
		}

		index = model->triangles[i].vindices[1];
		for (j = 0; j < 3; j++) {
			GLfloat tmp = t[j] - model->vertices[3 * index + j];
			mesh[6 * i + 2].position[j] = model->vertices[3 * index + j] + scale * tmp;
		}

		index = model->triangles[i].vindices[2];
		for (j = 0; j < 3; j++) {
			GLfloat tmp = t[j] - model->vertices[3 * index + j];
			mesh[6 * i + 3].position[j] = model->vertices[3 * index + j] + scale * tmp;
		}

		index = model->triangles[i].vindices[2];
		for (j = 0; j < 3; j++) {
			GLfloat tmp = t[j] - model->vertices[3 * index + j];
			mesh[6 * i + 4].position[j] = model->vertices[3 * index + j] + scale * tmp;
		}

		index = model->triangles[i].vindices[0];
		for (j = 0; j < 3; j++) {
			GLfloat tmp = t[j] - model->vertices[3 * index + j];
			mesh[6 * i + 5].position[j] = model->vertices[3 * index + j] + scale * tmp;
		}
	}
}

void ModelGroup::constructVO(GLuint vboid, GLuint vaoid) {
	vboId = vboid;
	vaoId = vaoid;

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
void ModelGroup::constructLineVO(GLuint vboid, GLuint vaoid) {
	vboLineId = vboid;
	vaoLineId = vaoid;
	// ---- generate VBO ----
	glBindBuffer(GL_ARRAY_BUFFER, vboid); //bind with the buffer , GL_ARRAY_BUFFER is for vertex type
	// use vertices to construct VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * 6 * model->numtriangles, mesh, GL_STATIC_DRAW);

	// ---- generate VAO ----
	glBindVertexArray(vaoid);
	glEnableVertexAttribArray(0);
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);

	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int ModelGroup::getVertexNum(void) {
	return 3 * model->numtriangles;
}

GLMmodel* ModelGroup::getGLMmodel(void) {
	return model;
}

float ModelGroup::getBoundingRadius(void) {
	return radius;
}

std::pair<float, float> ModelGroup::getBoundingY() {
	return std::pair<float, float>(ymin, ymax);
}
