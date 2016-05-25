// Dateien zur Abgabe
//	StandardShading.fragmentshader
//	StandardShading.vertexshader
//	teapot.obj
//	Debug/CGTutorial.exe
//	mandrill.bmp
//	eine eigene Dokumentation zur Bedienung

// TO: sftp://uranus.f4.htw-berlin.de/home/tj/public_html/cg/belege/




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

// Achtung, die OpenGL-Tutorials nutzen glfw 2.7, glfw kommt mit einem veränderten API schon in der Version 3 

// Befindet sich bei den OpenGL-Tutorials unter "common"
#include "shader.hpp"

// Wuerfel und Kugel
#include "objects.hpp"

#include "objloader.hpp"

// Aufgabe 7
#include "texture.hpp"


// Erklären ObenGL-Statemachine, lowlevel
// Version 1: seit 1992, elegantes API für die Plattformunabhägige 3D-Programmierung 
// Version 2: seit 2002, Ergänzung durch Shader-Programme, die auf Grafikkarte laufen
// Version 3: seit 2008, Fixedfunction-Pipeline nur noch als Compability-Extension
// Version 4: seit 2010 noch kaum unterstützt
// Wir setzen nun auf Version 3, schwieriger zu erlernen als Version 1
// glm-Bibliothek für Matrix-Operationen (fehlen in Version3), glew-Bibliothek, um API-Funktionen > 1.1 nutzen zu können
// glfw-Bibliothek, um OpenGL plattformunabhängig nutzen zu können (z. B. Fenster öffnen)
#define UEBUNG1 /* feste Rotation */
// Pflichtteil: Ergebnis zeigen ...
// Kode zeilenweise erklären. (Orientiert sich an http://www.opengl-tutorial.org/, Evtl. diese Tutorials dort ausführen, um zu vertiefen.)
// Dort wird glfw in der Version 2 genutzt, wir verwenden die aktuelle Version 3 http://www.glfw.org/
// Schwarze Linien wg. Shader. Kurz erklaeren, was die Shader machen...
// Vorgehensweise erklaeren, Jeweils alte cpp-Datei nach CGTutorial.cpp.n kopieren, damit jede Aenderung nachvollziehbar bleibt.
// (Voraussetzung fuer Support)
// Aufgabe Rotation: glm::rotate    (http://glm.g-truc.net/glm.pdf)
#define UEBUNG2 /* variable Rotation ueber R-Taste */
// Eventloop, kann man muss man aber nicht in glfw (Alternativ Glut, dort muss man)
// Aufgabe: Hinweis globale Variable, Taste...
#define UEBUNG3 /* Kamerasteuerung */
// Aufgabe drei Unabhängige Rotationen, wird zu erstem Teil des Pflichtteils
// Hinweis auf andere Kaperasteuerungen ->  http://www.opengl-tutorial.org
#define UEBUNG4 /* Ausgemaltes Objekt und Z-Buffer */
// OpenGL-Befehle: http://wiki.delphigl.com/index.php/Hauptseite auf Deutsch!
// GLEW http://glew.sourceforge.net/
// Wireframe vs. Solid thematisieren, Z-Buffer wird noetig, um Verdeckungsproblem zu lösen
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-4-a-colored-cube/
///////////////////////////////////////////////////////
#define UEBUNG5 /* Einlesen eines Objekts */
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// Bessere OBJ-Loader vorhanden, basieren aber auf OpenGL1.1
// Andere Formate möglich, z. B. 3ds, s. a. Link am Ende der Seite
// Wir nehmen Teapot, Anekdote zu Teapot:  von Newell 1975 (hat berühmtes Lehrbuch geschrieben) glut, 3dsmax, Toystory, Monster AG, ...
//
// Vertex Buffer Objects (VBO) (ab 1.5) enthalten alle Daten zu Oberflaechen eines Objekts (Eckpunktkoordinaten, Normalen, Texturkoordinaten -> Uebung 6)
// koennen in separatem Speicher einer Grafikkarte gehalten werden (haengt vom Modell ab)
// koennen ueber API-Aufrufe veraendert werden --> glBufferData, glMapBuffer (GL_STATIC_DRAW, um mitzuteilen, dass sich nichts aendern wird)
// Vertex Array Objects (VAO) (ab 3) kapseln mehrere VBOs eines Objects zwecks Optimierung und einfachrer Benutzung:
// Beim Zeichnen nur 2 Aufrufe: glBindVertexArray und glDrawArrays
//#define UEBUNG5TEDDY  /* Nach der Teddy-Übung fuer Uebung6 wieder ausschalten */
// Alternativ: Teddy wenn die Zeit reicht: google nach teddy chameleon, teapot.obj mit xxx.obj austauschen....
// Modellieren kann also auch einfach sein, Freies Tool Blender (open Source), Professionelle Werkzeuge Maya, 3dsmax, etc. (nicht gegenstand der LV)
#define UEBUNG6 /* Beleuchten, neuer Shader */
// Teddy-Modell hat keine Normalen, passt nicht zu Shadern, wieder zur Teekanne zurueck.
// Shader anschauen, Alter Shader "ColourFragmentShader" setzt Farbe direkt, wird interpoliert ("Gouraud-Shading")
// "TransformVertexShader" gibt Farben weiter, legt layout der Eingabe-Daten fest, verwendet MVP um Eckpunkte zu transvormieren ("Eine Matrix reicht")
// Neue Shader koennen mehr (Immer noch nicht, was die Fixed-Function-Pipeline konnte) 
// Normalen stehen senkrecht auf Oberflaechen, wichtig fuer die Beleuchtung
// Normalen muessen anders behandelt werden als Eckpunkte, deshalb Trennung von MVP in Model View Projection
// -> Shader verändern in Uebung 15
#define UEBUNG7 /* Texturieren */
// Farbtexturen sind digitale Rasterbilder (andere Texturen in Kombination moeglich "Multi-Texturing" nicht Gegenstand der Uebung -> VL Textur)
// Imageloader fuer png und jpg nicht in den Bibliotheken enthalten -> SOIL
// DDS-Format kompatibel zu Texturkompression der Hardware. Wir nehmen aber BMP !
// s. a.:  http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/, Achtung glfwLoadTexture2D gibt' nicht mehr in glfw-version 3
// Granularitaet: Textur pro Objekt. kann also gewechselt werden ! --> Uebung 8
// Texturierte Teekanne ist 2. Teil des Pflichtteils 
///////////////////////////////////////////////////////
#define UEBUNG8 /* Mehrere Objekte */
// Polygone bilden Objekt, Objekte haben eigene Transformationen ("Model"-Matrix) eigene Texturen, etc.
// VAOs abwechselnd binden, MVP-Matrizen abwechselnd setzen, ggf Texturen abwechselnd binden, OpenGL verwendet jeweils positive Zahlen als
// Namen, die man sich generieren lassen muss.
// Model-Matrix fuer jedes Objekt anders, Szenen haben meist hierarchische Struktur, Multiplikationskette von der Wurzel zum Blatt
// Roboter-Beispiel, um Ketten zu lernen
#define UEBUNG9 /* Koordinatenachse */
// Notwendigkeit Koordinatensysteme anzeigen zu lassen... -> drawCS-Funktion mit drei Balkenen
// Wie erhaelt man langen duennen Balken mit bekannten Geometrien ?
// Aufgabe: Balken (leider mit M, V, P, ... als Parameter) Hinweis ginge auch als Singleton...
// Skalierungsparameter ruhig ausprobieren.
// sieht man den Balken im Bauch der Teekanne ?
#define UEBUNG10 /* Koordinatenkreuz */
// Aufgabe: Drei Balken, spaeter entsteht die Notwendigkeit Matrizen zu sichern (evtl. Mechanismus von OpenGL1 erwaehnen)
#define UEBUNG11 /* Ein Robotersegment */
// Teekanne ausblenden, Kugel zeichnen, Transformationsreihenfolge nochmal thematisieren
// Aufgabe: -Funktion mit Parameter!
#define UEBUNG12 /* Drei Robotersegmente uebereinander */
// Aufgabe: statischer Roboter, Unterschied lokales vs. globales Translate
#define UEBUNG13 /* Rotationen in den Gelenken */
// Aufgabe Roboter mit Tastensteuerung, Reihenfolge der Transformationen von oben nach unten und umgekehrt mit Stiften erläutern
#define UEBUNG14 /* Lampe am Ende */
// Uebung 15 im StandardShading pixelshader
// Hier mal exemplarisch den Pixelshader aendern, um die Beleuchtung ansprechender zu machen
// Erwaehnen, dass man Parameter wie "MVP" kontrollieren koennte.
// Hier beginnt die Wellt der Shader-programmierung, die nicht Gegenstand der Uebung ist.
// Lampe am Ende eines steuerbaren Roboters ist dritter Teil des Pflichtteils
// (5 Punkte: 3 Rotationen, Teekanne, texturiert, Roboter, Licht am Ende) 


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
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		
		// Camera matrix
		View = glm::lookAt(glm::vec3(0,0,-5), // Camera is at (0,0,-5), in World Space
						   glm::vec3(0,0,0),  // and looks at the origin
						   glm::vec3(0,1,0)); // Head is up (set to 0,-1,0 to look upside-down)
		
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

		drawCs(-0.6,0.01,0.01);


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

