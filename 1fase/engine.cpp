#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <math.h>
#include <iostream>
#include <fstream>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace tinyxml2;
using namespace std;


vector<float> xs,ys,zs;

// angle of rotation for the camera direction
float angle=0.0f;
float angle2 = 0.0f;
float raio= 5.0f;
// actual vector representing the camera's direction
float l=0.0f;
// XZ position of the camera
float x=0.0f,z=5.0f,y=0.0f;

/*
* Le as coordenadas do ficheiro passado e adiciona aos vetores das coordenadas.
*/ 
void lerCoord(string ofile) 
{
	string buffer;
	ifstream file;
	file.open(ofile);
	if(!file.is_open())
	{
		cerr << "Couldn't open file.";
		exit(EXIT_FAILURE);
	}

	while(getline(file,buffer))
	{
		float x,y,z;
		istringstream iss(buffer);
		
		iss >> x >> y >> z;
		xs.push_back(x);
		ys.push_back(y);
		zs.push_back(z);
	}
}


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void Regularkey(unsigned char key, int a, int b)
{
	switch (key) {		
		case 'j':
		{
			raio+=0.1f;
			break;
		}
		case 'k':
		{
			raio-=0.1f;
			break;
		}
	}

	glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy) {

// put code to process special keys in here

    float fraction = 0.1f;
    switch (key) {
    	case GLUT_KEY_LEFT :
    		angle -= 0.1f;
    		break;
    	case GLUT_KEY_RIGHT :
    		angle += 0.1f;
    		break;
    	case GLUT_KEY_UP :
    		angle2+=0.1f;
    		if(angle2>1.5){
    			angle2=1.5;
    		}
    		break;
    	case GLUT_KEY_DOWN :
    		angle2-=0.1f;
    		if(angle2<-1.5){
    			angle2=-1.5;
    		}
    		break;
	}
	glutPostRedisplay();
}

void renderScene(void) {
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();

	gluLookAt(raio*sin(angle)*cos(angle2),raio*sin(angle2),raio*cos(angle2)*cos(angle),
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	// DRAW
	int tam=xs.size();
	glBegin(GL_TRIANGLES);
		glColor3f(1, 1, 1);
		for(int i=0; i<tam; i++)
			glVertex3f(xs[i],ys[i],zs[i]);
	glEnd();

	glutSwapBuffers();
}

int main(int argc, char **argv) {
	string ficheiro;

	//XML parser
	const char * attributeText = nullptr;
	XMLDocument xmlDoc;
	xmlDoc.LoadFile(argv[1]);

	XMLNode *rootnode = xmlDoc.FirstChild();
	if (rootnode == nullptr)
	{
		cout << "No Root found.";
		return XML_ERROR_FILE_READ_ERROR;
	}
	XMLElement * rElement = rootnode->FirstChildElement("model");

	if (rElement == nullptr)
	{
		cout << "Element error.";
		return XML_ERROR_PARSING_ELEMENT;
	}

	while(rElement != nullptr)
	{
		attributeText = rElement->Attribute("file");
		if (attributeText == nullptr)
		{
			cout << "Atribute error.";
			return XML_ERROR_PARSING_ATTRIBUTE;
		}
		ficheiro = attributeText;
		rElement = rElement->NextSiblingElement(); // proximo "file". Estão à mesma altura na árvore, logo são "irmãos".

		lerCoord(ficheiro);
	}

	// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(600,500);
	glutCreateWindow("Assignment");

	// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutKeyboardFunc(Regularkey);
	glutSpecialFunc(processSpecialKeys);

	//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT,GL_LINE);
	
// enter GLUT's main cycle
	glutMainLoop();

	return 1;
}