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
#include "ModelGroup.h"

extern "C"
{
	#include "glm_helper.h"
}

#define PROGRAM_NUM (3)
#define OBJ_NUM (3)
#define deltaTime (10)		// in ms (1e-3 second)

//no need to modify the following function declarations and gloabal variables
void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
bool intersectSphere(glm::vec3, const glm::vec3&, const glm::vec3&, float, float&, float&);
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

// models
int modelIdx = 0;
ModelGroup models[OBJ_NUM];
GLuint vbo_id[OBJ_NUM];
GLuint vbo_line_id[OBJ_NUM];
GLuint vaoHandle[OBJ_NUM];
GLuint vaoHandle_line[OBJ_NUM];

// programs
int mode = 0;
GLuint program[PROGRAM_NUM];
GLuint program_line[PROGRAM_NUM];
char *vertfile[PROGRAM_NUM] = {"shaders/explode.vert", "shaders/explode_scanline.vert" , "shaders/explode_point.vert" };
char *fragfile[PROGRAM_NUM] = {"shaders/explode.frag", "shaders/explode_scanline.frag" , "shaders/explode_point.frag" };
char *vertfile_line[PROGRAM_NUM] = { "shaders/mesh.vert", "shaders/mesh.vert", "shaders/mesh.vert" };
char *fragfile_line[PROGRAM_NUM] = { "shaders/mesh.frag", "shaders/mesh.frag", "shaders/mesh.frag" };

// some basic parameters
glm::vec2 windowSize(512.0, 512.0);

GLfloat Ka[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Kd[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Ks[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat shine = 100;

float eyex = 0.0;
float eyey = 0.0;
float eyez = 5.6;

GLfloat light_pos[] = { 1.1, 1.0, 1.3 }; //light positions
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };

// for all explode effect
bool pause = true;
const float showMeshTime = 1.0;
float showMeshValue = 0.0;
	// discard some traingles of OBJ with complicate mesh for better visual effect, differ with different models
float showPercent[OBJ_NUM] = { 0.01, 0.5, 0.1 };
	// enlarge size of mesh after discarding, differ with different models and showPercent
float meshEnlargeSize[OBJ_NUM] = { 10, 1, 3 };

// for normal explode effect
const float expandTime = 2.0;
const float startFadePercent = 0.5;
const float fadeTime = 2.0;
float expandValue = 0.0;
float fadeValue = 0.0;

// for scan line explode effect
const float scanTime = 2.0;
float scanlineValue = -0.1;

// for point explode effect
glm::vec3 raycastPoint;
const float spreadTime = 2.0;
float spreadValue = 0.0;

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
			if (mode == 0) {
				if (expandValue < 1) {
					expandValue = std::fmin(expandValue + d / expandTime, 1);
				}
				if (expandValue >= startFadePercent && fadeValue < 1) {
					fadeValue = std::fmin(fadeValue + d / fadeTime, 1);
				}
			}
			else if (mode == 1) {
				if (scanlineValue < 1.5) {
					scanlineValue = std::fmin(scanlineValue + d / scanTime, 1.5);
				}
			}
			else if (mode == 2) {
				if (spreadValue < 2) {
					spreadValue = std::fmin(spreadValue + d / spreadTime, 2);
				}
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
	glutReshapeWindow(windowSize.x, windowSize.y);

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

	return 0;
}

// init death effect while changing effect or model, or restart
void initDeath(void) {
	showMeshValue = expandValue = fadeValue = 0;
	scanlineValue = -0.1;
	spreadValue = 0;
	randomSeed = dis(gen);
	pause = true;
}
void init(void) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	mainTextureID = loadTexture(main_tex_dir, 512, 256);

	// create shader programs for exlpode effect and mesh line
	for (int i = 0; i < PROGRAM_NUM; i++) {
		GLuint vert = createShader(vertfile[i], "vertex");
		GLuint frag = createShader(fragfile[i], "fragment");
		program[i] = createProgram(vert, frag);
	}
	for (int i = 0; i < PROGRAM_NUM; i++) {
		GLuint vert = createShader(vertfile_line[i], "vertex");
		GLuint frag = createShader(fragfile_line[i], "fragment");
		program_line[i] = createProgram(vert, frag);
	}
	// create vbos and vaos
	glGenBuffers(OBJ_NUM, vbo_id);					// generate vbo buffers and assign their pointer to vboid
	glGenVertexArrays(OBJ_NUM, vaoHandle);			// generate vao arrays and assign their pointer to vaoid
	// create vbos and vaos of line mesh
	glGenBuffers(OBJ_NUM, vbo_line_id);
	glGenVertexArrays(OBJ_NUM, vaoHandle_line);
	// load model into vbos and vaos
	for (int idx = 0; idx < OBJ_NUM; idx++) {
		models[idx] = ModelGroup(obj_file_dir[idx]);
		models[idx].constructVO(vbo_id[idx], vaoHandle[idx]);
		models[idx].constructLineVO(vbo_line_id[idx], vaoHandle_line[idx]);
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void setGlobalMV(void) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();				//initialization to Identity matrix
	gluLookAt(
		eyex,			//our head
		eyey,
		eyez,
		eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180),  //look at some position
		eyey + sin(eyet*M_PI / 180),
		eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
		0.0,
		1.0,
		0.0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//please try not to modify the following block of code(you can but you are not supposed to)
	setGlobalMV();
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
	
		float modelview[16], proj[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
		GLint loc;
		
		glBindVertexArray(vaoHandle[modelIdx]);
		glUseProgram(program[mode]);
			// main texture
			loc = glGetUniformLocation(program[mode], "tex");
			glActiveTexture(GL_TEXTURE0 + 0); //GL_TEXTUREi = GL_TEXTURE0 + i
			glBindTexture(GL_TEXTURE_2D, mainTextureID);
			glUniform1i(loc, 0);

			// model view matrix
			loc = glGetUniformLocation(program[mode], "modelview");	//get the location of uniform variable in shader
			glUniformMatrix4fv(loc, 1, GL_FALSE, modelview);		//assign value to it 
			// projection matrix
			loc = glGetUniformLocation(program[mode], "proj");
			glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
			// all modes have basic phong shading
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

			// parameters for all explode effects
			if (showMeshValue == 1)
				glDisable(GL_CULL_FACE);
			loc = glGetUniformLocation(program[mode], "showMeshValue");
			glUniform1f(loc, showMeshValue);
			loc = glGetUniformLocation(program[mode], "showPercent");
			glUniform1f(loc, showPercent[modelIdx]);
			loc = glGetUniformLocation(program[mode], "meshEnlargeSize");
			glUniform1f(loc, meshEnlargeSize[modelIdx]);
			loc = glGetUniformLocation(program[mode], "seed");
			glUniform1f(loc, randomSeed);
			if (mode == 0) {							// normal explode effect
				loc = glGetUniformLocation(program[mode], "expandValue");
				glUniform1f(loc, expandValue);
				loc = glGetUniformLocation(program[mode], "fadeValue");
				glUniform1f(loc, fadeValue);
			}
			if (mode == 1) {							// explosion with scanline
				loc = glGetUniformLocation(program[mode], "scanlineValue");
				glUniform1f(loc, scanlineValue);
				loc = glGetUniformLocation(program[mode], "min_y");
				glUniform1f(loc, models[modelIdx].getBoundingY().first);
				loc = glGetUniformLocation(program[mode], "max_y");
				glUniform1f(loc, models[modelIdx].getBoundingY().second);
			}
			else if (mode == 2) {						// explode from point
				loc = glGetUniformLocation(program[mode], "mRaycastPoint");
				glUniform3fv(loc, 1, glm::value_ptr(raycastPoint));
				loc = glGetUniformLocation(program[mode], "spreadValue");
				glUniform1f(loc, spreadValue);
				loc = glGetUniformLocation(program[mode], "longestDis");
				glUniform1f(loc, models[modelIdx].getBoundingRadius() + glm::length(raycastPoint));
			}
			glDrawArrays(GL_TRIANGLES, 0, models[modelIdx].getVertexNum());
		glUseProgram(0);
		
		glEnable(GL_CULL_FACE);	
		glBindTexture(GL_TEXTURE_2D, NULL);

		//draw mesh
		if (showMeshValue > 0 && showMeshValue < 1) {
			glBindVertexArray(vaoHandle_line[modelIdx]);
			glUseProgram(program_line[mode]);
				glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
				glGetFloatv(GL_PROJECTION_MATRIX, proj);
				loc = glGetUniformLocation(program_line[mode], "modelview"); //get the location of uniform variable in shader
				glUniformMatrix4fv(loc, 1, GL_FALSE, modelview); //assign value to it 
				loc = glGetUniformLocation(program_line[mode], "proj");
				glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
				loc = glGetUniformLocation(program_line[mode], "showMeshValue");
				glUniform1f(loc, showMeshValue);
				glLineWidth(2.f);
				glDrawArrays(GL_LINES, 0, 6 * models[modelIdx].getVertexNum());
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
	windowSize = glm::vec2(width, height);
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
			if (!pause && showMeshValue == 0 && mode == 2) {
				// get raycast point
				glm::vec2 mouse = (glm::vec2(x, y) / windowSize) * 2.0f - 1.0f;
				mouse.y = -mouse.y; //origin is top-left and +y mouse is down

				// set global modelview
				setGlobalMV();

				// get raycast point in global coordinate
				float modelview[16], projection[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
				glGetFloatv(GL_PROJECTION_MATRIX, projection);
				glm::mat4 toWorld = glm::inverse(glm::make_mat4(projection) * glm::make_mat4(modelview));

				glm::vec4 from = toWorld * glm::vec4(mouse, -1.0f, 1.0f);
				glm::vec4 to = toWorld * glm::vec4(mouse, 1.0f, 1.0f);

				from /= from.w; //perspective divide ("normalize" homogeneous coordinates)
				to /= to.w;

				float t1, t2;
				glm::vec3 direction = glm::normalize(glm::vec3(to - from));
				if (intersectSphere(glm::vec3(from), direction, glm::make_vec3(ball_pos), models[modelIdx].getBoundingRadius(), t1, t2)) {
					//object i has been clicked. probably best to find the minimum t1 (front-most object)
					raycastPoint = glm::vec3(from) + direction * t1;
				}
				else {
					raycastPoint = glm::make_vec3(ball_pos);
				}
				//printf("global: (%.2f %.2f %.2f)\n", raycastPoint.x, raycastPoint.y, raycastPoint.z);

				// change to model coordinate
				glm::vec4 mvRaycastPoint = glm::make_mat4(modelview) * glm::vec4(raycastPoint, 1.0);
				mvRaycastPoint /= mvRaycastPoint.w;
				glPushMatrix();
					glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
					glRotatef(ball_rot[0], 1, 0, 0);
					glRotatef(ball_rot[1], 0, 1, 0);
					glRotatef(ball_rot[2], 0, 0, 1);
					glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
					glm::vec4 mRaycastPoint = glm::inverse(glm::make_mat4(modelview)) * mvRaycastPoint;
					raycastPoint = glm::vec3( mRaycastPoint / mRaycastPoint.w);
				glPopMatrix();
				//printf("model: (%.2f %.2f %.2f)\n", raycastPoint.x, raycastPoint.y, raycastPoint.z);
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

//ray at position p with direction d intersects sphere at b with radius r. returns intersection times along ray t1 and t2
bool intersectSphere(glm::vec3 p, const glm::vec3& d, const glm::vec3& b, float r, float& t1, float& t2) {
	//http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection
	p -= b;
	float A = glm::dot(d, d);
	float B = 2.0f * glm::dot(d, p);
	float C = glm::dot(p, p) - r * r;

	float dis = B * B - 4.0f * A * C;

	if (dis < 0.0f)
		return false;

	float S = sqrt(dis);

	t1 = (-B - S) / (2.0f * A);
	t2 = (-B + S) / (2.0f * A);
	return true;
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
