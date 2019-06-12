#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif 


using namespace std;

void getPoints(int *patches, float vertices[][3], float *ret){
    int i, k;
    for(i = 0; i < 16; i++){
        for(k = 0; k < 3; k++){
            ret[k + i * 4] = vertices[patches[i]][k];
        }
        ret[k + i*4] = 1;
    }
}


void bezAux(float t, float *p0, float *p1, float *p2, float *p3, float *res){
    res[0] = (1-t)*(1-t)*(1-t)*p0[0] + (3*t*(1-t)*(1-t))*p1[0] + (3*t*t*(1-t))*p2[0] + (t*t*t)*p3[0];
    res[0] = (1-t)*(1-t)*(1-t)*p0[0] + (3*t*(1-t)*(1-t))*p1[0] + (3*t*t*(1-t))*p2[0] + (t*t*t)*p3[0];
    res[0] = (1-t)*(1-t)*(1-t)*p0[0] + (3*t*(1-t)*(1-t))*p1[0] + (3*t*t*(1-t))*p2[0] + (t*t*t)*p3[0];
}

void bezP(float a, float b, float p[][3], float *res){

    int j = 0;
    float auxP[4][3];
    for(int i = 0; i < 16; i+=4){
        bezAux(a, p[i], p[i+1], p[i+2], p[i+3], auxP[j]);
        j+=1;
    }

    bezAux(b, auxP[0], auxP[1], auxP[2], auxP[3], res);
    
}


void bezierCalc(float points[][3], int tessellation, ofstream &outf){
    float frac = 1.0/tessellation;

    for(int i=0; i < tessellation; i++){
        float res[4][3];
        for(int j=0; j < tessellation; j++){
            bezP(frac*i, frac*j, points, res[0]);
            bezP(frac*i, frac*(j+1), points, res[1]);
            bezP(frac*(i+1), frac*j, points, res[2]);
            bezP(frac*(i+1), frac*(j+1), points, res[3]);
        }

        outf << res[0][0] << " " << res[0][1] << " " << res[0][2] << endl;
        outf << res[2][0] << " " << res[2][1] << " " << res[2][2] << endl;
        outf << res[3][0] << " " << res[3][1] << " " << res[3][2] << endl;

        outf << res[0][0] << " " << res[0][1] << " " << res[0][2] << endl;
        outf << res[3][0] << " " << res[3][1] << " " << res[3][2] << endl;
        outf << res[1][0] << " " << res[1][1] << " " << res[1][2] << endl;
    }

}



void getPontos(int *patches, float points[][3], float res[][3]){
    for(int i = 0; i < 16; i++){
        res[i][0] = points[patches[i]][0];
        res[i][1] = points[patches[i]][1];
        res[i][2] = points[patches[i]][2];
    }
}

void draw_teapot(char *ifile, char *ofile, int tessellation){

    // teapot.3d
	ofstream outfile;
	outfile.open(ofile);

    // teapot.patch
    ifstream infile;
    infile.open(ifile);
    
    char lineRead[64];

    infile >> lineRead;
    int numPatches = stoi(lineRead);
    /* printf("%d\n", numPatches); */
    int numIndexPatches = 16;
    int patches[numPatches][numIndexPatches];

    for(int i = 0; i<numPatches; i++){
        for(int j = 0; j < numIndexPatches; j ++){
            infile >> lineRead;
            patches[i][j] = stoi(lineRead);
        }
    }

    infile >> lineRead;
    int numVertices = stoi(lineRead);

    float vertices[numVertices][3];
    for(int i = 0; i < numVertices; i++){
        for(int j = 0; j < 3; j++){
            infile >> lineRead;
            vertices[i][j] = stof(lineRead);
        }
    }

    float points[16][3];
    for(int i = 0; i < numPatches; i++){
        /* getPoints(patches[i], vertices, points); */
        getPontos(patches[i], vertices, points);
        /* bezier(points, tessellation, outfile); */
        /* bezierPatch(points, tessellation, outfile); */
        /* pontos[4] = {points[i*4], points[i*4+1] ,points[i*4+2], points[i*4+3]}; */
        bezierCalc(points, tessellation, outfile);

    }
    outfile.close();

    

    
    /* float P[(tessellation+1) * (tessellation+1)]; */

    /* float pointsPatch[numPatches][3]; */
    /* for(int nP = 0; nP < numPatches; nP++){ */
    /*     for(int i = 0; i < 16; i++){ */
    /*         pointsPatch[i][0] = vertices[patches[nP][i]][0]; */
    /*         pointsPatch[i][1] = vertices[patches[nP][i]][1]; */
    /*         pointsPatch[i][2] = vertices[patches[nP][i]][2]; */
    /*     } */

    /*     for(int j = 0, h = 0; j <= tessellation; j++){ */
    /*         for(int i = 0; i <= tessellation; i++, h++){ */
    /*             float u = i/(float)tessellation; */
    /*             float v = j/(float)tessellation; */
    /*             P[h] = bezierDivisions(pointsPatch, u, v); */
    /*         } */
    /*     } */

    /*     float fvertices[tessellation*tessellation]; */
    /*     float nvertices[tessellation*tessellation*4]; */
    /*     for(int j = 0, k = 0; j < tessellation; j++){ */
    /*         for(int i = 0; i < tessellation; i++, k++){ */
    /*             fvertices[k] = 4; */
    /*             nvertices[k * 4] = ((tessellation+1) * j + i); */
    /*             nvertices[k * 4 + 1] = (tessellation+1) * (j + 1) + i; */
    /*             nvertices[k * 4 + 2] = (tessellation+1) * (j + 1) + i + 1; */
    /*             nvertices[k * 4 + 3] = (tessellation+1) * j + i + 1; */
    /*         } */
    /*     } */
    /*     /1* pFinal.push_back(Mesh(tessellation * tessellation, fvertices, nvertices, P); *1/ */
    /* } */

    /* float M[4][4] = {{-1,3,3,1}, {3, -6, 3, 0}, {-3, 3, 0, 0}, {1, 0, 0, 0}}; */


    /* for(int i = 0; i < 16; i++){ */
    /*     for(int j = 0; j < 3; j++){ */
    /*         printf("%f, ", pointsPatch[i][j]); */
    /*     } */
    /*     printf("\n"); */
    /* } */



    /* float patchPoints[16][3]; */
    /* // cicle throw all the patches */
    /* for(int np = 0; np < numPatches; np++){ */
    /*     // patchPoints -> all points for a certain patch */
    /*     for(int i = 0; i < numIndexPatches; i++){ */
    /*         for(int j = 0; j < 3; j++){ */
    /*             patchPoints[i][j] = */
    /*             points[patches[0][i]][j]; */
    /*         } */
    /*     } */
    /*     // function to make points for this patch */
    /*     genBezierPoints(16, 3, patchPoints); */
    /* } */
}

void draw_plane2( char* ofile, float x ){
    x = x/2;
    float y = 0.0;


	ofstream file;
	file.open( ofile );

    // triangulo 0
    file << x << " " << y << " " << x << endl;
    file << x << " " << y << " " << -x << endl;
    file << -x << " " << y << " " << x << endl;

    // triangulo 1
    file << -x << " " << y << " " << x << endl;
    file << x << " " << y << " " << -x << endl;
    file << -x << " " << y << " " << -x << endl;

    file.close();
}

void draw_plane( char* ofile, float x ){
    x = x/2;


	ofstream file;
	file.open( ofile );

    // triangulo sup
    file << "0.0" << " " << "0.0" << " " << "0.0" << endl;
    file << -x << " " << "0.0" << " " << x << endl;
    file << x << " " << "0.0" << " " << x << endl;

    // triangulo inf
    file << "0.0" << " " << "0.0" << " " << "0.0" << endl;
    file << x << " " << "0.0" << " " << -x << endl;
    file << -x << " " << "0.0" << " " << -x << endl;

    // triangulo esq
    file << "0.0" << " " << "0.0" << " " << "0.0" << endl;
    file << -x << " " << "0.0" << " " << -x << endl;
    file << -x << " " << "0.0" << " " << x << endl;

    // triangulo dir
    file << "0.0" << " " << "0.0" << " " << "0.0" << endl;
    file << x << " " << "0.0" << " " << x << endl;
    file << x << " " << "0.0" << " " << -x << endl;

    file.close();
}

void draw_box2( char* ofile, float x, float y, float z, float divisions){

	ofstream file;
	file.open( ofile );

    float divStep = 1 / divisions;
    /* x = x/2; */
    /* y = y/2; */
    /* z = z/2; */
    float xs = x/divisions;
    float ys = y/divisions;
    float zs = z/divisions;
    x = x/2;
    y = y/2;
    z = z/2;


    for(int i = 0; i < divisions; i++){
        for(int j = 0; j < divisions; j++){
            // Lado Frontal
			file << x - (xs * j) <<  " " << y - (ys * i) << " " << z << endl;
			file << x - (xs * (j + 1)) <<  " " <<  y - (ys * (i + 1)) << " " << z << endl;
			file << x - (xs * j) <<  " " << y - (ys * (i + 1)) << " " << z << endl;

			file << x - (xs * j) <<  " " << y - (ys * i) << " " << z << endl;
			file << x - (xs * (j + 1)) <<  " " << y - (ys * i) << " " << z << endl;
			file << x - (xs * (j + 1)) <<  " " << y - (ys * (i + 1)) << " " << z << endl;

            // Lado Traseiro
			file << x - (xs * (j + 1)) <<  " " << y - (ys * i) << " " << -z << endl;
			file << x - (xs * j) <<  " " << y - (ys * (i + 1)) << " " << -z << endl;
			file << x - (xs * (j + 1)) <<  " " << y - (ys * (i + 1)) << " " << -z << endl;

			file << x - (xs * j) <<  " " << y - (ys * (i + 1)) << " " << -z << endl;
			file << x - (xs * (j + 1)) <<  " " << y - (ys * i) << " " << -z << endl;
			file << x - (xs * j) <<  " " << y - (ys * i) << " " << -z<< endl;
            
            // Lado Direito
			file << x <<  " " << y - (ys * i) << " " << z - (zs * (j + 1)) << endl;
			file << x <<  " " << y - (ys * (i+1)) << " " << z - (zs * j) << endl;
			file << x <<  " " << y - (ys * (i+1)) << " " << z - (zs * (j + 1)) << endl;

			file << x <<  " " << y - (ys * (i + 1)) << " " << z - (zs * j) << endl;
			file << x <<  " " << y - (ys * i) << " " << z - (zs * (j + 1)) << endl;
			file << x <<  " " << y - (ys * i) << " " << z - (zs * j) << endl;

            // Lado Esquerdo
			file << -x <<  " " << y - (ys * i) << " " << z - (zs * j) << endl;
			file << -x <<  " " << y - (ys * (i + 1)) << " " << z - (zs * (j + 1)) << endl;
			file << -x <<  " " << y - (ys * (i + 1)) << " " << z - (zs * j) << endl;

			file << -x <<  " " << y - (ys * (i + 1)) << " " << z - (zs * (j + 1)) << endl;
			file << -x <<  " " << y - (ys * i) << " " << z - (zs * j) << endl;
			file << -x <<  " " << y - (ys * i) << " " << z - (zs * (j + 1)) << endl;

            // Topo
			file << x - (xs * j) <<  " " << -y << " " << z - (zs * i) << endl;
			file << x - (xs * (j + 1)) <<  " " << -y << " " << z - (zs * (i + 1)) << endl;
			file << x - (xs * j) <<  " " << -y << " " << z - (zs * (i + 1)) << endl;

			file << x - (xs * (j + 1)) <<  " " << -y << " " << z - (zs * (i + 1)) << endl;
			file << x - (xs * j) <<  " " << -y << " " << z - (zs * i) << endl;
			file << x - (xs * (j + 1)) <<  " " << -y << " " << z - (zs * i) << endl;

            // Base
			file << x - (xs * j) <<  " " << y << " " << z - (zs * (i + 1)) << endl;
			file << x - (xs * (j + 1)) <<  " " << y << " " <<  z - (zs * i) << endl;
			file << x - (xs * j) <<  " " << y << " " << z - (zs * i) << endl;

			file << x - (xs * (j + 1)) <<  " " << y << " " << z - (zs * i) << endl;
			file << x - (xs * j) <<  " " << y << " " << z - (zs * (i + 1)) << endl;
			file << x - (xs * (j + 1)) <<  " " << y << " " << z - (zs * (i + 1)) << endl;
        }
    }

    file.close();
}

void draw_box( char* ofile, float x, float y, float z ){
    x = x/2;
    y = y/2;
    z = z/2;

    ofstream file;
    file.open( ofile );

    // ## Base ##
    // triangulo 1
    file << x << " " << -y << " " << -z << endl;
    file << x << " " << -y << " " << z << endl;
    file << -x << " " << -y << " " << -z << endl;

    // triangulo 2
    file << -x << " " << -y << " " << -z << endl;
    file << x << " " << -y << " " << z << endl;
    file << -x << " " << -y << " " << z << endl;

    // ## Lado esq ##
    // triangulo 1
    file << -x << " " << y << " " << -z << endl;
    file << -x << " " << -y << " " << -z << endl;
    file << -x << " " << -y << " " << z << endl;

    // triangulo 2
    file << -x << " " << -y << " " << z << endl;
    file << -x << " " << y << " " << z << endl;
    file << -x << " " << y << " " << -z << endl;

    // ## Lado dir ##
    // triangulo 1
    file << x << " " << -y << " " << z << endl;
    file << x << " " << -y << " " << -z << endl;
    file << x << " " << y << " " << -z << endl;

    // triangulo 2
    file << x << " " << y << " " << -z << endl;
    file << x << " " << y << " " << z << endl;
    file << x << " " << -y << " " << z << endl;

    // ## Tras ##
    // triangulo 1
    file << -x << " " << -y << " " << -z << endl;
    file << x << " " << y << " " << -z << endl;
    file << x << " " << -y << " " << -z << endl;

    // triangulo 2
    file << -x << " " << -y << " " << -z << endl;
    file << -x << " " << y << " " << -z << endl;
    file << x << " " << y << " " << -z << endl;

    // ## Frente ##
    // triangulo 1
    file << x << " " << -y << " " << z << endl;
    file << x << " " << y << " " << z << endl;
    file << -x << " " << -y << " " << z << endl;

    // triangulo 2
    file << x << " " << y << " " << z << endl;
    file << -x << " " << y << " " << z << endl;
    file << -x << " " << -y << " " << z << endl;

    // ## Topo ##
    // triangulo 1
    file << -x << " " << y << " " << -z << endl;
    file << -x << " " << y << " " << z << endl;
    file << x << " " << y << " " << z << endl;

    // triangulo 2
    file << x << " " << y << " " << z << endl;
    file << x << " " << y << " " << -z << endl;
    file << -x << " " << y << " " << -z << endl;

    file.close();
}

void draw_sphere( char* ofile, float raio, float slices, float stacks ){
    ofstream file;
    file.open( ofile );

    stacks = div((int)stacks,2).quot + 1;

    float sliceStep = ( 2 * M_PI ) / slices;
    float stackStep = M_PI / stacks;

    int i;
    int j;

    for( i = 0; i < slices; i++ ){
        for( j = 0; j <= stacks; j++ ){

            // beta
            float stackAngle =  M_PI  / 2 - j  * stackStep;
            // alpha
            float nextstackAngle = M_PI  /  2 - ( j + 1 ) * stackStep;

            // next beta
            float sliceAngle = i * sliceStep;
            // next alpha
            float nextsliceAngle = ( i + 1 ) * sliceStep;

            // defenicao dos 4 pontos usados em cada iteracao para o desenho dos 2 triangulos
            float x11 = raio * cos( stackAngle ) * sin( sliceAngle );
            float y11 = raio * sin( stackAngle );
            float z11 = raio * cos( stackAngle ) * cos( sliceAngle );

            float x21 = raio * cos( stackAngle ) * sin( nextsliceAngle );
            float y21 = raio * sin( stackAngle );
            float z21 = raio * cos( stackAngle ) * cos( nextsliceAngle );

            float x22 = raio * cos( nextstackAngle ) * sin( nextsliceAngle );
            float y22 = raio * sin( nextstackAngle );
            float z22 = raio * cos( nextstackAngle ) * cos( nextsliceAngle );

            float x12 = raio * cos( nextstackAngle ) * sin( sliceAngle );
            float y12 = raio * sin( nextstackAngle );
            float z12 = raio * cos( nextstackAngle ) * cos( sliceAngle );

            if(j > 0){
            file << x11 << " " << y11 << " " << z11 << endl;
            file << x22 << " " << y22 << " " << z22 << endl;
            file << x21 << " " << y21 << " " << z21 << endl;
            }

            if(j < stacks){
            file << x11 << " " << y11 << " " << z11 << endl;
            file << x12 << " " << y12 << " " << z12 << endl;
            file << x22 << " " << y22 << " " << z22 << endl;
            }
        }

    }

    file.close();
}


void draw_cone(char* ofile,float raio, float altura, int slices, int stack){
	ofstream file;
	file.open( ofile );

	float angulo = (2*M_PI)/(float)slices;
	float nblock = altura/(float)stack;


	//Criar a base
	for(int i= 0;i<slices;i++){
		file << "0.0" << " " << "0.0" << " " << "0.0" << endl;
		file << raio*cos(angulo*i) << " " << "0.0" << " " << raio*sin(angulo*i) << endl;
		file << raio*cos(angulo*(i+1)) << " " << "0.0" << " " << raio*sin(angulo*(i+1)) << endl;

	}


	//criar as stacks
	for(int i= 0;i<slices;i++){
		for(int j=0;j<stack;j++){
			file << raio*((stack-(j+1))/(float)stack)*cos(angulo*i) << " " << (j+1)*nblock << " " << raio*((stack-(j+1))/(float)stack)*sin(angulo*i) << endl;
			file << raio*((stack-j)/(float)stack)*cos(angulo*(i+1)) << " " << j*nblock << " " << raio*((stack-j)/(float)stack)*sin(angulo*(i+1)) << endl;
			file << raio*((stack-j)/(float)stack)*cos(angulo*i) << " " << j*nblock << " " << raio*((stack-j)/(float)stack)*sin(angulo*i) << endl;


			file << raio*((stack-(j+1))/(float)stack)*cos(angulo*i) << " " << (j+1)*nblock << " " << raio*((stack-(j+1))/(float)stack)*sin(angulo*i) << endl;
			file << raio*((stack-(j+1))/(float)stack)*cos(angulo*(i+1)) << " " << (j+1)*nblock << " " << raio*((stack-(j+1))/(float)stack)*sin(angulo*(i+1)) << endl;
			file << raio*((stack-j)/(float)stack)*cos(angulo*(i+1)) << " " << j*nblock << " " << raio*((stack-j)/(float)stack)*sin(angulo*(i+1)) << endl;

		}
	}

	file.close();
}



int main( int argc, char **argv ){
  char ofile[100];

    switch( argc ){ 
        case ( 4 ):
            // draw_pane
            // generator plane size filename.3d
            if( !strcmp( argv[1], "plane" ) ){ 
                float x = atof( argv[2] );

                if( x <= 0 ){
                    printf( "Erro, As dimensoes tem de ser positivas.\n" );
                    exit(1);
                }

                strcpy( ofile, argv[3] );
                draw_plane2( ofile, x );
            }

            else
                if(!strcmp(argv[1], "teapot.patch")){
                    char infile[100];
                    strcpy( infile, argv[1]);
                    strcpy( ofile, argv[3] );
                    int tessellation = stoi(argv[2]);
                    draw_teapot(infile, ofile, tessellation);
                    /* printf("infile: %s\nofile: %s\ntessellation: %d\n", infile, ofile, tessellation); */
                }

        case ( 6 ):
            // draw_box
            // generator box sizex sizey sizez box.3d
            if( !strcmp( argv[1], "box" ) ){
                float x = atof( argv[2] );
                float y = atof( argv[3] );
                float z = atof( argv[4] );

                if( x <= 0 || y <=0 || z <=0 ){
                    printf( "Erro, As dimensoes tem de ser positivas.\n" );
                    exit(1);
                }

                strcpy( ofile, argv[5] );
                draw_box( ofile, x, y, z );
            }



            if( !strcmp( argv[1], "sphere" ) ){
                float r = atof( argv[2] );
                float slices = atof( argv[3] );
                float stacks = atof(argv[4] );

                if( r <= 0 || slices < 3 || stacks < 4 ){
                    printf( "Erro, Dados nao validos.\n" );
                    exit(1);
                }

                strcpy( ofile, argv[5] );
                draw_sphere( ofile, r, slices, stacks);

            }


        case ( 7 ):
            if ( !strcmp( argv[1], "cone" ) ){
                float radius = atof( argv[2] );
                float height = atof( argv[3] );
                float slices = atoi( argv[4] );
                float stacks = atoi( argv[5] );

                if( radius <= 0 || height <= 0 ||slices < 3 || stacks < 1 ){
                    printf( "Erro, Dados nao validos.\n" );
                    exit(1);
                }

                strcpy( ofile, argv[6] );
                draw_cone( ofile, radius, height, slices, stacks);
            }
            if (!strcmp(argv[1], "box")){
                float x = atof(argv[2]);
                float y = atof(argv[3]);
                float z = atof(argv[4]);
                float d = atof(argv[5]);

                if( x <= 0 || y <=0 || z <= 0 || d <= 0 ){
                    printf("Dados nao validos\n");
                    exit(1);
                }

                strcpy(ofile, argv[6]);
                draw_box2(ofile, x, y, z, d);
            }

    }

    return( 0 );
}
