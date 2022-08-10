//  ========================================================================
//  COSC422: Computer Graphics (2022);  University of Canterbury.
//  FILE NAME: main.cpp
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  This program requires the following files:
//         TerrainPatches.vert, TerrainPatches.frag
//         TerrainPatches.cont, TerrainPatches.eval
//  ========================================================================
#define GLM_FORCE_SWIZZLE
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h" 

using namespace std;

GLuint vaoID;
GLuint texID[5];
GLuint theProgram;
GLuint mvpMatrixLoc, eyeLoc, lgtLoc, watLoc, snwLoc, texNum, fogEn;
float lookAngle = 180.0, lookDepth = 20.0;
float  eye_x = 0, eye_y = 40, eye_z = 30;      //Initial camera position
float look_x = 0, look_y = -12, look_z = -40;    //"Look-at" point along -z direction
float toRad = 3.14159265/180.0;     //Conversion from degrees to rad
bool textureMode = true;
GLfloat snowHeight = 0.8, waterHeight = 0.4, lightHeight = 80.0, lightx=40.0, lightz=-100.0;
GLint textureNumber = 0, fogEnable = 0;
float perlinGrid[1024 * 1024];

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];     //Element array for 9x9 = 81 quad patches

glm::mat4 projView;

//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;
	//verts array
	for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
	{
		for(int j = 0; j < 10; j++)
		{
			indx = 10*i + j;
			verts[3*indx] = 10*i - 45;		//x
			verts[3*indx+1] = 0;			//y
			verts[3*indx+2] = -10*j;		//z
		}
	}

	//elems array
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			indx = 9*i +j;
			start = 10*i + j;
			elems[4*indx] = start;
			elems[4*indx+1] = start + 10;
			elems[4*indx+2] = start + 11;
			elems[4*indx+3] = start + 1;			
		}
	}
}

//Loads height map
void loadTexture()
{
    glGenTextures(6, texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
	loadTGA("heightMaps/Terrain_hm_01.tga");
	//generateProcedural(64, 64);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texID[1]);
	loadTGA("images/snow_m.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texID[2]);
	loadTGA("images/rocks.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texID[3]);
	loadTGA("images/water01.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texID[4]);
	loadTGA("images/grass02.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texID[5]);
	loadTGA("heightMaps/Terrain_hm_03.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}

//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}

//Initialise the shader program, create and load buffer data
void initialise()
{
//--------Load terrain height map-----------
	loadTexture();
//--------Load shaders----------------------
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "TerrainPatches.vert");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "TerrainPatches.frag");
	GLuint shaderc = loadShader(GL_TESS_CONTROL_SHADER, "TerrainPatches.cont");
	GLuint shadere = loadShader(GL_TESS_EVALUATION_SHADER, "TerrainPatches.eval");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "TerrainPatches.geom");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shaderf);
	glAttachShader(program, shaderc);
	glAttachShader(program, shadere);
	glAttachShader(program, shaderg);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	glUseProgram(program);

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	eyeLoc = glGetUniformLocation(program, "eyePos");
	lgtLoc = glGetUniformLocation(program, "lgtLoc");
	snwLoc = glGetUniformLocation(program, "snwLoc");
	watLoc = glGetUniformLocation(program, "watLoc");
	texNum = glGetUniformLocation(program, "texNum");
	fogEn = glGetUniformLocation(program, "fogEnable");

	GLuint mapTexLoc = glGetUniformLocation(program, "heightMap");
	glUniform1i(mapTexLoc, 0);

	GLuint snowTexLoc = glGetUniformLocation(program, "Snowimage");  //tex1 is a uniform variable of type Sampler2D in the fragment shader
	glUniform1i(snowTexLoc, 1);    //Assign a value 0 to the variable tex1 to specify that it should use the texture from unit 0.

	GLuint rockTexLoc = glGetUniformLocation(program, "Rockimage");  //tex1 is a uniform variable of type Sampler2D in the fragment shader
	glUniform1i(rockTexLoc, 2);    //Assign a value 0 to the variable tex1 to specify that it should use the texture from unit 0.

	GLuint waterTexLoc = glGetUniformLocation(program, "Waterimage");  //tex1 is a uniform variable of type Sampler2D in the fragment shader
	glUniform1i(waterTexLoc, 3);    //Assign a value 0 to the variable tex1 to specify that it should use the texture from unit 0.

	GLuint grassTexLoc = glGetUniformLocation(program, "Grassimage");  //tex1 is a uniform variable of type Sampler2D in the fragment shader
	glUniform1i(grassTexLoc, 4);    //Assign a value 0 to the variable tex1 to specify that it should use the texture from unit 0.

	GLuint mapTex1Loc = glGetUniformLocation(program, "heightMap1");
	glUniform1i(mapTex1Loc, 5);

//---------Load buffer data-----------------------
	generateData();

	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

glm::mat4 getLookView(glm::vec3 eyePos, float angle, float depth)
{
	look_x = depth * sin(toRad * angle);
	look_z = depth * cos(toRad * angle);
	glm::vec3 lookAtPos = glm::vec3(look_x, look_y, look_z) + eyePos;
	glm::mat4 view = lookAt(eyePos, lookAtPos, glm::vec3(0.0, 1.0, 0.0));
	return view;
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	glm::vec4 cameraPosn = glm::vec4(eye_x, eye_y, eye_z, 1.0);
	glm::vec4 light = glm::vec4(lightx, lightHeight, lightz, 1.0);
	
	//--------Compute matrices----------------------
	glm::mat4 proj = glm::perspective(30.0f * toRad, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
	
	glm::mat4 view = getLookView(cameraPosn.xyz(), lookAngle, lookDepth);
	projView = proj * view;  //Product matrix
	glm::vec4 lightEye = projView * light;     //Light position in eye coordinates

	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
	glUniform4fv(lgtLoc, 1, &light[0]);
	glUniform4fv(eyeLoc, 1, &cameraPosn[0]);
	glUniform1f(watLoc, waterHeight);
	glUniform1f(snwLoc, snowHeight);
	glUniform1i(texNum, textureNumber);
	glUniform1i(fogEn, fogEnable);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 81 * 4, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}

void special(int key, int x, int y) 
{
	//Camera Movement
	if (key == GLUT_KEY_UP) {
		eye_x += 1 * sin(toRad * lookAngle);
		eye_z += 1 * cos(toRad * lookAngle);
	}
	else if (key == GLUT_KEY_DOWN) {
		eye_x -= 1 * sin(toRad * lookAngle);
		eye_z -= 1 * cos(toRad * lookAngle);
	};
	if (key == GLUT_KEY_LEFT) {
		eye_x += 1 * sin(toRad * (lookAngle + 90));
		eye_z += 1 * cos(toRad * (lookAngle + 90));
	}
	else if (key == GLUT_KEY_RIGHT) {
		eye_x -= 1 * sin(toRad * (lookAngle + 90));
		eye_z -= 1 * cos(toRad * (lookAngle + 90));
	};

	//Camera Angle
	if (key == GLUT_KEY_SHIFT_R) eye_y--;

	float level = 13 - (eye_z - 10)/4;
	glutPostRedisplay();

}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 13) eye_y++;

	if (key == '['){
		lookAngle+=10;
	} 
	else if (key == ']') {
		lookAngle-=10;
	}

	if (key == 'q') waterHeight+=0.05;
	else if (key == 'a') waterHeight-=0.05;
	if (waterHeight > 1.0) waterHeight = 1.0;
	if (waterHeight < 0.0) waterHeight = 0.0;
	
	if (key == 'w') snowHeight+=0.05;
	else if (key == 's') snowHeight-=0.05;
	if (snowHeight > 1.0) snowHeight = 1.0;
	if (snowHeight < 0.0) snowHeight = 0.0;

	if (key == 'e') lightHeight+=10;
	else if (key == 'd') lightHeight-=10;
	if (key == 'l') lightx+=10;
	else if (key == 'j') lightx-=10;
	if (key == 'k') lightz+=10;
	else if (key == 'i') lightz-=10;

	if (key == 'e' |key == 'd' |key == 'l' |key == 'j' |key == 'k' |key == 'i' )
		printf("x:%f, y:%f, z:%f\n", lightx, lightHeight, lightz);

	if (key == '1') textureNumber = 0;
	else if (key == '2') textureNumber =1;

	if (key == 'f') {
		fogEnable = !fogEnable;
	}
	if (snowHeight < waterHeight) snowHeight = waterHeight + 0.02;

	//printf("x:%f, y:%f, z:%f\n", look_x, look_y, look_z);
	if (key == ' ') {
		textureMode = !textureMode;
		if (textureMode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			printf("Switching to texture mode!\n");
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			printf("Switching to wireframe mode!\n");
		}
	}

	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Terrain");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise();
	glutDisplayFunc(display); 
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}

