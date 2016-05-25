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


void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

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


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;

	case GLFW_KEY_SPACE:
		winkel += 10;
		break;

	case GLFW_KEY_X:
		x_achse += 10;
		break;

	case GLFW_KEY_Z:
		y_achse += 10;
		break;

	case GLFW_KEY_Y:
		z_achse += 10;
		break;

		// Bewegung mit Pfeiltaste
	case GLFW_KEY_LEFT:
		left += 10;
		break;

	case GLFW_KEY_RIGHT:
		right += 10;
		break;

	case GLFW_KEY_UP:
		up += 10;
		break;

	case GLFW_KEY_DOWN:
		down += 10;
		break;

	case GLFW_KEY_1:
		firstSeg += 10;
		break;

	case GLFW_KEY_2:
		secSeg += 10;
		break;

	case GLFW_KEY_3:
		thSeg += 10;
		break;

	default:
		break;
	}
}


// Diese Drei Matrizen global (Singleton-Muster), damit sie jederzeit modifiziert und
// an die Grafikkarte geschickt werden koennen
glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 Save;
GLuint programID;


void sendMVP()
{
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model;
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform, konstant fuer alle Eckpunkte
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);


	// Aufgabe 6
	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &Projection[0][0]);
}


// Teilaufgabe 9.1 - Koordinatensystem erzeugen
void drawCs(float x, float y, float z)
{
	Save = Model;
	Model = glm::scale(Model, glm::vec3(x, y, z));
	sendMVP();
	drawCube();
	Model = Save;

	Save = Model;
	Model = glm::scale(Model, glm::vec3(y, x, z));
	sendMVP();
	drawCube();
	Model = Save;

	Save = Model;
	Model = glm::scale(Model, glm::vec3(z, y, x));
	sendMVP();
	drawCube();
	Model = Save;
}

// Teilaufgabe 9.2 - Koordinatensystem erzeugen
void drawSeg(float h)
{
	Save = Model;
	Model = glm::translate(Model, glm::vec3(0.0, h / 2, 0.0));
	Model = glm::scale(Model, glm::vec3(h / 6, h / 2, h / 6));
	sendMVP();
	drawSphere(10, 10);
	Model = Save;
}

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
		"CG - Tutorial", // Ueberschrift
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
	// ohne Farbe
	//programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	// mit Farbe
	//programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
	// Aufgabe 6
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Shader auch benutzen !
	glUseProgram(programID);


	// --- Kannenmodell
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("teapot.obj", vertices, uvs, normals);
	// Hallo

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


	// --- Teil Kannenmodell

	// Eventloop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		//glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

		// Camera matrix
		View = glm::lookAt(glm::vec3(0, 0, -5), // Camera is at (0,0,-5), in World Space
			glm::vec3(0, 0, 0),  // and looks at the origin
			glm::vec3(0, 1, 0)); // Head is up (set to 0,-1,0 to look upside-down)

								 // Model matrix : an identity matrix (model will be at the origin)
		Model = glm::mat4(1.0f);

		// Modellierung

		Model = glm::rotate(Model, x_achse, glm::vec3(1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, y_achse, glm::vec3(0.0f, 1.0f, 0.0f));
		Model = glm::rotate(Model, z_achse, glm::vec3(0.0f, 0.0f, 1.0f));


		/*
		// Modellierung mit Pfeiltasten
		Model = glm::rotate(Model, up, glm::vec3(1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, down, glm::vec3(-1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, left, glm::vec3(0.0f, -1.0f, 0.0f));
		Model = glm::rotate(Model, right, glm::vec3(0.0f, 1.0f, 0.0f));
		*/


		Save = Model;

		// Aufgabe 8
		// Kanne
		Save = Model;
		Model = glm::translate(Model, glm::vec3(1.5, 0.0, 0.0));
		Model = glm::scale(Model, glm::vec3(1.0 / 1000.0, 1.0 / 1000.0, 1.0 / 1000.0));


		// Aufgabe 6
		//glm::vec3 lightPos = glm::vec3(4, 4, -4);
		//glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);

		// Aufgabe 7
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);				// Die Textturen sind durchnummeriert
		glBindTexture(GL_TEXTURE_2D, Texture);		// Verbindet die Textur
													// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

		sendMVP();

		glBindVertexArray(VertexArrayIDTeapot);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		Model = Save;
		// Modellierung mit Pfeiltasten

		Model = glm::rotate(Model, up, glm::vec3(1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, down, glm::vec3(-1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, left, glm::vec3(0.0f, -1.0f, 0.0f));
		Model = glm::rotate(Model, right, glm::vec3(0.0f, 1.0f, 0.0f));

		drawCs(-0.6, 0.01, 0.01);


		Model = Save;

		// Erste Segment
		Model = glm::rotate(Model, firstSeg, glm::vec3(1.0f, 0.0f, 0.0f));
		drawSeg(0.5);

		// Zweite Segment
		Model = glm::translate(Model, glm::vec3(0.0, 0.5, 0.0));
		Model = glm::rotate(Model, secSeg, glm::vec3(1.0f, 0.0f, 0.0f));
		drawSeg(0.4);
		drawCs(-0.4, 0.01, 0.01);

		// Dritte Segment
		Model = glm::translate(Model, glm::vec3(0.0, 0.4, 0.0));
		Model = glm::rotate(Model, thSeg, glm::vec3(1.0f, 0.0f, 0.0f));
		drawSeg(0.3);
		drawCs(-0.2, 0.01, 0.01);

		// Lichtposition an der Spitze des letzten Segments
		Model = glm::translate(Model, glm::vec3(0.0, 0.3, 0.0));
		glm::vec4 lightPos = Model * glm::vec4(0, 0, 0, 1);
		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);



		// Aufgabe 8
		// Kugel
		/*
		Model = Save;
		Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));
		// Transformationsmatrizen an den Shader senden
		sendMVP();
		drawSphere(10, 10);
		*/

		// Würfel
		/*
		Model = Save;
		Model = glm::translate(Model, glm::vec3(-1.5, 0.0, 0.0));
		Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));
		sendMVP();
		drawCube();
		*/



		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();





	}

	// Aufgabe 6
	glDeleteBuffers(1, &normalbuffer);

	// Aufgabe 7
	glDeleteBuffers(1, &uvbuffer);
	glDeleteTextures(1, &Texture);

	glDeleteBuffers(1, &vertexbuffer);

	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}



