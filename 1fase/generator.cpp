#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;


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

/* void draw_box2( char* ofile, float x, float y, float z, float divisions){ */

/*     float divStep = 1 / divisions; */
/*     /1* x = x/2; *1/ */
/*     /1* y = y/2; *1/ */
/*     /1* z = z/2; *1/ */

/*     int i, j; */
/*     for( i = 0; i < divisions; i++ ){ */
/*             float pz = z/2; */
/*             float px = -(x/2); */
/*         for( j = 0; j < divisions; j++ ){ */


/*             file << x << " " << y << " " << pz << endl; */
/*             file << x << " " << y << " " << pz << endl; */
/*             file << x << " " << y << " " << pz << endl; */

/*             file << x << " " << y << " " << pz << endl; */
/*             file << x << " " << y << " " << pz << endl; */
/*             file << x << " " << y << " " << pz << endl; */

/*         } */
/*     } */


/*     } */
/* } */

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
                draw_plane( ofile, x );
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

    }

    return( 0 );
}
