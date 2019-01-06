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
struct Point
{
	GLfloat position[3];
};
typedef struct Point Point;

class ModelGroup
{
	public:
		ModelGroup();
		ModelGroup(char *);
		ModelGroup(ModelGroup&&);
		ModelGroup& operator=(ModelGroup&&);
		~ModelGroup();
		void loadModel(char *);
		void constructVO(GLuint, GLuint);
		void constructLineVO(GLuint, GLuint);
		int getVertexNum(void);
		GLMmodel* getGLMmodel(void);
		float getBoundingRadius(void);
		std::pair<float, float> getBoundingY();
	private:
		ModelGroup& operator=(const ModelGroup&) = default;
	private:
		GLMmodel *model;
		Vertex *vertices;
		Point *mesh;
		float ymin;
		float ymax;
		float radius;
		GLuint vboId;
		GLuint vaoId;
		GLuint vboLineId;
		GLuint vaoLineId;
};

