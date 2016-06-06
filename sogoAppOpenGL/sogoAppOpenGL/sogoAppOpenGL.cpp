// sogoAppOpenGL.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

//#include "stdafx.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Befindet sich bei den OpenGL-Tutorials unter "common"
#include "shader.hpp"

// Wuerfel und Kugel
#include "objects.hpp"
#include "objloader.hpp"
#include "texture.hpp"

// =========================================================================
//
//	variables
//
// =========================================================================

/*
	Variablen zur Steuerung
*/
float winkel = 0;
float x_achse = 0;
float y_achse = 0;
float z_achse = 0;

float left = 0;
float right = 0;
float up = 0;
float down = 0;

float firstSeg = 0;
float secSeg = 0;
float thSeg = 0;

// Diese Drei Matrizen global (Singleton-Muster), damit sie jederzeit modifiziert und
// an die Grafikkarte geschickt werden koennen
glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 Save;
GLuint programID;

std::vector<glm::vec2> uvs;

// =========================================================================
//
//	method
//
// =========================================================================

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}


/*
	Tastensteuerung
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;

	// Bewegung mit Pfeiltaste
	case GLFW_KEY_LEFT:
		left += 5;
		break;

	case GLFW_KEY_RIGHT:
		right += 5;
		break;

	case GLFW_KEY_UP:
		up += 5;
		break;

	case GLFW_KEY_DOWN:
		down += 5;
		break;

	default:
		break;
	}
}

void sendMVP()
{
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model;
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform, konstant fuer alle Eckpunkte
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);

	/*
	// Aufgabe 6
	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &Projection[0][0]);
	*/
}



// =========================================================================
//
//	main
//
// =========================================================================

int main(void)
{

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Fehler werden auf stderr ausgegeben, s. o.
	glfwSetErrorCallback(error_callback);

	// Open a window and create its OpenGL context
	// glfwWindowHint vorher aufrufen, um erforderliche Resourcen festzulegen
	GLFWwindow* window = glfwCreateWindow(1024, // Breite
		768,  // Hoehe
		"Sogo", // Ueberschrift
		NULL,  // windowed mode
		NULL); // shared windoe

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make the window's context current (wird nicht automatisch gemacht)
	// aktiviert das entsprechende (brnoetigte) Fenster
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	// GLEW ermöglicht Zugriff auf OpenGL-API > 1.1
	glewExperimental = true; // Needed for core profile

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Auf Keyboard-Events reagieren
	glfwSetKeyCallback(window, key_callback);

	// white background
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	// Punkte die kleiner sind kommen durch.
	glDepthFunc(GL_LESS);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Shader auch benutzen !
	glUseProgram(programID);

	// =========================================================================
	//
	//	Kannenmodell
	//
	// =========================================================================

	/*
	// --- Kannenmodell
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	// Laden der Objektdatei
	bool res = loadOBJ("teapot.obj", vertices, uvs, normals);

	// Jedes Objekt eigenem VAO zuordnen, damit mehrere Objekte moeglich sind
	// VAOs sind Container fuer mehrere Buffer, die zusammen gesetzt werden sollen.
	GLuint VertexArrayIDTeapot;
	glGenVertexArrays(1, &VertexArrayIDTeapot);
	glBindVertexArray(VertexArrayIDTeapot);

	// Ein ArrayBuffer speichert Daten zu Eckpunkten (hier xyz bzw. Position)
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer); // Kennung erhalten
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); // Daten zur Kennung definieren
												 // Buffer zugreifbar für die Shader machen
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// Erst nach glEnableVertexAttribArray kann DrawArrays auf die Daten zugreifen...
	glEnableVertexAttribArray(0); // siehe layout im vertex shader: location = 0 
	glVertexAttribPointer(0,  // location = 0 
		3,  // Datenformat vec3: 3 floats fuer xyz 
		GL_FLOAT,
		GL_FALSE, // Fixedpoint data normalisieren ?
		0, // Eckpunkte direkt hintereinander gespeichert
		(void*)0); // abweichender Datenanfang ? 


				   // Aufgabe 6
	GLuint normalbuffer; // Hier alles analog für Normalen in location == 2
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2); // siehe layout im vertex shader 
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Aufgabe 7
	GLuint uvbuffer; // Hier alles analog für Texturkoordinaten in location == 1 (2 floats u und v!)
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1); // siehe layout im vertex shader 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// Load the texture
	GLuint Texture = loadBMP_custom("mandrill.bmp");

	*/

	// --- Teil Kannenmodell


	GLuint uvbuffer; // Hier alles analog f�r Texturkoordinaten in location == 1 (2 floats u und v!)
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	//glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1); // siehe layout im vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// Load the texture
	GLuint Texture = loadBMP_custom("mandrill.bmp");

	// =========================================================================
	//
	//	Eventloop
	//
	// =========================================================================
	
	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		//glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

		// Camera matrix
		View = glm::lookAt(glm::vec3(0, 0, -20), // Camera is at (0,0,-5), in World Space
			glm::vec3(0, 0, 0),  // and looks at the origin
			glm::vec3(0, 1, 0)); // Head is up (set to 0,-1,0 to look upside-down)

								 // Model matrix : an identity matrix (model will be at the origin)
		Model = glm::mat4(1.0f);

		
		// Modellierung mit Pfeiltasten
		Model = glm::rotate(Model, up, glm::vec3(1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, down, glm::vec3(-1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, left, glm::vec3(0.0f, -1.0f, 0.0f));
		Model = glm::rotate(Model, right, glm::vec3(0.0f, 1.0f, 0.0f));

		Save = Model;

		// =========================================================================
		//
		//	Playfield
		//
		// =========================================================================

		float x, y, z, koord1, koord2;
		
		koord1 = 3.1;
		for (y = 0; y <= koord1; y += koord1/3)
		{
			for (x = 0; x <= koord1; x += koord1/3)
			{
				for (z = 0; z <= koord1; z += koord1/3)
				{
					/*
					Model = Save;
					Model = glm::scale(Model, glm::vec3(0.2, 0.2, 0.2));
					Model = glm::translate(Model, glm::vec3(x, y, z));
					drawSphere(10, 200);
					sendMVP();
					*/

					Model = Save;
					Model = glm::translate(Model, glm::vec3(x, y, z));
					Model = glm::scale(Model, glm::vec3(0.1, 0.5, 0.1));
					drawCube();
					sendMVP();
				}
			}
		}

		for (y = 0; y <= koord1; y += koord1 / 3)
		{
			for (x = 6; x <= koord1+6; x += koord1 / 3)
			{
				for (z = 0; z <= koord1; z += koord1 / 3)
				{
				
					Model = Save;
					Model = glm::translate(Model, glm::vec3(x, y, z));
					Model = glm::scale(Model, glm::vec3(0.2, 0.2, 0.2));
					drawSphere(10, 200);
					sendMVP();
				
					/*
					Model = Save;
					Model = glm::translate(Model, glm::vec3(x, y, z));
					Model = glm::scale(Model, glm::vec3(0.1, 0.5, 0.1));
					drawCube();
					sendMVP();
					*/
				}
			}
		}
		
		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	// Aufgabe 6
	//glDeleteBuffers(1, &normalbuffer);

	// Aufgabe 7
	//glDeleteBuffers(1, &uvbuffer);
	//glDeleteTextures(1, &Texture);

	//glDeleteBuffers(1, &vertexbuffer);

	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}



