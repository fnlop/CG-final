/*
CG Final - Death Effect
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include <random>
#include "../GL/glew.h"
#include "../GL/glut.h"
#include "../shader_lib/shader.h"
#include "glm/glm.h"
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/type_ptr.hpp"

extern "C"
{
	#include "glm_helper.h"
}

#define PROGRAM_NUM (1)
#define OBJ_NUM (3)
#define deltaTime (10)		// in ms (1e-3 second)

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

//no need to modify the following function declarations and gloabal variables
void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
void draw_light_bulb(void);
void camera_light_ball_move();
GLuint loadTexture(char* name, GLfloat width, GLfloat height);

namespace
{
	char *obj_file_dir[OBJ_NUM] = { "../Resources/bunny_s.obj",  "../Resources/Ball.obj" , "../Resources/teapot.obj" };
	char *main_tex_dir = "../Resources/Stone.ppm";
	char *noise_tex_dir = "../Resources/Noise.ppm";
	char *ramp_tex_dir = "../Resources/Ramp.ppm";
	
	GLfloat light_rad = 0.05;//radius of the light bulb
	float eyet = 0.0;//theta in degree
	float eyep = 90.0;//phi in degree
	bool mleft = false;
	bool mright = false;
	bool mmiddle = false;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool lforward = false;
	bool lbackward = false;
	bool lleft = false;
	bool lright = false;
	bool lup = false;
	bool ldown = false;
	bool bforward = false;
	bool bbackward = false;
	bool bleft = false;
	bool bright = false;
	bool bup = false;
	bool bdown = false;
	bool bx = false;
	bool by = false;
	bool bz = false;
	bool brx = false;
	bool bry = false;
	bool brz = false;
	int mousex = 0;
	int mousey = 0;
}

const float speed = 0.03; // camera / light / ball moving speed
const float rotation_speed = 0.05; // ball rotating speed

// No need for model texture, 'cause glmModel() has already loaded it for you.
// To use the texture, check glmModel documentation.
GLuint mainTextureID; // TA has already loaded this texture for you
GLuint vbo_id[OBJ_NUM];
GLuint vbo_line_id[OBJ_NUM];
GLuint vaoHandle[OBJ_NUM];
GLuint vaoHandle_line[OBJ_NUM];
int modelIdx = 0;
GLuint program[PROGRAM_NUM + 2];
int mode = 0;
char *vertfile[PROGRAM_NUM + 2] = {"shaders/explode.vert", "shaders/mesh.vert"};
char *fragfile[PROGRAM_NUM + 2] = {"shaders/explode.frag", "shaders/mesh.frag"};
GLfloat Ka[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Kd[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Ks[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat shine = 100;

GLMmodel *models[OBJ_NUM]; //TA has already loaded the model for you(!but you still need to convert it to VBO(s)!)


float eyex = 0.0;
float eyey = 0.0;
float eyez = 5.6;

GLfloat light_pos[] = { 1.1, 1.0, 1.3 }; //light positions
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };

const float showMeshTime = 1.0;
const float expandTime = 2.0;
const float startFadePercent = 0.5;
const float fadeTime = 2.0;
bool pause = true;
float showMeshValue = 0.0;
float expandValue = 0.0;
float fadeValue = 0.0;
//float showPercent = 0.01;				// discard some traingles of OBJ with complicate mesh for better visual effect
//float meshEnlargeSize = 10;				// enlarge size of mesh after discarding
// discard some traingles of OBJ with complicate mesh for better visual effect, differ with different models
float showPercent[OBJ_NUM] = { 0.01, 0.5, 0.1 };
// enlarge size of mesh after discarding, differ with different models and showPercent
float meshEnlargeSize[OBJ_NUM] = { 10, 1, 3 };

// random for better visual effect
std::default_random_engine gen = std::default_random_engine((std::random_device())());
std::uniform_real_distribution<float> dis(0, 1);
float randomSeed;

void Tick(int id) {
	double d = deltaTime / 1000.0;
	if (!pause) {
		if (showMeshValue < 1) {
			showMeshValue = std::fmin(showMeshValue + d / showMeshTime, 1);
		}
		else {
			if (expandValue < 1) {
				expandValue = std::fmin(expandValue + d / expandTime, 1);
			}
			if (expandValue >= startFadePercent && fadeValue < 1) {
				fadeValue = std::fmin(fadeValue + d / fadeTime, 1);
			}
		}
		//printf("%.2f %.2f %.2f\n", showMeshValue, expandValue, fadeValue);
	}
	glutPostRedisplay();
	glutTimerFunc(deltaTime, Tick, 0);				
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("CG_FINAL_Team4");
	glutReshapeWindow(512, 512);

	glewInit();

	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutTimerFunc(deltaTime, Tick, 0);			//pass Timer function

	glutMainLoop();

	for (GLMmodel *m : models) {
		glmDelete(m);
	}

	return 0;
}

void loadModel(char *path, int idx) {
	// load model obj file
	GLMmodel *model = glmReadOBJ(path);			//object file
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	models[idx] = model;
	//print_model_info(model);

//GLMtriangle: Structure that defines a triangle in a model.
//	/	typedef struct _GLMtriangle {
//		GLuint vindices[3];           /* array of triangle vertex indices */
//		GLuint nindices[3];           /* array of triangle normal indices */
//		GLuint tindices[3];           /* array of triangle texcoord indices*/
//		GLuint findex;                /* index of triangle facet normal */

	// ---- generate VBO ----
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id[idx]); //bind with the buffer , GL_ARRAY_BUFFER is for vertex type
	int index;
	Vertex *vertices = new Vertex[3 * model->numtriangles];			//total vertices
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
	}
	// use vertices to construct VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3 * model->numtriangles, vertices, GL_STATIC_DRAW);
	
	// ---- generate VAO ----
	glBindVertexArray(vaoHandle[idx]);
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

	//line vbo
	//create vbo , need vertex information
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line_id[idx]); //bind with the buffer , GL_ARRAY_BUFFER is for vertex type
	Point *mesh = new Point[6 * model->numtriangles];
	for (unsigned int i = 0; i < model->numtriangles; i+=1) {
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
		/*
		index = model->triangles[i].nindices[j];
		for (k = 0; k < 3; k++) {
			vertices[3 * i + j].normal[k] = model->normals[3 * index + k];
		}*/

		int scale = 0 ;
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * 6 * model->numtriangles, mesh, GL_DYNAMIC_DRAW);
	glBindVertexArray(vaoHandle_line[idx]);
	glEnableVertexAttribArray(0);
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);

	// release model
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
}

// init death effect while changing effect or model, or restart
void initDeath(void) {
	showMeshValue = expandValue = fadeValue = 0;
	randomSeed = dis(gen);
	pause = true;
}
void init(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	mainTextureID = loadTexture(main_tex_dir, 512, 256);

	//you may need to do something here(create shaders/program(s) and create vbo(s)/vao from GLMmodel model)
	for (int i = 0; i <= PROGRAM_NUM; i++) {
		GLuint vert = createShader(vertfile[i], "vertex");
		GLuint frag = createShader(fragfile[i], "fragment");
		program[i] = createProgram(vert, frag);
	}
	// create vbos and vaos
	glGenBuffers(OBJ_NUM, vbo_id);					// generate vbo buffers and assign their pointer to vboid
	glGenVertexArrays(OBJ_NUM, vaoHandle);			// generate vao arrays and assign their pointer to vaoid
	// create vbos and vaos of line mesh
	glGenBuffers(OBJ_NUM, vbo_line_id);
	glGenVertexArrays(OBJ_NUM, vaoHandle_line);
	// load model into vbos and vaos
	for (int i = 0; i < OBJ_NUM; i++) {
		loadModel(obj_file_dir[i], i);
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//please try not to modify the following block of code(you can but you are not supposed to)
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();				//initialization to Identity matrix
	gluLookAt(
		eyex,  //our head
		eyey, 
		eyez,
		eyex+cos(eyet*M_PI/180)*cos(eyep*M_PI / 180),  //look at some position
		eyey+sin(eyet*M_PI / 180), 
		eyez-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
		0.0,
		1.0,
		0.0);
	draw_light_bulb();
	
	//get light information
	//light will not change in push/pop
	float tmpmodelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, tmpmodelview);
	glm::vec3 lightnow = glm::make_mat4(tmpmodelview) * glm::vec4(glm::make_vec3(light_pos),  1.0);

	glPushMatrix();
		glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
		glRotatef(ball_rot[0], 1, 0, 0);
		glRotatef(ball_rot[1], 0, 1, 0);
		glRotatef(ball_rot[2], 0, 0, 1);
	// please try not to modify the previous block of code
		
	// you may need to do something here(pass uniform variable(s) to shader and render the model)
		glBindVertexArray(vaoHandle[modelIdx]);
		glUseProgram(program[mode]);
			float modelview[16];
			float proj[16];
			GLint loc;

			// main texture
			loc = glGetUniformLocation(program[mode], "tex");
			glActiveTexture(GL_TEXTURE0 + 0); //GL_TEXTUREi = GL_TEXTURE0 + i
			glBindTexture(GL_TEXTURE_2D, mainTextureID);
			glUniform1i(loc, 0);

			// model view matrix
			glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
			loc = glGetUniformLocation(program[mode], "modelview");	//get the location of uniform variable in shader
			glUniformMatrix4fv(loc, 1, GL_FALSE, modelview);		//assign value to it 
			// projection matrix
			glGetFloatv(GL_PROJECTION_MATRIX, proj);
			loc = glGetUniformLocation(program[mode], "proj");
			glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
			// all modes have basic phong shading
			//phong shading needs position in local space
			// normal matrix
			glm::mat4 Nmatrix = glm::transpose(glm::inverse(glm::make_mat4(modelview)));
			loc = glGetUniformLocation(program[mode], "Nmatrix");
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Nmatrix));

			loc = glGetUniformLocation(program[mode], "Ka");
			glUniform4fv(loc, 1, Ka);
			loc = glGetUniformLocation(program[mode], "Kd");
			glUniform4fv(loc, 1, Kd);
			loc = glGetUniformLocation(program[mode], "lightnow");
			glUniform3fv(loc, 1, glm::value_ptr(lightnow));
			loc = glGetUniformLocation(program[mode], "Ks");
			glUniform4fv(loc, 1, Ks);
			loc = glGetUniformLocation(program[mode], "alpha");
			glUniform1f(loc, shine);
			if (mode == 0) {					//explode effect
				glDisable(GL_CULL_FACE);
				loc = glGetUniformLocation(program[mode], "expandValue");
				glUniform1f(loc, expandValue);
				loc = glGetUniformLocation(program[mode], "showPercent");
				glUniform1f(loc, showPercent[modelIdx]);
				loc = glGetUniformLocation(program[mode], "meshEnlargeSize");
				glUniform1f(loc, meshEnlargeSize[modelIdx]);
				loc = glGetUniformLocation(program[mode], "fadeValue");
				glUniform1f(loc, fadeValue);
				loc = glGetUniformLocation(program[mode], "seed");
				glUniform1f(loc, randomSeed);
			}

			glDrawArrays(GL_TRIANGLES, 0, 3 * models[modelIdx]->numtriangles);
		glUseProgram(0);
		
		glEnable(GL_CULL_FACE);	
		glBindTexture(GL_TEXTURE_2D, NULL);

		//draw mesh


		if (showMeshValue > 0 && showMeshValue < 1) {
			glBindVertexArray(vaoHandle_line[modelIdx]);
			glUseProgram(program[PROGRAM_NUM]);			
				glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
				glGetFloatv(GL_PROJECTION_MATRIX, proj);
				loc = glGetUniformLocation(program[PROGRAM_NUM], "modelview"); //get the location of uniform variable in shader
				glUniformMatrix4fv(loc, 1, GL_FALSE, modelview); //assign value to it 
				loc = glGetUniformLocation(program[PROGRAM_NUM], "proj");
				glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
				loc = glGetUniformLocation(program[PROGRAM_NUM], "showMeshValue");
				glUniform1f(loc, showMeshValue);
				glLineWidth(2.f); 
				glDrawArrays(GL_LINES, 0, 6 * models[modelIdx]->numtriangles);
			glUseProgram(0);
		}
		glBindVertexArray(0);
		
	glPopMatrix();


	glutSwapBuffers();
	camera_light_ball_move();
}

//please implement mode toggle(switch mode between phongShading/Dissolving/Ramp) in case 'b'(lowercase)
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
	{	//ESC
		break;
	}
	case 'b':			//toggle mode
	{	
		mode = (mode + 1) % PROGRAM_NUM;
		initDeath();
		break;
	}
	case 'm':			//change model
	{
		modelIdx = (modelIdx + 1) % OBJ_NUM;
		initDeath();
		break;
	}
	case ' ':			// pause
	{
		pause ^= true;
		break;
	}
	case 'd':
	{
		right = true;
		break;
	}
	case 'a':
	{
		left = true;
		break;
	}
	case 'w':
	{
		forward = true;
		break;
	}
	case 's':
	{
		backward = true;
		break;
	}
	case 'q':
	{
		up = true;
		break;
	}
	case 'e':
	{
		down = true;
		break;
	}
	case 't':
	{
		lforward = true;
		break;
	}
	case 'g':
	{
		lbackward = true;
		break;
	}
	case 'h':
	{
		lright = true;
		break;
	}
	case 'f':
	{
		lleft = true;
		break;
	}
	case 'r':
	{
		lup = true;
		break;
	}
	case 'y':
	{
		ldown = true;
		break;
	}
	case 'i':
	{
		bforward = true;
		break;
	}
	case 'k':
	{
		bbackward = true;
		break;
	}
	case 'l':
	{
		bright = true;
		break;
	}
	case 'j':
	{
		bleft = true;
		break;
	}
	case 'u':
	{
		bup = true;
		break;
	}
	case 'o':
	{
		bdown = true;
		break;
	}
	case '7':
	{
		bx = true;
		break;
	}
	case '8':
	{
		by = true;
		break;
	}
	case '9':
	{
		bz = true;
		break;
	}
	case '4':
	{
		brx = true;
		break;
	}
	case '5':
	{
		bry = true;
		break;
	}
	case '6':
	{
		brz = true;
		break;
	}

	//special function key
	case 'z'://move light source to front of camera
	{
		light_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180);
		light_pos[1] = eyey + sin(eyet*M_PI / 180);
		light_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180);


	}
	case 'x'://move ball to front of camera
	{
		ball_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) * 3;
		ball_pos[1] = eyey + sin(eyet*M_PI / 180) * 5;
		ball_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180) * 3;
		break;
	}
	case 'c'://reset all pose
	{
		light_pos[0] = 1.1;
		light_pos[1] = 1.0;
		light_pos[2] = 1.3;
		ball_pos[0] = 0;
		ball_pos[1] = 0;
		ball_pos[2] = 0;
		ball_rot[0] = 0;
		ball_rot[1] = 0;
		ball_rot[2] = 0;
		eyex = 0.0;
		eyey = 0.0;
		eyez = 5.6;
		eyet = 0;
		eyep = 90;
		break;
	}
	default:
	{
		break;
	}
	}
}

//no need to modify the following functions
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	if (mright)
	{
		eyep -= (x-mousex)*0.1;
		eyet -= (y - mousey)*0.12;
		if (eyet > 89.9)
			eyet = 89.9;
		else if (eyet < -89.9)
			eyet = -89.9;
		if (eyep > 360)
			eyep -= 360;
		else if (eyep < 0)
			eyep += 360;
	}
	mousex = x;
	mousey = y;
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN && !mright && !mmiddle)
		{
			mleft = true;
			mousex = x;
			mousey = y;
			pause ^= true;
			if (pause) {
				initDeath();
			}
		}
		else
			mleft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mmiddle)
		{
			mright = true;
			mousex = x;
			mousey = y;
		}
		else
			mright = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mright)
		{
			mmiddle = true;
			mousex = x;
			mousey = y;
		}
		else
			mmiddle = false;
	}
}

void camera_light_ball_move()
{
	GLfloat dx = 0, dy = 0, dz=0;
	if(left|| right || forward || backward || up || down)
	{ 
		if (left)
			dx = -speed;
		else if (right)
			dx = speed;
		if (forward)
			dy = speed;
		else if (backward)
			dy = -speed;
		eyex += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		eyey += dy*sin(eyet*M_PI / 180);
		eyez += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (up)
			eyey += speed;
		else if (down)
			eyey -= speed;
	}
	if(lleft || lright || lforward || lbackward || lup || ldown)
	{
		dx = 0;
		dy = 0;
		if (lleft)
			dx = -speed;
		else if (lright)
			dx = speed;
		if (lforward)
			dy = speed;
		else if (lbackward)
			dy = -speed;
		light_pos[0] += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		light_pos[1] += dy*sin(eyet*M_PI / 180);
		light_pos[2] += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (lup)
			light_pos[1] += speed;
		else if(ldown)
			light_pos[1] -= speed;
	}
	if (bleft || bright || bforward || bbackward || bup || bdown)
	{
		dx = 0;
		dy = 0;
		if (bleft)
			dx = -speed;
		else if (bright)
			dx = speed;
		if (bforward)
			dy = speed;
		else if (bbackward)
			dy = -speed;
		ball_pos[0] += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		ball_pos[1] += dy*sin(eyet*M_PI / 180);
		ball_pos[2] += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (bup)
			ball_pos[1] += speed;
		else if (bdown)
			ball_pos[1] -= speed;
	}
	if(bx||by||bz || brx || bry || brz)
	{
		dx = 0;
		dy = 0;
		dz = 0;
		if (bx)
			dx = -rotation_speed;
		else if (brx)
			dx = rotation_speed;
		if (by)
			dy = rotation_speed;
		else if (bry)
			dy = -rotation_speed;
		if (bz)
			dz = rotation_speed;
		else if (brz)
			dz = -rotation_speed;
		ball_rot[0] += dx;
		ball_rot[1] += dy;
		ball_rot[2] += dz;
	}
}

void draw_light_bulb()
{
	GLUquadric *quad;
	quad = gluNewQuadric();
	glPushMatrix();
	glColor3f(0.4, 0.5, 0);
	glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
	gluSphere(quad, light_rad, 40, 20);
	glPopMatrix();
}

void keyboardup(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
	{
		right =false;
		break;
	}
	case 'a':
	{
		left = false;
		break;
	}
	case 'w':
	{
		forward = false;
		break;
	}
	case 's':
	{
		backward = false;
		break;
	}
	case 'q':
	{
		up = false;
		break;
	}
	case 'e':
	{
		down = false;
		break;
	}
	case 't':
	{
		lforward = false;
		break;
	}
	case 'g':
	{
		lbackward = false;
		break;
	}
	case 'h':
	{
		lright = false;
		break;
	}
	case 'f':
	{
		lleft = false;
		break;
	}
	case 'r':
	{
		lup = false;
		break;
	}
	case 'y':
	{
		ldown = false;
		break;
	}
	case 'i':
	{
		bforward = false;
		break;
	}
	case 'k':
	{
		bbackward = false;
		break;
	}
	case 'l':
	{
		bright = false;
		break;
	}
	case 'j':
	{
		bleft = false;
		break;
	}
	case 'u':
	{
		bup = false;
		break;
	}
	case 'o':
	{
		bdown = false;
		break;
	}
	case '7':
	{
		bx = false;
		break;
	}
	case '8':
	{
		by = false;
		break;
	}
	case '9':
	{
		bz = false;
		break;
	}
	case '4':
	{
		brx = false;
		break;
	}
	case '5':
	{
		bry = false;
		break;
	}
	case '6':
	{
		brz = false;
		break;
	}

	default:
	{
		break;
	}
	}
}

void idle(void)
{
	glutPostRedisplay();
}

GLuint loadTexture(char* name, GLfloat width, GLfloat height)
{
	return glmLoadTexture(name, false, true, true, true, &width, &height);
}
