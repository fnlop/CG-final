/*

CG Homework2 - Phong Shading, Dissolving effects and Ramp effects

Objective - learning GLSL, glm model datatype(for .obj file) 

Overview:

	1. Render the model with texture using VBO and/or VAO

	2. Implement Phong shading

	3. Implement Dissolving effects via shader

	4. Implement Ramp effects via shader (Basically a dicrete shading)

Good luck!

!!!IMPORTANT!!! 

1. Make sure to change the window name to your student ID!
2. Make sure to remove glmDraw() at line 211.
   It's meant for a quick preview of the ball object and shouldn't exist in your final code.

*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
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

/*you may need to do something here
you may use the following struct type to perform your single VBO method,
or you can define/declare multiple VBOs for VAO method.
Please feel free to modify it*/
struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
};
typedef struct Vertex Vertex;

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
	char *obj_file_dir = "../Resources/bunny.obj";
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

// You can modify the moving/rotating speed if it's too fast/slow for you
const float speed = 0.03; // camera / light / ball moving speed
const float rotation_speed = 0.05; // ball rotating speed

//you may need to use some of the following variables in your program 

// No need for model texture, 'cause glmModel() has already loaded it for you.
// To use the texture, check glmModel documentation.
GLuint mainTextureID; // TA has already loaded this texture for you
GLuint noiseTextureID; // TA has already loaded this texture for you
GLuint rampTextureID; // TA has already loaded this texture for you
GLuint vbo_id;
GLuint vaoHandle;
GLuint program[3];
int mode = 0;
char *vertfile[3] = {"shaders/phong.vert", "shaders/dissolving.vert" , "shaders/ramp.vert" };
char *fragfile[3] = {"shaders/phong.frag", "shaders/dissolving.frag" , "shaders/ramp.frag" };
GLfloat Ka[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Kd[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Ks[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat shine = 100;
float step = 0.01; //for dissolving
float threshold = 0;

GLMmodel *model; //TA has already loaded the model for you(!but you still need to convert it to VBO(s)!)

float eyex = 0.0;
float eyey = 0.0;
float eyez = 5.6;

GLfloat light_pos[] = { 1.1, 1.0, 1.3 }; //light positions
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };

int main(int argc, char *argv[]){
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	// remember to replace "YourStudentID" with your own student ID
	glutCreateWindow("CG_HW2_0756134");
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

	glutMainLoop();

	glmDelete(model);
	return 0;
}

void init(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);
	model = glmReadOBJ(obj_file_dir); //object file

	mainTextureID = loadTexture(main_tex_dir, 512, 256);
	noiseTextureID = loadTexture(noise_tex_dir, 360, 360);
	rampTextureID = loadTexture(ramp_tex_dir, 256, 256);
	
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	//you may need to do something here(create shaders/program(s) and create vbo(s)/vao from GLMmodel model)
	for (int i = 0; i < 3; i++) {
		GLuint vert = createShader(vertfile[i], "vertex");
		GLuint frag = createShader(fragfile[i], "fragment");
		program[i] = createProgram(vert, frag);
	}
	//create vbo , need vertex information
	glGenBuffers(1, &vbo_id); //generate a vbo buffer and assign its pointer to vboid
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id); //bind with the buffer , GL_ARRAY_BUFFER is for vertex type

//GLMtriangle: Structure that defines a triangle in a model.
//	/	typedef struct _GLMtriangle {
//		GLuint vindices[3];           /* array of triangle vertex indices */
//		GLuint nindices[3];           /* array of triangle normal indices */
//		GLuint tindices[3];           /* array of triangle texcoord indices*/
//		GLuint findex;                /* index of triangle facet normal */
	int index;
	Vertex *vertices = new Vertex[3 * model->numtriangles]; //total vertices
	for (unsigned int i = 0; i < model->numtriangles; i++) { //all triangles
		for (int j = 0; j < 3; j++) {  //vertices in a triangle
			index = model->triangles[i].vindices[j]; //specific vertex
			//position
			int k;
			for (k = 0; k < 3; k++) {
				vertices[3 * i + j].position[k] = model->vertices[3 * index + k];
			}
			index = model->triangles[i].nindices[j];
			//normal
			for (k = 0; k < 3; k++) {
				vertices[3 * i + j].normal[k] = model->normals[3 * index + k];
			}
			//texture
			index = model->triangles[i].tindices[j]; //
			for (k = 0; k < 2; k++) {
				vertices[3 * i + j].texcoord[k] = model->texcoords[2 * index + k];
			}
		}
	} //get all vertex info we want, give it to openGL
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3 * model->numtriangles, vertices, GL_STATIC_DRAW);  //try Dynamic?
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glEnableVertexAttribArray(0); //VAO[0] for position
	glEnableVertexAttribArray(1); //VAO[1] for normal
	glEnableVertexAttribArray(2); //VAO[2] for textures
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	//normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	//texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texcoord)));

 

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// APIs for creating shaders and creating shader programs have been done by TAs
	// following is an example for creating a shader program using given vertex shader and fragment shader
	/*
	GLuint vert = createShader("Shaders/bump.vert", "vertex");
	GLuint frag = createShader("Shaders/bump.frag", "fragment");
	GLuint program = createProgram(vert, frag);
	*/
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//you may need to do something here(declare some local variables you need and maybe load Model matrix here...)



	//please try not to modify the following block of code(you can but you are not supposed to)
	glMatrixMode(GL_MODELVIEW); //glMatrixMode()函数的参数，这个函数其实就是对接下来要做什么进行一下声明，也就是在要做下一步之前告诉计算机我要对“什么”进行操作了
	glLoadIdentity(); //initialization to I
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
		glUseProgram(program[mode]);
			float modelview[16];
			float proj[16];
			GLint loc;
			loc = glGetUniformLocation(program[mode], "tex");
			glActiveTexture(GL_TEXTURE0 + 0); //GL_TEXTUREi = GL_TEXTURE0 + i
			glBindTexture(GL_TEXTURE_2D, mainTextureID);
			glUniform1i(loc, 0);

			if (mode == 0) { //phong shading
			//phong shading needs position in local space
				glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
				glGetFloatv(GL_PROJECTION_MATRIX, proj);

				loc = glGetUniformLocation(program[0], "modelview"); //get the location of uniform variable in shader
				glUniformMatrix4fv(loc, 1, GL_FALSE, modelview); //assign value to it 
				loc = glGetUniformLocation(program[0], "proj");
				glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
				glm::mat4 Nmatrix = glm::transpose(glm::inverse(glm::make_mat4(modelview)));
				loc = glGetUniformLocation(program[0], "Nmatrix");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Nmatrix));
				loc = glGetUniformLocation(program[0], "Ka");
				glUniform4fv(loc, 1, Ka);
				loc = glGetUniformLocation(program[0], "Kd");
				glUniform4fv(loc, 1, Kd);
				loc = glGetUniformLocation(program[0], "lightnow");
				glUniform3fv(loc, 1, glm::value_ptr(lightnow));
				loc = glGetUniformLocation(program[0], "Ks");
				glUniform4fv(loc, 1, Ks);
				loc = glGetUniformLocation(program[0], "alpha");
				glUniform1f(loc, shine);
			}
			if (mode == 1) { //dissolving effect
				threshold += step;
				if (threshold > 1) {
					threshold = 0;
				}
				//printf("%f\n", threshold);
				glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
				glGetFloatv(GL_PROJECTION_MATRIX, proj);
				loc = glGetUniformLocation(program[1], "modelview"); //get the location of uniform variable in shader
				glUniformMatrix4fv(loc, 1, GL_FALSE, modelview); //assign value to it 
				loc = glGetUniformLocation(program[1], "proj");
				glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
				loc = glGetUniformLocation(program[1], "threshold");
				glUniform1f(loc, threshold);			
				loc = glGetUniformLocation(program[1], "noise");
				glActiveTexture(GL_TEXTURE1); //GL_TEXTUREi = GL_TEXTURE0 + i
				glBindTexture(GL_TEXTURE_2D, noiseTextureID);
				glUniform1i(loc, 1);
			}
			if (mode == 2) { //ramp
				//need diffuse
				glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
				glGetFloatv(GL_PROJECTION_MATRIX, proj);

				loc = glGetUniformLocation(program[2], "modelview"); //get the location of uniform variable in shader
				glUniformMatrix4fv(loc, 1, GL_FALSE, modelview); //assign value to it 
				loc = glGetUniformLocation(program[2], "proj");
				glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
				glm::mat4 Nmatrix = glm::transpose(glm::inverse(glm::make_mat4(modelview)));
				loc = glGetUniformLocation(program[2], "Nmatrix");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Nmatrix));
				loc = glGetUniformLocation(program[2], "Ka");
				glUniform4fv(loc, 1, Ka);
				loc = glGetUniformLocation(program[2], "Kd");
				glUniform4fv(loc, 1, Kd);
				loc = glGetUniformLocation(program[2], "lightnow");
				glUniform3fv(loc, 1, glm::value_ptr(lightnow));
				loc = glGetUniformLocation(program[2], "Ks");
				glUniform4fv(loc, 1, Ks);
				loc = glGetUniformLocation(program[2], "alpha");
				glUniform1f(loc, shine);				
				loc = glGetUniformLocation(program[2], "ramp");
				glActiveTexture(GL_TEXTURE1); //GL_TEXTUREi = GL_TEXTURE0 + i
				glBindTexture(GL_TEXTURE_2D, rampTextureID);
				glUniform1i(loc, 1);


			}
			glDrawArrays(GL_TRIANGLES, 0, 3 * model->numtriangles);

			glBindTexture(GL_TEXTURE_2D, NULL);

		glUseProgram(0);

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
	case 'b'://toggle mode
	{	
		mode = (mode + 1) % 3;
		//you may need to do somting here
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
	if (mleft)
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
