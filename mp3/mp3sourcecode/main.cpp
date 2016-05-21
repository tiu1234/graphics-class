
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <math.h>
#include <SDL/SDL.h>
#include <vector>

using namespace std;

#define MAX_LINE 200

int nFPS = 30;
FILE* file;

float rotate_angle = -99.0;
int pressed = 0;
double speed;
int last_x;
unsigned int tex;
unsigned int cube;
int mode = 0;
float img_w, img_h;

typedef struct Vertex{
	float x, y, z;
	float n_x, n_y, n_z;
	int count;
}Vertex;

typedef struct Triangle{
	int a, b, c;
}Triangle;

vector<Vertex> vertices;
vector<Triangle> triangles;

void readline(){
	vertices.clear();
	triangles.clear();
	char trash;
	while((trash = fgetc(file)) != EOF){
		if(trash == 'v' && (trash = fgetc(file)) == ' '){
			float x, y, z;
			if(fscanf(file, "%f %f %f\n", &x, &y, &z) == EOF){
				return;
			}
			Vertex temp;
			temp.x = x;
			temp.y = y;
			temp.z = z;
			temp.n_x = temp.n_y = temp.n_z = 0.0;
			temp.count = 0;
			vertices.push_back(temp);
		}
		else if(trash == 'f' && (trash = fgetc(file)) == ' '){
			int a, b, c;
			if(fscanf(file, "%d %d %d\n", &a, &b, &c) == EOF){
				return;
			}
			Triangle temp;
			temp.a = a-1;
			temp.b = b-1;
			temp.c = c-1;
			triangles.push_back(temp);
			float new1_x = vertices[temp.b].x-vertices[temp.a].x;
			float new1_y = vertices[temp.b].y-vertices[temp.a].y;
			float new1_z = vertices[temp.b].z-vertices[temp.a].z;
			float mag = new1_x*new1_x+new1_y*new1_y+new1_z*new1_z;
			mag = sqrt(mag);
			new1_x /= mag;
			new1_y /= mag;
			new1_z /= mag;
			float new2_x = vertices[temp.c].x-vertices[temp.a].x;
			float new2_y = vertices[temp.c].y-vertices[temp.a].y;
			float new2_z = vertices[temp.c].z-vertices[temp.a].z;
			mag = new2_x*new2_x+new2_y*new2_y+new2_z*new2_z;
			mag = sqrt(mag);
			new2_x /= mag;
			new2_y /= mag;
			new2_z /= mag;
			float nor_x = new1_y*new2_z-new1_z*new2_y;
			float nor_y = new1_z*new2_x-new1_x*new2_z;
			float nor_z = new1_x*new2_y-new1_y*new2_x;
			mag = nor_x*nor_x+nor_y*nor_y+nor_z*nor_z;
			mag = sqrt(mag);
			nor_x /= mag;
			nor_y /= mag;
			nor_z /= mag;
			vertices[temp.a].n_x += nor_x;
			vertices[temp.b].n_x += nor_x;
			vertices[temp.c].n_x += nor_x;
			vertices[temp.a].n_y += nor_y;
			vertices[temp.b].n_y += nor_y;
			vertices[temp.c].n_y += nor_y;
			vertices[temp.a].n_z += nor_z;
			vertices[temp.b].n_z += nor_z;
			vertices[temp.c].n_z += nor_z;
			vertices[temp.a].count++;
			vertices[temp.b].count++;
			vertices[temp.c].count++;
		}
	}
	for(unsigned int i = 0; i < vertices.size(); i++){
		vertices[i].n_x /= vertices[i].count;
		vertices[i].n_y /= vertices[i].count;
		vertices[i].n_z /= vertices[i].count;
	}
}

unsigned int loadTexture(const char* filename)
{
	SDL_Surface* img = SDL_LoadBMP(filename);
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, img->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	img_w = (float)img->w;
	img_h = (float)img->h;
	SDL_FreeSurface(img);
	return id;
}

unsigned int loadCube(const char* filename)
{
	SDL_Surface* img = SDL_LoadBMP(filename);
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, img->pixels);
	SDL_FreeSurface(img);
	return id;
}

void init(){

	GLfloat amb[] = {0.2,0.2,0.2};
	GLfloat diff[] = {1.0,1.0,1.0};
	GLfloat spec[] = {1.0,1.0,1.0};
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

	glClearColor (0.5, 0.5, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	
	readline();
	cube = loadCube("cube.bmp");
	tex = loadTexture("text.bmp");
}

void display(){
	GLfloat lpos[] = {5.0, 5.0, 10.0, 0.0};
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(5.0*cos(rotate_angle), 3.5, 5.0*sin(rotate_angle), 0.0, 1.0, 0.0, 0.0, 1.0, 0.0);
	glPushMatrix();
		GLfloat ambient[]  = {1.0f, 1.0f, 1.0f, 1.0f};  
		GLfloat diffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};   
		GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};   
		GLfloat emission[] = {0.0f, 0.0f, 0.0f, 1.0f};      
		GLfloat shininess  = {30.0f};
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, emission);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);
		if(!mode){
			glBindTexture(GL_TEXTURE_2D, tex);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}
		else{
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_2D, cube);
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
		}
		
		for(unsigned int i = 0; i < triangles.size(); i++){
			float temp_x, temp_y;
			double a_z, b_z, c_z;
			int text_flag = 0;
			glBegin(GL_TRIANGLES);
				if(!mode){
					a_z = atan2(vertices[triangles[i].a].z, vertices[triangles[i].a].x);
					b_z = atan2(vertices[triangles[i].b].z, vertices[triangles[i].b].x);
					c_z = atan2(vertices[triangles[i].c].z, vertices[triangles[i].c].x);
					if(a_z > (5/90)*3.1415 && b_z > (5/90)*3.1415 && c_z > (5/90)*3.1415 && vertices[triangles[i].a].y < 2.8 && vertices[triangles[i].b].y < 2.8 && vertices[triangles[i].c].y < 2.8){
						text_flag = 1;
					}
					if(text_flag){
						temp_x = (a_z+3.1415)/6.283*1.97-0.90;
						temp_y = 1.0-vertices[triangles[i].a].y*65.0/img_h+0.05;
						glTexCoord2f(temp_x, temp_y);
					}
				}
				glNormal3f(vertices[triangles[i].a].n_x, vertices[triangles[i].a].n_y, vertices[triangles[i].a].n_z);
				glVertex3f(vertices[triangles[i].a].x, vertices[triangles[i].a].y, vertices[triangles[i].a].z);
				
				if(!mode){
					if(text_flag){
						temp_x = (b_z+3.1415)/6.283*1.97-0.90;
						temp_y = 1.0-vertices[triangles[i].b].y*65.0/img_h+0.05;
						glTexCoord2f(temp_x, temp_y);
					}
				}
				glNormal3f(vertices[triangles[i].b].n_x, vertices[triangles[i].b].n_y, vertices[triangles[i].b].n_z);
				glVertex3f(vertices[triangles[i].b].x, vertices[triangles[i].b].y, vertices[triangles[i].b].z);
				
				if(!mode){
					if(text_flag){
						temp_x = (c_z+3.1415)/6.283*1.97-0.90;
						temp_y = 1.0-vertices[triangles[i].c].y*65.0/img_h+0.05;
						glTexCoord2f(temp_x, temp_y);
					}
				}
				glNormal3f(vertices[triangles[i].c].n_x, vertices[triangles[i].c].n_y, vertices[triangles[i].c].n_z);
				glVertex3f(vertices[triangles[i].c].x, vertices[triangles[i].c].y, vertices[triangles[i].c].z);
			glEnd();
		}
	glPopMatrix();


	glutSwapBuffers();
	glFlush ();

	glutPostRedisplay();
}

void reshape(int w, int h){
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0,(float)w/h,0.01,10.0);
	glMatrixMode (GL_MODELVIEW);
}

void timer(int v){
	if(!pressed){
		rotate_angle += speed;
		if(speed < 0.0){
			if(speed+0.001 >= 0.0){
				speed = 0.0;
			}
			else{
				speed += 0.001;
			}
		}
		if(speed > 0.0){
			if(speed-0.001 <= 0.0){
				speed = 0.0;
			}
			else{
				speed -= 0.001;
			}
		}
	}
	if(rotate_angle >= 360.0){
		rotate_angle -= 360.0;
	}
	if(rotate_angle <= -360.0){
		rotate_angle += 360.0;
	}
	glutPostRedisplay();
	glutTimerFunc(1000/nFPS,timer,v);
}

void keyboard(unsigned char key, int x, int y){
	switch(key){
		case 27:
			exit(0);
			break;
		case 'q':
			mode = ~mode;
			break;
		default:
			break;
	}
}

void mouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		pressed = 1;
		last_x = x;
		speed = 0.0;
	}
	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		pressed = 0;
	}
}

void motion(int x, int y){
	if(pressed){
		speed = ((double)(x-last_x))/100.0;
		rotate_angle += ((double)(x-last_x))/200.0;
		last_x = x;
	}
}

int main(int argc, char** argv){
	file = fopen("teapot_0.obj", "rb");
	if(file == NULL){
		printf("No \"teapot_0.obj\".\n");
		return 0;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("mp3");
	init();
	fclose(file);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(100,timer,nFPS);
	glutMainLoop();
	return 0;
}

