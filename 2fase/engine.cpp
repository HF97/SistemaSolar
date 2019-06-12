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

typedef struct drawInfo {
    // flag:
    //-1 glPushMatrix
    // 0 glPopMatrix 
    //
    // 1 translate
    // 2 rotate
    // 3 scale
    //
    // 4 plane
    // 5 box
    // 6 sphere
    // 7 cone
    int flag;
    float ang;
    float scale;
    float x;
    float y;
    float z;
} drawInfo;

vector<drawInfo> sceneInfo;


void imprimevec(){
    drawInfo cenas;
    int maior = sceneInfo.size();
    for(int i = 0; i < maior; i++){
        cenas = sceneInfo[i];
        printf("Flag:%2d Ang:%f X:%f Y:%f Z:%f\n",
                cenas.flag, cenas.ang, cenas.x,
                cenas.y, cenas.z);
    }
}

// angle of rotation for the camera direction
float angle=0.0f;
float angle2 = 0.0f;
float raio= 30.0f;
// actual vector representing the camera's direction
float l=0.0f;
// XZ position of the camera
float x=0.0f,z=5.0f,y=0.0f;

void lerCoord(string ofile) 
{

    xs.clear();
    ys.clear();
    zs.clear();

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

/*
 * XML Parse of a model
 */
void ParserObject(char *figura, XMLElement *ModelPtr) {
    drawInfo aux;
    /* char *figura; */
    /* strcpy(figura,ModelPtr->Attribute("file")); */

    if (!strcmp(figura,"plane.3d")){

        aux.flag = 4;
        sceneInfo.push_back(aux);

    }

    if (!strcmp(figura,"box.3d")){

        aux.flag = 5;
        sceneInfo.push_back(aux);

    }

    if (!strcmp(figura,"sphere.3d")){

        aux.flag = 6;
        sceneInfo.push_back(aux);

    }

    if (!strcmp(figura,"cone.3d")){

        aux.flag = 7;
        sceneInfo.push_back(aux);

    }
}

/*
 * XML Parse inside Models
 */
void ParserModels(XMLElement *Model) {
    XMLElement *ModelPtr;
    char *figura;

    ModelPtr = Model->FirstChildElement();

    while(ModelPtr){
        if(!strcmp(ModelPtr->Value(),"model")){
            strcpy(figura,ModelPtr->Attribute("file"));
            ParserObject(figura, ModelPtr);
        }
        if(ModelPtr->NextSiblingElement())
            ModelPtr = ModelPtr->NextSiblingElement();
        else
            return;
        
    }

}

/*
 * XML Parse inside Root Group
 */
void ParserGroup(XMLElement *Group) {
    XMLElement *GroupPtr;

    GroupPtr = Group->FirstChildElement();

    while(GroupPtr){

    if(!strcmp(GroupPtr->Value(),"group")) {

        drawInfo aux;
        aux.flag = -1;
        sceneInfo.push_back(aux);

        ParserGroup(GroupPtr);

        aux.flag = 0;
        sceneInfo.push_back(aux);

    }

    if(!strcmp(GroupPtr->Value(),"models")) {

        drawInfo aux;
        aux.flag = -1;
        sceneInfo.push_back(aux);

        ParserModels(GroupPtr);

        aux.flag = 0;
        sceneInfo.push_back(aux);

    }

    if(!strcmp(GroupPtr->Value(),"translate")) {
        float x, y, z;
        x = stof(GroupPtr->Attribute("X"));
        y = stof(GroupPtr->Attribute("Y"));
        z = stof(GroupPtr->Attribute("Z"));

        drawInfo aux;
        aux.flag = 1;
        aux.x = x;
        aux.y = y;
        aux.z = z;
        sceneInfo.push_back(aux);
    }

    if(!strcmp(GroupPtr->Value(),"scale")) {
        x = stof(GroupPtr->Attribute("X"));
        y = stof(GroupPtr->Attribute("Y"));
        z = stof(GroupPtr->Attribute("Z"));

        drawInfo aux;
        aux.flag = 3;
        aux.x = x;
        aux.y = y;
        aux.z = z;
        sceneInfo.push_back(aux);
    }

    if(!strcmp(GroupPtr->Value(),"rotate")) {
        float ang;
        float x;
        float y;
        float z;
        ang = stof(GroupPtr->Attribute("angle"));
        x = stof(GroupPtr->Attribute("axisX"));
        y = stof(GroupPtr->Attribute("axisY"));
        y = stof(GroupPtr->Attribute("axisZ"));

        drawInfo aux;
        aux.flag = 2;
        aux.ang = ang;
        aux.x = x;
        aux.y = y;
        aux.z = z;
        sceneInfo.push_back(aux);
    }


    if(GroupPtr->NextSiblingElement())
       GroupPtr = GroupPtr->NextSiblingElement();
    else
        return;
    }
}

/*
 * XML Parse of the root Group
 */
void ParserRoot(XMLNode *RootNode) {
	XMLElement *Group = RootNode->FirstChildElement();

    if(!Group)
        return;

    while(!strcmp(Group->Value(),"group")){

        drawInfo aux;
        aux.flag = -1;
        sceneInfo.push_back(aux);

        ParserGroup(Group);

        aux.flag = 0;
        sceneInfo.push_back(aux);

        if(Group->NextSiblingElement())
	        Group = Group->NextSiblingElement();
        else
            return;
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
			raio+=1.0f;
			break;
		}
		case 'k':
		{
			raio-=1.0f;
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

/*
 * Render Scene from sceneInfo Vertex
 */
void newrenderScene(void) {
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();

	/* gluLookAt(0.0, 0.0, 30.0, */
	/* 	0.0, 0.0, 0.0, */
	/* 	0.0f, 1.0f, 0.0f); */
	gluLookAt(raio*sin(angle)*cos(angle2),raio*sin(angle2),raio*cos(angle2)*cos(angle),
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	// DRAW
    int size = sceneInfo.size();
    int tam;
    int flag, x, y, z;
    float ang;

    for(int i = 0; i < size; i++) {
        flag = sceneInfo[i].flag;
        ang = sceneInfo[i].ang;
        x = sceneInfo[i].x;
        y = sceneInfo[i].y;
        z = sceneInfo[i].z;
        switch(flag) {
            case -1:
                glPushMatrix();
                printf("Push\n");
                break;

            case 0:
                glPopMatrix();
                printf("Pop\n");
                break;

            case 1:
                glTranslatef(x,y,z);
                printf("translate\n");
                break;

            case 2:
                glRotatef(ang,x,y,z);
                printf("rotate\n");
                break;

            case 3:
                glScalef(x,y,z);
                printf("scale\n");
                break;

            case 4:
                lerCoord("plane.3d");
                tam = xs.size();
	            glBegin(GL_TRIANGLES);
	            	glColor3f(1, 1, 1);
	            	for(int i=0; i<tam; i++)
	            		glVertex3f(xs[i],ys[i],zs[i]);
	            glEnd();
                printf("plane\n");
                break;

            case 5:
                lerCoord("box.3d");
                tam = xs.size();
	            glBegin(GL_TRIANGLES);
	            	glColor3f(1, 1, 1);
	            	for(int i=0; i<tam; i++)
	            		glVertex3f(xs[i],ys[i],zs[i]);
	            glEnd();
                printf("box\n");
                break;

            case 6:
                lerCoord("sphere.3d");
                tam = xs.size();
	            glBegin(GL_TRIANGLES);
	            	glColor3f(1, 1, 1);
	            	for(int i=0; i<tam; i++)
	            		glVertex3f(xs[i],ys[i],zs[i]);
	            glEnd();
                printf("sphere\n");
                break;

            case 7:
                lerCoord("cone.3d");
                tam = xs.size();
	            glBegin(GL_TRIANGLES);
	            	glColor3f(1, 1, 1);
	            	for(int i=0; i<tam; i++)
	            		glVertex3f(xs[i],ys[i],zs[i]);
	            glEnd();
                printf("cone\n");
                break;
        }

    }

    
    printf("SWAP\n");
    glutSwapBuffers();
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
	/* const char * attributeText = nullptr; */
	XMLDocument xmlDoc;
	xmlDoc.LoadFile(argv[1]);

	XMLNode *rootnode;
    rootnode = xmlDoc.FirstChild();
	if (rootnode == nullptr)
	{
		cout << "No Root found.";
		return (XML_ERROR_FILE_READ_ERROR);
	}

	XMLElement *ElemGroup = rootnode->FirstChildElement("group");

    if (!ElemGroup) {
        cout << "Group error.";
        return (XML_ERROR_PARSING_ELEMENT);
    }

    ParserRoot(rootnode);
    imprimevec();

    /* int maior = v.size(); */
    /* for(int i = 0; i<maior; i++){ */
    /*     i aux = v[i]; */
    /*     printf("%d: %d %d\n", i, aux.x, aux.y); */
    /* while(ElemGroup) { */
    /*     GroupParser(ElemGroup); */
    /*     /1* parseGroup(ElemGroup); *1/ */
    /*     ElemGroup = ElemGroup->NextSiblingElement(); */
    /* } */

	/* if (rElement == nullptr) */
	/* { */
	/* 	cout << "Element error."; */
	/* 	return XML_ERROR_PARSING_ELEMENT; */
	/* } */

	/* while(rElement != nullptr) */
	/* { */
	/* 	attributeText = rElement->Attribute("file"); */
        /* // attributeText = sphere.3d */
        /* printf("%s",attributeText); */
	/* 	if (attributeText == nullptr) */
	/* 	{ */
	/* 		cout << "Atribute error."; */
	/* 		return XML_ERROR_PARSING_ATTRIBUTE; */
	/* 	} */
	/* 	ficheiro = attributeText; */
	/* 	rElement = rElement->NextSiblingElement(); // proximo "file". Estão à mesma altura na árvore, logo são "irmãos". */

		/* lerCoord(ficheiro); */
	/* } */


	// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(600,500);
	glutCreateWindow("Assignment");

	// Required callback registry 
	/* glutDisplayFunc(renderScene); */
	glutDisplayFunc(newrenderScene);
	glutReshapeFunc(changeSize);

    glutIdleFunc(newrenderScene);

	glutKeyboardFunc(Regularkey);
	glutSpecialFunc(processSpecialKeys);

	//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT,GL_FILL);
	
    // enter GLUT's main cycle
	glutMainLoop();

	return 0;
}
