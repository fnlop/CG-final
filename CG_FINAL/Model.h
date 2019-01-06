#pragma once
#include "glm/glm.h"

struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
	GLfloat trianglePosition[3];
};
typedef struct Vertex Vertex;

class Model
{
	public:
		Model();
		Model(char *);
		Model(Model&&);
		Model& operator=(Model&&);
		~Model();
		void loadModel(char *);
		void constructVO(GLuint, GLuint);
		int getVertexNum(void);
		GLMmodel* getGLMmodel(void);
		float getBoundingRadius(void);
	private:
		GLMmodel *model;
		Vertex *vertices;
		float radius;
};

