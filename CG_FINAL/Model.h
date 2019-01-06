#pragma once
#include "glm/glm.h"
#include <utility>

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
		std::pair<float, float> getBoundingY();
	private:
		GLMmodel *model;
		Vertex *vertices;
		float ymin;
		float ymax;
		float radius;
};

