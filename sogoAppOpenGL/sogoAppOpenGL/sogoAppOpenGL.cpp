// sogoAppOpenGL.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

//#include "stdafx.h"

// Include standard headers
#include <iostream>
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

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &Projection[0][0]);
}
/*
	Dient zur Darstellung eines 3D Modells. 
*/
struct Mesh
{
	/*
		filename	Übergabe der pbj Datei
	*/
	Mesh(std::string fileName)
	{
		// --- Kannenmodell
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		// Laden der Objektdatei
		bool res = loadOBJ(fileName.c_str(), vertices, uvs, normals);

		// Jedes Objekt eigenem VAO zuordnen, damit mehrere Objekte moeglich sind
		// VAOs sind Container fuer mehrere Buffer, die zusammen gesetzt werden sollen.
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// Ein ArrayBuffer speichert Daten zu Eckpunkten (hier xyz bzw. Position)
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


					   // Aufgabe 6 // Hier alles analog für Normalen in location == 2
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2); // siehe layout im vertex shader 
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Aufgabe 7 // Hier alles analog für Texturkoordinaten in location == 1 (2 floats u und v!)
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1); // siehe layout im vertex shader 
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		vertexCount = vertices.size();
	}

	virtual ~Mesh()
	{
		glDeleteBuffers(1, &vertexbuffer);

		glDeleteBuffers(1, &normalbuffer);

		glDeleteBuffers(1, &uvbuffer);
	}

	GLuint VertexArrayID;

	GLuint vertexbuffer;
	GLuint normalbuffer;
	GLuint uvbuffer;

	int vertexCount;

};

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
	//programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Shader auch benutzen !
	glUseProgram(programID);
	/*
	// =========================================================================
	//
	//	Kannenmodell
	//
	// =========================================================================

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

	
	// --- Teil Kannenmodell
	*/

	Mesh Sphere("Sphere.obj");
	Mesh Cube("Cube.obj");
	Mesh Kanne("teapot.obj");
	

	// Load the texture
	GLuint TextureAffe = loadBMP_custom("mandrill.bmp");
	GLuint TextureLoewe = loadBMP_custom("Loewe.jpg"); // kein bmp somit nicht funktionable

	// =========================================================================
	//
	//	Eventloop
	//
	// =========================================================================
	
	//KugelRadius
	float kugelRad = 1.0f;

	int koord1 = 3;


	while (!glfwWindowShouldClose(window))
	{
		


		// Clear the screen
		//glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

		

		// =========================================================================
		//
		//	Cam View
		//
		// =========================================================================
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

		int x, y, z;

		// World space offset
		// Verschiebungsweite
		float lookAtPoint = (koord1 * kugelRad * 2) / 2;

		glm::mat4 offsetSaves = glm::translate(Model, glm::vec3(0, kugelRad, 0));

		for (x = 0; x <= koord1; x++)
		{
			for (z = 0; z <= koord1; z++)
			{
				Model = Save;
				
				Model = glm::translate(Model, glm::vec3(x * (kugelRad * 2) - lookAtPoint, -lookAtPoint, z * (kugelRad * 2) - lookAtPoint));
				Model = glm::scale(Model, glm::vec3(kugelRad * 2, kugelRad /4, kugelRad * 2));
				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0);				// Die Textturen sind durchnummeriert
				glBindTexture(GL_TEXTURE_2D, TextureAffe);		// Verbindet die Textur
																// Set our "myTextureSampler" sampler to user Texture Unit 0
				glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

				sendMVP();
				glBindVertexArray(Cube.VertexArrayID);
				glDrawArrays(GL_TRIANGLES, 0, Cube.vertexCount);
			}
		}

		for (y = 0; y <= koord1; y++)
		{			
			for (x = 0; x <= koord1; x++)
			{				
				for (z = 0; z <= koord1; z++)
				{
					Model = offsetSaves;
					// 
					Model = glm::translate(Model, glm::vec3(x * (kugelRad * 2) - lookAtPoint, y * (kugelRad * 2) - lookAtPoint, z * (kugelRad * 2) - lookAtPoint));

					if (x == 1 && z == 3 && y == 0 )
					{
												
						
						Model = glm::scale(Model, glm::vec3(kugelRad*2, kugelRad*2, kugelRad*2));
						//Model = glm::scale(Model, glm::vec3(1.0 / 1000.0, 1.0 / 1000.0, 1.0 / 1000.0));
						

						// Bind our texture in Texture Unit 0
						glActiveTexture(GL_TEXTURE0);				// Die Textturen sind durchnummeriert
						glBindTexture(GL_TEXTURE_2D, TextureAffe);		// Verbindet die Textur
																	// Set our "myTextureSampler" sampler to user Texture Unit 0
						glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

						sendMVP();
						glBindVertexArray(Sphere.VertexArrayID);
						glDrawArrays(GL_TRIANGLES, 0, Sphere.vertexCount);

						
					}
					else
					{
						Model = glm::scale(Model, glm::vec3(kugelRad / 5, kugelRad * 2, kugelRad / 5));
						// Bind our texture in Texture Unit 0
						glActiveTexture(GL_TEXTURE0);				// Die Textturen sind durchnummeriert
						glBindTexture(GL_TEXTURE_2D, TextureAffe);		// Verbindet die Textur
																		// Set our "myTextureSampler" sampler to user Texture Unit 0
						glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

						sendMVP();
						glBindVertexArray(Cube.VertexArrayID);
						glDrawArrays(GL_TRIANGLES, 0, Cube.vertexCount);
					}
				}

			}
		}

		Model = Save;
		
		// Lichtposition an der Spitze des letzten Segments
		glm::vec4 lightPos = glm::vec4(8, 3, 0, 1);
		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);

		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	//glDeleteBuffers(1, &vertexbuffer);

	// Aufgabe 6
	//glDeleteBuffers(1, &normalbuffer);

	// Aufgabe 7
	//glDeleteBuffers(1, &uvbuffer);
	glDeleteTextures(1, &TextureAffe);
	glDeleteTextures(1, &TextureLoewe);	

	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}



