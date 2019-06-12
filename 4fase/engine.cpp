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
#include <IL/il.h>
#include <IL/ilu.h>

using namespace tinyxml2;
using namespace std;

GLuint vertices, normals, texCoord, *texID;

int timebase = 0;
int frame = 0;

vector<float> xs,ys,zs;


typedef struct lightScene{
    // 1 -> POINT
    // 2 -> DIRECTIONAL
    // 3 -> SPOT
    int type;
    float px;
    float py;
    float pz;
}lightScene;

vector<lightScene> sceneLights;

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
    vector<float> normals;
    vector<float> texture;
    float red;
    float green;
    float blue;
    int Fdiff;
    float diff[4];
    int Fspec;
    float spec[4];
    int Femis;
    float emis[4];
    int Famb;
    float amb[4];
    char *textureName;
    int Ftexture;
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
float raio= 50.0f;
// actual vector representing the camera's direction
float l=0.0f;
// XZ position of the camera
float cx=0.0f,cz=5.0f,cy=0.0f;

// void lerCoord(string ofile) 
// {

//     xs.clear();
//     ys.clear();
//     zs.clear();

// 	string buffer;
// 	ifstream file;
// 	file.open(ofile);
// 	if(!file.is_open())
// 	{
// 		cerr << "Couldn't open file.";
// 		exit(EXIT_FAILURE);
// 	}

    

// 	while(getline(file,buffer))
// 	{
// 		float x,y,z;
// 		istringstream iss(buffer);
		
// 		iss >> x >> y >> z;
// 		xs.push_back(x);
// 		ys.push_back(y);
// 		zs.push_back(z);
// 	}
// }

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

        getline(file,buffer);

        iss >> x >> y >> z;
        pontos.normals.push_back(x);
        pontos.normals.push_back(y);
        pontos.normals.push_back(z);

        getline(file,buffer);

        iss >> x >> y >> z;
        pontos.texture.push_back(x);
        pontos.texture.push_back(y);
        pontos.texture.push_back(z);
	}
    

    char *texture;

    float red = 1, green = 1, blue = 1;

    float diffR = 0, diffG = 0, diffB = 0;
    float emisR = 0, emisG = 0, emisB = 0;
    float specR = 0, specG = 0, specB = 0;
    float ambR = 0, ambG = 0, ambB = 0;

    pontos.Fdiff = 0; pontos.Femis = 0; pontos.Fspec = 0; pontos.Famb = 0;
    pontos.Ftexture = 0;


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

    if(ModelPtr->Attribute("diffR")){
        diffR = stof(ModelPtr->Attribute("diffR"));
        pontos.Fdiff = 1;
    }

    if(ModelPtr->Attribute("diffG")){
        diffG = stof(ModelPtr->Attribute("diffG"));
    }

    if(ModelPtr->Attribute("diffB")){
        diffB = stof(ModelPtr->Attribute("diffB"));
    }

    if(ModelPtr->Attribute("ambR")){
        diffR = stof(ModelPtr->Attribute("ambR"));
        pontos.Famb = 1;
    }

    if(ModelPtr->Attribute("ambG")){
        ambG = stof(ModelPtr->Attribute("ambG"));
    }

    if(ModelPtr->Attribute("ambB")){
        ambB = stof(ModelPtr->Attribute("ambB"));
    }

    if(ModelPtr->Attribute("specR")){
        specR = stof(ModelPtr->Attribute("specR"));
        pontos.Fspec = 1;
    }

    if(ModelPtr->Attribute("specG")){
        specG = stof(ModelPtr->Attribute("specG"));
    }

    if(ModelPtr->Attribute("specB")){
        specB = stof(ModelPtr->Attribute("specB"));
    }


    if(ModelPtr->Attribute("emisR")){
        emisR = stof(ModelPtr->Attribute("emisR"));
        pontos.Femis = 1;
    }

    if(ModelPtr->Attribute("emisG")){
        emisG = stof(ModelPtr->Attribute("emisG"));
    }

    if(ModelPtr->Attribute("emisB")){
        emisB = stof(ModelPtr->Attribute("emisB"));
    }

    if(ModelPtr->Attribute("texture")){
        texture = (char *)malloc(sizeof(char) * strlen(ModelPtr->Attribute("texture")));
        strcpy(texture, ModelPtr->Attribute("texture"));

        pontos.Ftexture = 1;
        pontos.textureName = (char *)malloc(sizeof(char) *strlen(texture));
        strcpy(pontos.textureName, texture);
        // cout << texture << endl;
    }

    pontos.red   = red;
    pontos.green = green;
    pontos.blue  = blue;

    pontos.diff[0] = diffR; pontos.diff[1] = diffG; pontos.diff[2] = diffB;
    pontos.diff[3] = pontos.Fdiff;

    pontos.amb[0] = ambR; pontos.amb[1] = ambG; pontos.amb[2] = ambB;
    pontos.amb[3] = pontos.Famb;

    pontos.spec[0] = specR; pontos.spec[1] = specG; pontos.spec[2] = specB;
    pontos.spec[3] = pontos.Fspec;
    // cout << specR << " " << specG << " " << specB << endl;

    pontos.emis[0] = emisR; pontos.emis[1] = emisG; pontos.emis[2] = emisB;
    pontos.emis[3] = pontos.Femis;


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


void ParserLights(XMLElement *Light){
    XMLElement *LightPtr;
    LightPtr = Light->FirstChildElement();

    while(LightPtr){
        if(!strcmp(LightPtr->Value(), "light") && LightPtr->Attribute("type")){
            int type = 1;

            if(!strcmp(LightPtr->Attribute("type"), "POINT"))
                type = 1;
            else
                if(!strcmp(LightPtr->Attribute("type"), "DIRECTIONAL"))
                    type = 2;
                // SPOT
                else
                    type = 3;


            float x = stof(LightPtr->Attribute("posX"));
            float y = stof(LightPtr->Attribute("posY"));
            float z = stof(LightPtr->Attribute("posZ"));
            // printf("%d: %f, %f, %f\n", type, x, y, z);

            lightScene light;
            light.type = type;
            light.px = x;
            light.py = y;
            light.pz = z;

            sceneLights.push_back(light);
            LightPtr = LightPtr->NextSiblingElement();
        }
    }

}

/*
 * XML Parse of the root Group
 */
void ParserRoot(XMLNode *RootNode) {
	XMLElement *Group = RootNode->FirstChildElement();

    if(!Group)
        return;

    if(!strcmp(Group->Value(), "lights")){
        ParserLights(Group);
        if(Group->NextSiblingElement())
            Group = Group->NextSiblingElement();
        else
            return;
    }

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

int loadTexture(std::string s){
    
	unsigned int t,tw,th;
	unsigned char *texData;
	unsigned int texID;

	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilGenImages(1,&t);
	ilBindImage(t);
	ilLoadImage((ILstring)s.c_str());
	tw = ilGetInteger(IL_IMAGE_WIDTH);
	th = ilGetInteger(IL_IMAGE_HEIGHT);
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	texData = ilGetData();

	glGenTextures(1,&texID);
	
	glBindTexture(GL_TEXTURE_2D,texID);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,		GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,		GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAG_FILTER,   	GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

void drawVBO(modelPoints model, int modelNum){
    if(sceneLights.size() == 0){
        float red = model.red;
        float green = model.green;
        float blue = model.blue;
        glColor3f(red, green, blue);
    }

    float arr[model.points.size()];
    copy(model.points.begin(), model.points.end(), arr);

    float arrn[model.normals.size()];
    copy(model.normals.begin(), model.normals.end(), arrn);

    float arrt[model.texture.size()];
    copy(model.texture.begin(), model.texture.end(), arrt);

    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arr), arr, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrn), arrn, GL_STATIC_DRAW);
    glNormalPointer(GL_FLOAT, 0, 0);


    if(model.Ftexture){
        glEnable(GL_TEXTURE_2D);
        glBindBuffer(GL_ARRAY_BUFFER, texCoord);
        glBufferData(GL_ARRAY_BUFFER, sizeof(arrt), arrt, GL_STATIC_DRAW);
        glBindTexture(GL_TEXTURE_2D, texID[modelNum]);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        // cout << "Textura: " << model.Ftexture  << endl;
    }
    // cout << "Textura: " << model.Ftexture  << endl;
    // else{
    //     // limpar textura da stack
    //     glBindTexture(GL_TEXTURE_2D, 0);
    // }


    // cout << model.diff[0] << " " << model.diff[1] << " " << model.diff[2] << " " << endl;
    // cout << model.Fdiff << " " << model.Famb << " " << model.Fspec << " " << model.Femis << endl;

    // cout << "MODELO:" << endl;

    for(int i = 0; i < sceneLights.size(); i++){
    // if(model.Fdiff){
        // cout << "Diffuse: " << model.diff[0] << " " << model.diff[1] << " " << model.diff[2] << " " <<model.diff[3] << endl;
	    glMaterialfv(GL_FRONT,GL_DIFFUSE, model.diff);
        glLightfv(GL_LIGHT0+i, GL_DIFFUSE, model.diff);
    // }
    // if(model.Famb){
        // cout << "Ambient: " << model.amb[0] << " " << model.amb[1] << " " << model.amb[2] << " " << model.amb[3] << endl;
        glMaterialfv(GL_FRONT, GL_AMBIENT, model.amb);
        glLightfv(GL_LIGHT0+i, GL_AMBIENT, model.amb);
    // }

    // if(model.Femis){
        // cout << "Emission: " << model.emis[0] << " " << model.emis[1] << " " << model.emis[2] << " " << model.emis[3] << endl;
        glMaterialfv(GL_FRONT, GL_EMISSION, model.emis);
        glLightfv(GL_LIGHT0+i, GL_EMISSION, model.emis);
    // }

    // if(model.Fspec){
        // cout << "Spec: " << model.spec[0] << " " << model.spec[1] << " " << model.spec[2] << " " << model.spec[3] << endl;
        glMaterialfv(GL_FRONT, GL_SPECULAR, model.spec);
        glLightfv(GL_LIGHT0+i, GL_SPECULAR, model.spec);
    // }

    }

    glDrawArrays(GL_TRIANGLES, 0, sizeof(arr)/3);
    // glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void setLights(){
    if(sceneLights.size() > 0)
        glEnable(GL_LIGHTING);
    // enable all lights
    for(int i = 0; i < sceneLights.size(); i++){
        glEnable(GL_LIGHT0+i);
        float pos[4];
        pos[0] = sceneLights[i].px;
        pos[1] = sceneLights[i].py;
        pos[2] = sceneLights[i].pz;
        pos[3] = 1.0;
        
        // cout <<"Light Possition:" << " "<< pos[0] << " " << pos[1] << " " << pos[2] << " " << pos[3] << endl;
	    glLightfv(GL_LIGHT0+i, GL_POSITION, pos);
    }
}

// void loadImages(){
//     texCoord = (GLuint *)realloc(texCoord, models.size());
//     glGenBuffers(models.size(),texCoord);
//     for(int i = 0; i < models.size(); i++){
//         if(models[i].Ftexture == 0)
//             continue;

//     }
// }

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

    setLights();

	// DRAW
    int size = sceneInfo.size();
    int tam;
    int flag, x, y, z;
    float ang;
    int modelPtr = 0;
    int transTPtr = 0;
    int rotateTPtr = 0;

    // loadImages();


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
                drawVBO(models[modelPtr], modelPtr);
                modelPtr+=1;
                break;
                }

            case 10:
                {
                    int num = translationsTime[transTPtr].points.size();
                    if (num >= 4){
                        POINT_COUNT = num-1;
                        // renderCatmullRomCurve(translationsTime[transTPtr].points);
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

void saveTextures(){
    texID = (GLuint *)malloc(sizeof(GLuint) * models.size());
    for(int i = 0; i < models.size(); i++){
        if(models[i].Ftexture){
            texID[i] = loadTexture(models[i].textureName);
            // cout << models[i].textureName << endl;
        }
    }
}


int main(int argc, char **argv) {
	string ficheiro;

	//XML parser
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
	
    // glClearColor(0.119, 0, 0.34, 0);
    glClearColor(0.0385, 0, 0.11, 0);
    glGenBuffers(1,&vertices);
    glGenBuffers(1,&normals);
    glGenBuffers(1,&texCoord);
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    saveTextures();

    // enter GLUT's main cycle
	glutMainLoop();

	return 0;
}
