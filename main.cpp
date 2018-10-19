// Mohammad Imrul Jubair
// cite: http://www.opengl-tutorial.org/
// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>

using namespace std;
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseCallback(GLFWwindow* window, int button, int action, int mods);

glm::mat4 rotation = mat4(1.0f);
glm::mat4 scaling = mat4(1.0f);


float scalar = 1.0f;
string filenameVert = "./vert_data/vrt.txt"; // vertices
string filenameData = "./vert_data/dt.txt"; // data

int main()
{

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 800, 600, "Rendering Project", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	// Dark blue background
	glClearColor(0.8f, 0.8f, 0.95f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "./shaders/VertexShader.vertexshader", "./shaders/FragmentShader.fragmentshader" );
	GLuint programIDedge = LoadShaders( "./shaders/VertexShader.vertexshader", "./shaders/edge.fragmentshader" );


	// loading vertices
	std::vector<float> g_vertex_buffer_data;

	ifstream inFile;
	float temp;
	int count = 0;
	inFile.open(filenameVert);
	while(!inFile.eof())
	{
			inFile >> temp;
			count = count + 1;
			g_vertex_buffer_data.push_back(temp);
	}


	// loading data
	std::vector<float> g_color_buffer_data;

	ifstream inFileData;
	inFileData.open(filenameData);
	while(!inFileData.eof())
	{
				inFileData >> temp;
				g_color_buffer_data.push_back(temp);
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * g_vertex_buffer_data.size(), g_vertex_buffer_data.data() , GL_STATIC_DRAW);


	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * g_color_buffer_data.size(), g_color_buffer_data.data(), GL_STATIC_DRAW);


	do{

				GLuint MatrixID = glGetUniformLocation(programID, "MVP");

				glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

				glm::mat4 View  = glm::lookAt(
													glm::vec3(0,0,-35), // Camera is at (4,3,-3), in World Space
													glm::vec3(0,0,0), // and looks at the origin
													glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   						);

				glm::mat4 Model = glm::mat4(1.0f);
				scaling = scale (mat4(1.0f), vec3 (scalar)) * scaling;
				View	*= scaling*rotation;
				glm::mat4 MVP  = Projection * View * Model;


				// Clear the screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Use our shader
				glUseProgram(programID);

				// Send our transformation to the currently bound shader,
				// in the "MVP" uniform
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

				// 2nd attribute buffer : colors
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );


				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDrawArrays(GL_TRIANGLES, 0, count);

				// 2nd pass for render with edges

				glUseProgram(programIDedge);
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				// Draw the triangle !
				glLineWidth(1.13f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawArrays(GL_TRIANGLES, 0, count);


				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);

				// Swap buffers
				glfwSwapBuffers(window);
				glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteProgram(programIDedge);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
		 rotation = rotate(mat4(1.0f), 0.1f, vec3 (0.0f, 1.0f, 0.0f)) * rotation;
		}
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
		rotation = rotate(mat4(1.0f), 0.1f, vec3 (0.0f, -1.0f, 0.0f)) * rotation;
		}

	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)){
		 rotation = rotate(mat4(1.0f), 0.1f, vec3 (1.0f, 0.0f, 0.0f)) * rotation;
		}
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)){
		rotation = rotate(mat4(1.0f), 0.1f, vec3 (-1.0f, 0.0f, 0.0f)) * rotation;
		}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    scalar = 1.01f;
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    scalar = 1.0f;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    scalar = 1.0f / 1.01f;
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    scalar = 1.0f;
}
