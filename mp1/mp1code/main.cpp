//  
// Example code for OpenGL programming
//
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <ctime>
#include <SOIL/SOIL.h>
#include <math.h>

int nFPS = 30;
float fRotateAngle = 0.f;
clock_t startClock=0,curClock;
long long int prevF=0,curF=0;
int dipMode=1;
float vertices[16][2];
int flag = 0;

void init(void)
{
	// init your data, setup OpenGL environment here
	glClearColor(0.5,0.5,1.0,1.0); // clear color is gray		
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment this function if you only want to draw wireframe model
					// GL_POINT / GL_LINE / GL_FILL (default)
	glPointSize(4.0);
	//initialization for triangles
	vertices[0][0] = -0.6;
	vertices[0][1] = 0.6;
	vertices[1][0] = -0.6;
	vertices[1][1] = 1.0;
	vertices[2][0] = -0.3;
	vertices[2][1] = 0.6;
	vertices[3][0] = 0.6;
	vertices[3][1] = 1;
	vertices[4][0] = 0.3;
	vertices[4][1] = 0.6;
	vertices[5][0] = 0.6;
	vertices[5][1] = 0.6;
	
	vertices[6][0] = 0.0;
	vertices[6][1] = -1.0;
	vertices[7][0] = -0.6;
	vertices[7][1] = -1.0;
	vertices[8][0] = -0.6;
	vertices[8][1] = -0.6;
	vertices[9][0] = -0.3;
	vertices[9][1] = -0.6;
	vertices[10][0] = -0.3;
	vertices[10][1] = 0.6;
	vertices[11][0] = 0.3;
	vertices[11][1] = 0.6;
	vertices[12][0] = 0.3;
	vertices[12][1] = -0.6;
	vertices[13][0] = 0.6;
	vertices[13][1] = -0.6;
	vertices[14][0] = 0.6;
	vertices[14][1] = -1.0;
}

void display(void)
{
	if(dipMode==1)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	

	curF++;
	// put your OpenGL display commands here
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// reset OpenGL transformation matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset transformation matrix to identity

	// setup look at transformation so that 
	// eye is at : (0,0,3)
	// look at center is at : (0,0,0)
	// up direction is +y axis
	gluLookAt(0.f,0.f,3.f,0.f,0.f,0.f,0.f,1.f,0.f);
	glRotatef(fRotateAngle,0.f,1.f,0.f);

	// Test drawing a solid teapot
	glColor3f(1.0,0.5,0.0); // set current color to Orange
	//glutSolidTeapot(1.f); // call glut utility to draw a solid teapot
	
	//triangles
	int index = 0;
	glBegin(GL_TRIANGLE_STRIP );
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(vertices[index][0], vertices[index][1]);
		index++;
	glEnd();

	//animation
    if((int)fRotateAngle%10 == 0){
		for(int i = 0; i < 15; i++){
			if(flag == 0){
				if(vertices[i][0] > 0){
					vertices[i][0] += 0.1;
				}
				else{
					vertices[i][0] -= 0.1;
				}
				if(vertices[i][1] > 0){
					vertices[i][1] += 0.1;
				}
				else{
					vertices[i][1] -= 0.1;
				}
			}
			else{
				if(vertices[i][0] > 0){
					vertices[i][0] -= 0.1;
				}
				else{
					vertices[i][0] += 0.1;
				}
				if(vertices[i][1] > 0){
					vertices[i][1] -= 0.1;
				}
				else{
					vertices[i][1] += 0.1;
				}
			}
		}
		if(flag == 0){
			flag = 1;
		}
		else{
			flag = 0;
		}
	}
	//glFlush();
	glutSwapBuffers();	// swap front/back framebuffer to avoid flickering 

	curClock=clock();
	float elapsed=(curClock-startClock)/(float)CLOCKS_PER_SEC;
	if(elapsed>1.0f){
		float fps=(float)(curF-prevF)/elapsed;
		printf("fps:%f\n",fps);
		prevF=curF;
		startClock=curClock;
	}
}
 
void reshape (int w, int h)
{
	// reset viewport ( drawing screen ) size
	glViewport(0, 0, w, h);
	float fAspect = ((float)w)/h; 
	// reset OpenGL projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.f,fAspect,0.001f,30.f); 
}



void keyboard(unsigned char key, int x, int y)
{
	// put your keyboard control here
	if (key == 27) 
	{
		// ESC hit, so quit
		printf("demonstration finished.\n");
		exit(0);
	}
	if( key == 'h'){
		dipMode = 1-dipMode;
	}
}

void mouse(int button, int state, int x, int y)
{
	// process your mouse control here
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		printf("push left mouse button.\n");
}


void timer(int v)
{
	fRotateAngle += 1.f; // change rotation angles
    if(fRotateAngle >= 360){
        fRotateAngle = 0.0;
    }
    
	glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again
}

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);
	// set up for double-buffering & RGB color buffer & depth test
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
	glutInitWindowSize (500, 500); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ((const char*)"Hello Triangle");

	init(); // setting up user data & OpenGL environment
	
	// set up the call-back functions 
	glutDisplayFunc(display);  // called when drawing 
	glutReshapeFunc(reshape);  // called when change window size
	glutKeyboardFunc(keyboard); // called when received keyboard interaction
	glutMouseFunc(mouse);	    // called when received mouse interaction
	glutTimerFunc(100,timer,nFPS); // a periodic timer. Usually used for updating animation
	
	startClock=clock();

	glutMainLoop(); // start the main message-callback loop

	return 0;
}
