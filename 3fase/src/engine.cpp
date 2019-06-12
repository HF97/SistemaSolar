#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
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
#include <GL/glew.h>
#include <math.h>

using namespace tinyxml2;
using namespace std;

GLuint buffers[1];

int timebase = 0;
int frame = 0;

vector<float> xs,ys,zs;

typedef struct translateT{
    float ttime;
    vector<float*> points;
} translateT;

vector<translateT> translationsTime;

typedef struct rotateT{
    float ttime;
    float x;
    float y;
    float z;
} rotateT;

vector<rotateT> rotateTime;


typedef struct modelPoints {
    // points {x,y,z, x,y,z ......}
    vector<float> points;
    float red;
    float green;
    float blue;
} modelPoints;

vector<modelPoints> models;

typedef struct drawInfo {
    // flag:
    //-1 glPushMatrix
    // 0 glPopMatrix 
    //
    // 1 translate
    // 10 translate time
    // 11 scale time
    // 2 rotate
    // 3 scale
    //
    // 5 draw model
    //
    int flag;
    float ang;
    float scale;
    float x;
    float y;
    float z;
} drawInfo;

vector<drawInfo> sceneInfo;



#define TOTAL_POINTS 100
int POINT_COUNT;


void buildRotMatrix(float *x, float *y, float *z, float *m) {

	m[0] = x[0]; m[1] = x[1]; m[2] = x[2] ; m[3] = 0 ;
	m[4] = y[0]; m[5] = y[1]; m[6] = y[2] ; m[7] = 0 ;
	m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
	m[12] = 0  ; m[13] = 0  ; m[14] = 0   ; m[15] = 1;
}


void cross(float *a, float *b, float *res) {

	res[0] = a[1]*b[2] - a[2]*b[1];
	res[1] = a[2]*b[0] - a[0]*b[2];
	res[2] = a[0]*b[1] - a[1]*b[0];
    res[3] = 0.0;
}


void normalize(float *a) {

	float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] = a[0]/l;
	a[1] = a[1]/l;
	a[2] = a[2]/l;
    a[3] = 1.0;
}


float length(float *v) {

	float res = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	return res;

}

void multMatrixVector(float *v, float *m, float *res) {

	for (int j = 0; j < 4; ++j) {
		res[j] = 0;
		for (int k = 0; k < 4; ++k) {
			res[j] += v[k] * m[k * 4 + j];
            /* printf("%d\n", k * 4 + j); */
		}
	}
    /* res [3] = 0; */

}

void multMatrix(float* m1, float* m2, float *res){
    for(int i = 0; i < 4; i++){
        multMatrixVector(m1 + i*4, m2, res + i*4);
    }
}


void getCatmullRomPoint(float t,
                        float *p0, float *p1, float *p2, float *p3,
                        float *pos, float *deriv) {

    float m[4][4] = {{-0.5f, 1.5f,  -1.5f, 0.5f},
        {1.0f,  -2.5f, 2.0f,    -0.5f},
        {-0.5f, 0.0f,    0.5f,  0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f}};


    float auxPos[4];
    float auxDeriv[4];

    auxPos[0] = (t*t*t)*m[0][0] + (t*t)*m[1][0] + t*m[2][0] + 1*m[3][0];
    auxPos[1] = (t*t*t)*m[0][1] + (t*t)*m[1][1] + t*m[2][1] + 1*m[3][1];
    auxPos[2] = (t*t*t)*m[0][2] + (t*t)*m[1][2] + t*m[2][2] + 1*m[3][2];
    auxPos[3] = (t*t*t)*m[0][3] + (t*t)*m[1][3] + t*m[2][3] + 1*m[3][3];

    auxDeriv[0] = 3*(t*t)*m[0][0] + 2*t*m[1][0] + m[2][0];
    auxDeriv[1] = 3*(t*t)*m[0][1] + 2*t*m[1][1] + m[2][1];
    auxDeriv[2] = 3*(t*t)*m[0][2] + 2*t*m[1][2] + m[2][2];
    auxDeriv[3] = 3*(t*t)*m[0][3] + 2*t*m[1][3] + m[2][3];

    pos[0] = auxPos[0]*p0[0] + auxPos[1]*p1[0] + auxPos[2]*p2[0] + auxPos[3]*p3[0];
    pos[1] = auxPos[0]*p0[1] + auxPos[1]*p1[1] + auxPos[2]*p2[1] + auxPos[3]*p3[1];
    pos[2] = auxPos[0]*p0[2] + auxPos[1]*p1[2] + auxPos[2]*p2[2] + auxPos[3]*p3[2];

    deriv[0] = auxDeriv[0]*p0[0] + auxDeriv[1]*p1[0] + auxDeriv[2]*p2[0] + auxDeriv[3]*p3[0];
    deriv[1] = auxDeriv[0]*p0[1] + auxDeriv[1]*p1[1] + auxDeriv[2]*p2[1] + auxDeriv[3]*p3[1];
    deriv[2] = auxDeriv[0]*p0[2] + auxDeriv[1]*p1[2] + auxDeriv[2]*p2[2] + auxDeriv[3]*p3[2];

}


void getGlobalCatmullRomPoint(vector<float*> p, float gt, float *pos, float *deriv) {

	float t = gt * POINT_COUNT; // verdadeiro t
	int index = floor(t);  //segmento
	t = t - index; // posicao no segmento

    // pontos a usar
	int indices[4];
	indices[0] = (index + POINT_COUNT-1)%POINT_COUNT;
	indices[1] = (indices[0]+1)%POINT_COUNT;
	indices[2] = (indices[1]+1)%POINT_COUNT;
	indices[3] = (indices[2]+1)%POINT_COUNT;

    // ponto num certo t
	getCatmullRomPoint(t, p[indices[0]], p[indices[1]], p[indices[2]], p[indices[3]], pos, deriv);
}

void renderCatmullRomCurve(vector<float*> points) {
    glColor3f(1.0f, 1.0f, 1.0f);
  	glBegin(GL_LINE_LOOP);
    float pos[3];
    float deriv[3];
    for(int i = 0; i < TOTAL_POINTS; i++){
        float t = ((float)i)/TOTAL_POINTS;
        getGlobalCatmullRomPoint(points, t, pos, deriv);
        glVertex3f(pos[0],pos[1],pos[2]);
    }
    glEnd();
}


void catmullMov(vector<float*> p, float ttime){

	int gt = glutGet(GLUT_ELAPSED_TIME);
    int timeM = (int)(1000*ttime);
    int timeint = gt % timeM;
    float t = ((float)timeint)/timeM; // t [0,1[

    float pos[3];
    float deriv[3];

    getGlobalCatmullRomPoint(p, t, pos, deriv);
    glTranslatef(pos[0], pos[1], pos[2]);
}

void setRotateT(rotateT rotateState){
    float ttime = rotateState.ttime;
    float x = rotateState.x;
    float y = rotateState.y;
    float z = rotateState.z;

    int gt = glutGet(GLUT_ELAPSED_TIME);
    int timeM = (int)(1000*ttime);
    int timeint = gt%timeM;
    timeM = (float)timeM;
    glRotatef(((float)timeint/(float)timeM)*360.0, x, y, z);
}


// angle of rotation for the camera direction
float angle=0.0f;
float angle2 = 0.0f;
float raio= 80.0f;
// actual vector representing the camera's direction
float l=0.0f;
// XZ position of the camera
float cx=0.0f,cz=5.0f,cy=0.0f;

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

void saveModel(char *modelo, XMLElement *ModelPtr) {

	string buffer;
	ifstream file;
	file.open(modelo);
	if(!file.is_open())
	{
		cerr << "Couldn't open file.";
		exit(EXIT_FAILURE);
	}

    modelPoints pontos;
	while(getline(file,buffer)){

		float x,y,z;
		istringstream iss(buffer);
		
		iss >> x >> y >> z;
        
		pontos.points.push_back(x);
		pontos.points.push_back(y);
		pontos.points.push_back(z);
	}
    

    float red   = 1;
    float green = 1;
    float blue  = 1;

    // Color defenition
    if(ModelPtr->Attribute("R")){
        red = stof(ModelPtr->Attribute("R"));
    }

    if(ModelPtr->Attribute("G")){
        green = stof(ModelPtr->Attribute("G"));
    }

    if(ModelPtr->Attribute("B")){
        blue = stof(ModelPtr->Attribute("B"));
    }

    pontos.red   = red;
    pontos.green = green;
    pontos.blue  = blue;

    models.push_back(pontos);

}

/*
 * XML Parse of a model
 */
void ParserObject(char *figura, XMLElement *ModelPtr) {
    drawInfo aux;
    aux.flag = 5;
    sceneInfo.push_back(aux);
    /* int s = strlen(figura); */
    /* figura[s-1] ='d'; */
    saveModel(figura, ModelPtr);
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
void parserRotateTime(XMLElement *ScalePtr, float ttime){
    rotateT rT;

    rT.ttime = ttime;
    rT.x = stof(ScalePtr->Attribute("axisX"));
    rT.y = stof(ScalePtr->Attribute("axisY"));
    rT.z = stof(ScalePtr->Attribute("axisZ"));

    drawInfo aux;
    aux.flag = 11;
    sceneInfo.push_back(aux);
    rotateTime.push_back(rT);

}

void parserTranslateTime(XMLElement *TransPtr, float ttime){
    TransPtr = TransPtr->FirstChildElement();
    float x, y, z;
    translateT transT;
    transT.ttime = ttime;

    if(!TransPtr) return;

    while(TransPtr){
        if(!strcmp(TransPtr->Value(), "point")){
            float x = stof(TransPtr->Attribute("X"));
            float y = stof(TransPtr->Attribute("Y"));
            float z = stof(TransPtr->Attribute("Z"));

            float *vetor;
            vetor = (float*)malloc(3*sizeof(float));
            vetor[0] = x;
            vetor[1] = y;
            vetor[2] = z;
            transT.points.push_back(vetor);
        }
        TransPtr = TransPtr->NextSiblingElement();
    }

    drawInfo aux;
    aux.flag = 10;
    sceneInfo.push_back(aux);
    translationsTime.push_back(transT);
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
        if(GroupPtr->Attribute("time")){
            parserTranslateTime(GroupPtr, stof(GroupPtr->Attribute("time")));
        }
        else{
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
    }

    if(!strcmp(GroupPtr->Value(),"scale")) {
        float x = stof(GroupPtr->Attribute("X"));
        float y = stof(GroupPtr->Attribute("Y"));
        float z = stof(GroupPtr->Attribute("Z"));

        drawInfo aux;
        aux.flag = 3;
        aux.x = x;
        aux.y = y;
        aux.z = z;
        sceneInfo.push_back(aux);
    }

    if(!strcmp(GroupPtr->Value(),"rotate")) {
        if(GroupPtr->Attribute("time")){
            parserRotateTime(GroupPtr, stof(GroupPtr->Attribute("time")));
        }
        else{
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
			raio+=2.0f;
			break;
		}
		case 'k':
		{
			raio-=2.0f;
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

void frames_second(){
    float fps;
    int time;
    char s[64];

    frame ++;
    time = glutGet(GLUT_ELAPSED_TIME);
    if(time - timebase > 1000) {
        fps = frame * 1000.0 / (time-timebase);
        timebase = time;
        frame = 0;
        sprintf(s, "FPS: %f6.2", fps);
        glutSetWindowTitle(s);
    }
}


void drawVBO(modelPoints model){
    float red = model.red;
    float green = model.green;
    float blue = model.blue;

    glColor3f(red, green, blue);

    float arr[model.points.size()];
    copy(model.points.begin(), model.points.end(), arr);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arr), arr, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(arr)/3);
    


}


/*
 * Render Scene from sceneInfo Vertex
 */
void newrenderScene(void) {
    frames_second();

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
    int modelPtr = 0;
    int transTPtr = 0;
    int rotateTPtr = 0;


    for(int i = 0; i < size; i++) {
        flag = sceneInfo[i].flag;
        ang = sceneInfo[i].ang;
        x = sceneInfo[i].x;
        y = sceneInfo[i].y;
        z = sceneInfo[i].z;
        switch(flag) {
            case -1:
                {
                glPushMatrix();
                break;
                }

            case 0:
                {
                glPopMatrix();
                break;
                }

            case 1:
                {
                glTranslatef(x,y,z);
                break;
                }

            case 2:
                {
                glRotatef(ang,x,y,z);
                break;
                }

            case 3:
                {
                glScalef(x,y,z);
                break;
                }


            case 5:
                {
                drawVBO(models[modelPtr]);
                modelPtr+=1;
                break;
                }

            case 10:
                {
                    int num = translationsTime[transTPtr].points.size();
                    if (num >= 4){
                        POINT_COUNT = num-1;
                        renderCatmullRomCurve(translationsTime[transTPtr].points);
                        catmullMov(translationsTime[transTPtr].points, translationsTime[transTPtr].ttime);
                        /* setCatmulMat(translationsTime[transTPtr].points, translationsTime[transTPtr].ttime); */
                        transTPtr += 1;
                    }
                    break;
                }

            case 11:
                {
                /* printf("RotateTime\n"); */
                setRotateT(rotateTime[rotateTPtr]);
                rotateTPtr += 1;
                /* float x = scaleTime[scaleTPtr].x; */
                /* float y = scaleTime[scaleTPtr].y; */
                /* float z = scaleTime[scaleTPtr].z; */

                /* int gt = glutGet(GLUT_ELAPSED_TIME); */
                /* float ttime = scaleTime[scaleTPtr].ttime; */
                /* int timeM = (int)(1000*ttime); */
                /* int timeint = gt % timeM; */

                /* /1* timeM = (float)timeM; *1/ */
                /* /1* timeint = (float)timeint; *1/ */
                /* float t = (timeint/timeM); */

                /* glRotatef(((float)timeint/(float)timeM)*360, x, y, z); */

                /* scaleTPtr += 1; */
                /* break; */
                }
        }

    }

    
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
    /* imprimevec(); */

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

#ifndef __APPLE__
    glewInit();
#endif

	//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT,GL_FILL);
	
    glGenBuffers(1,buffers);
	glEnableClientState(GL_VERTEX_ARRAY);
    // enter GLUT's main cycle
	glutMainLoop();

	return 0;
}
