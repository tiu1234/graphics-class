
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <SDL/SDL.h>
#include <math.h>
#include <vector>

using namespace std;


int nFPS = 30;

GLuint tex;
float img_w, img_h;
float y_max = 0.0f;
float time = 0.0f;
int LEVEL = 3;
glm::vec3 eye = glm::vec3(-4.0, 0.0, 4.0);
glm::vec3 look_at = glm::vec3(0.0, 0.0, 0.0);
GLenum mode = GL_FILL;

typedef struct Vertex Vertex;
typedef struct Edge Edge;
typedef struct Face Face;

struct Vertex{
	int index;
	int connected;
	glm::vec3 normal;
	glm::vec2 tex_coords;
	glm::vec3 coords;
	glm::vec3 new_coords;
	Edge* start_edge;
};

struct Edge{
	int index;
    glm::vec3 edge_vertex;
	Edge* pair;
    Edge* next;
	Face* face;
	Vertex* end_vertex;
};

struct Face{
	int index;
	glm::vec3 face_vertex;
	glm::vec4 adj;
	glm::vec3 normal;
	Edge* start_edge;
};

vector<glm::vec3> vertices;
vector<glm::vec4> faces;
vector<Vertex*> vertices_level[4];
vector<Face*> faces_level[4];

void loadTexture(const char* filename){
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	SDL_Surface* img = SDL_LoadBMP(filename);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, img->pixels);
	SDL_FreeSurface(img);
}

void half_edge(int level){
	for(int i = 0; i < (int)faces_level[level].size();i++){
		faces_level[level][i]->face_vertex = glm::vec3(0.0, 0.0, 0.0);
		Edge** temp = &(faces_level[level][i]->start_edge);
		for(int j = 0; j <= 3; j++){
			*temp = (Edge*)malloc(sizeof(Edge));
			(*temp)->edge_vertex = glm::vec3(0.0, 0.0, 0.0);
			(*temp)->pair = NULL;
			if(j == 3){
				(*temp)->next = faces_level[level][i]->start_edge;
			}
			(*temp)->face = faces_level[level][i];
			temp = &((*temp)->next);
		}
	}
	for(int i = 0; i < (int)faces_level[level].size();i++){
		Edge* temp = faces_level[level][i]->start_edge;
		for(int j = 0; j <= 3; j++){
			int index = faces_level[level][i]->adj[(j+1)%4];
			temp->end_vertex = vertices_level[level][index];
			if(vertices_level[level][index]->start_edge == NULL){
				vertices_level[level][index]->start_edge = temp->next;
			}
			temp = temp->next;
		}
	}
	for(int i = 0; i < (int)faces_level[level].size(); i++){
		Edge* temp = faces_level[level][i]->start_edge;
		for(int j = 0 ; j <= 3; j++){
			faces_level[level][i]->face_vertex += vertices_level[level][faces_level[level][i]->adj[j]]->coords;
			if(temp->pair == NULL){
				Vertex* end = temp->end_vertex;
				Vertex* start = temp->next->next->next->end_vertex;
				bool found = false;
				for(int k = 0; !found && k < (int)faces_level[level].size(); k++){
					found = false;
					if(k == i){
						continue;
					}
					Edge* search = faces_level[level][k]->start_edge;
					for(int m = 0; m <= 3; m++){
						if(start == search->end_vertex && end == search->next->next->next->end_vertex){
							temp->pair = search;
							search->pair = temp;
							found = true;
							break;
						}
						search = search->next;
					}
				}
			}
			temp = temp->next;
		}
		faces_level[level][i]->face_vertex /= 4;
	}
	for(int i = 0; i < (int)faces_level[level].size(); i++){
		Edge* temp = faces_level[level][i]->start_edge;
		for(int j = 0; j < 4; j++){
			if(temp->edge_vertex == glm::vec3(0.0, 0.0, 0.0)){
				temp->edge_vertex = faces_level[level][i]->face_vertex+temp->pair->face->face_vertex+temp->end_vertex->coords+temp->pair->end_vertex->coords;
				temp->edge_vertex /= 4;
				temp->pair->edge_vertex = temp->edge_vertex;
			}
			temp = temp->next;
		}
	}
	for(int i = 0; i < (int)vertices_level[level].size(); i++){
		Vertex* v = vertices_level[level][i];
		Edge* e = v->start_edge;
		float count = 0.0f;
		glm::vec3 a = glm::vec3(0.0, 0.0, 0.0);
		glm::vec3 b = glm::vec3(0.0, 0.0, 0.0);
		do{
			a += e->face->face_vertex;
			b += e->edge_vertex;
			count += 1.0f;
			e = e->pair->next;
		}while(e != v->start_edge);
		a /= count*count;
		b = ((b*2.0f)/count)/count;
		v->new_coords = a+b+((count-3.0f)/count)*(v->coords);
	}
}

void catmull(int level){
	vector<glm::vec4 > new_faces;
	vector<glm::vec3> new_vertices;
	for(int i = 0; i < (int)faces_level[level-1].size(); i++){
		Edge* e = faces_level[level-1][i]->start_edge;
		for(int j = 0; j <= 3; j++){
			int start = faces_level[level-1][i]->adj[j];
			glm::vec4 new_face;
			vector<glm::vec3> newpoints(4);
			newpoints[0] = vertices_level[level-1][start]->new_coords;
			newpoints[1] = e->edge_vertex;
			newpoints[2] = faces_level[level-1][i]->face_vertex;
			newpoints[3] = e->next->next->next->edge_vertex;
			for(int k = 0; k <= 3; k++){
				int pos = -1;
				for(int n = 0; n < (int)new_vertices.size(); n++){
					if(new_vertices[n] == newpoints[k]){
						pos = n;
						break;
					}
				}
				if(pos != -1){
					new_face[k] = pos;
				}
				else{
					new_vertices.push_back(newpoints[k]);
					new_face[k] = new_vertices.size()-1;
				}
			}
			new_faces.push_back(new_face);
			e = e->next;
		}
	}
	for(int i = 0; i < (int)new_vertices.size(); i++){
		Vertex* v = (Vertex*)malloc(sizeof(Vertex));
		v->index = i;
		v->connected = 0;
		v->coords = new_vertices[i];
		v->start_edge = NULL;
		if(new_vertices[i].y > y_max){
			y_max = new_vertices[i].y;
		}
		vertices_level[level].push_back(v);
	}
	for(int i = 0; i < (int)new_faces.size(); i++){
		Face* new_face = (Face*)malloc(sizeof(Face));
		new_face->start_edge = NULL;
		new_face->face_vertex = glm::vec3(0.0, 0.0, 0.0);
		new_face->adj= new_faces[i];
		new_face->normal = glm::normalize(glm::cross(vertices_level[level][new_faces[i].y]->coords-vertices_level[level][new_faces[i].x]->coords, vertices_level[level][new_faces[i].z]->coords-vertices_level[level][new_faces[i].x]->coords));
		vertices_level[level][new_faces[i].x]->normal += new_face->normal;
		vertices_level[level][new_faces[i].y]->normal += new_face->normal;
		vertices_level[level][new_faces[i].z]->normal += new_face->normal;
		vertices_level[level][new_faces[i].w]->normal += new_face->normal;
		faces_level[level].push_back(new_face);
	}
	for(int i = 0; i < (int)new_vertices.size(); i++){
		vertices_level[level][i]->normal = glm::normalize(vertices_level[level][i]->normal);
		vertices_level[level][i]->tex_coords = glm::vec2((atan2(vertices_level[level][i]->coords.x, vertices_level[level][i]->coords.z)+3.1415926)/(2*3.1415926), vertices_level[level][i]->coords.y/y_max);
	}
	new_faces.clear();
	new_vertices.clear();
	half_edge(level);
}

void init(){
	GLfloat amb[] = {0.0, 0.0, 0.0};
	GLfloat diff[] = {1.0, 1.0, 1.0};
	GLfloat spec[] = {1.0, 1.0, 1.0};
	GLfloat light_position[] = {3.0, 2.0, 2.0, 0.0};
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glClearColor(0.5, 0.5, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	
	GLfloat ambient[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuse[] = {0.8, 0.8, 0.8, 1.0};
	GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat shininess[] = {50.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess); 
	
	vertices.push_back(glm::vec3(-1.0, 1.2, 0.6));
	vertices.push_back(glm::vec3(-1.0, 0.6, 0.6));
	vertices.push_back(glm::vec3(-0.5, 0.6, 0.6));
	vertices.push_back(glm::vec3(-0.5, -0.6, 0.6));
	vertices.push_back(glm::vec3(-1.0, -0.6, 0.6));
	vertices.push_back(glm::vec3(-1.0, -1.2, 0.6));
	vertices.push_back(glm::vec3(-0.5, -1.2, 0.6));
	vertices.push_back(glm::vec3(0.5, -1.2, 0.6));
	vertices.push_back(glm::vec3(1.0, -1.2, 0.6));
	vertices.push_back(glm::vec3(1.0, -0.6, 0.6));
	vertices.push_back(glm::vec3(0.5, -0.6, 0.6));
	vertices.push_back(glm::vec3(0.5, 0.6, 0.6));
	vertices.push_back(glm::vec3(1.0, 0.6, 0.6));
	vertices.push_back(glm::vec3(1.0, 1.2, 0.6));
	vertices.push_back(glm::vec3(0.5, 1.2, 0.6));
	vertices.push_back(glm::vec3(-0.5, 1.2, 0.6));
	vertices.push_back(glm::vec3(-1.0, 1.2, 0.0));
	vertices.push_back(glm::vec3(-1.0, 0.6, 0.0));
	vertices.push_back(glm::vec3(-0.5, 0.6, 0.0));
	vertices.push_back(glm::vec3(-0.5, -0.6, 0.0));
	vertices.push_back(glm::vec3(-1.0, -0.6, 0.0));
	vertices.push_back(glm::vec3(-1.0, -1.2, 0.0));
	vertices.push_back(glm::vec3(-0.5, -1.2, 0.0));
	vertices.push_back(glm::vec3(0.5, -1.2, 0.0));
	vertices.push_back(glm::vec3(1.0, -1.2, 0.0));
	vertices.push_back(glm::vec3(1.0, -0.6, 0.0));
	vertices.push_back(glm::vec3(0.5, -0.6, 0.0));
	vertices.push_back(glm::vec3(0.5, 0.6, 0.0));
	vertices.push_back(glm::vec3(1.0, 0.6, 0.0));
	vertices.push_back(glm::vec3(1.0, 1.2, 0.0));
	vertices.push_back(glm::vec3(0.5, 1.2, 0.0));
	vertices.push_back(glm::vec3(-0.5, 1.2, 0.0));
	faces.push_back(glm::vec4(0, 1, 2, 15));
	faces.push_back(glm::vec4(15, 2, 11, 14));
	faces.push_back(glm::vec4(14, 11, 12, 13));
	faces.push_back(glm::vec4(2, 3, 10, 11));
	faces.push_back(glm::vec4(4, 5, 6, 3));
	faces.push_back(glm::vec4(3, 6, 7, 10));
	faces.push_back(glm::vec4(10, 7, 8, 9));
	faces.push_back(glm::vec4(16, 31, 18, 17));
	faces.push_back(glm::vec4(31, 30, 27, 18));
	faces.push_back(glm::vec4(30, 29, 28, 27));
	faces.push_back(glm::vec4(18, 27, 26, 19));
	faces.push_back(glm::vec4(20, 19, 22, 21));
	faces.push_back(glm::vec4(19, 26, 23, 22));
	faces.push_back(glm::vec4(26, 25, 24, 23));
	faces.push_back(glm::vec4(0, 16, 17, 1));
	faces.push_back(glm::vec4(1, 17, 18, 2));
	faces.push_back(glm::vec4(2, 18, 19, 3));
	faces.push_back(glm::vec4(3, 19, 20, 4));
	faces.push_back(glm::vec4(4, 20, 21, 5));
	faces.push_back(glm::vec4(5, 21, 22, 6));
	faces.push_back(glm::vec4(6, 22, 23, 7));
	faces.push_back(glm::vec4(7, 23, 24, 8));
	faces.push_back(glm::vec4(8, 24, 25, 9));
	faces.push_back(glm::vec4(9, 25, 26, 10));
	faces.push_back(glm::vec4(10, 26, 27, 11));
	faces.push_back(glm::vec4(11, 27, 28, 12));
	faces.push_back(glm::vec4(12, 28, 29, 13));
	faces.push_back(glm::vec4(13, 29, 30, 14));
	faces.push_back(glm::vec4(14, 30, 31, 15));
	faces.push_back(glm::vec4(15, 31, 16, 0));
	
	for(int i = 0; i < (int)vertices.size(); i++){
		Vertex* v = (Vertex*)malloc(sizeof(Vertex));
		v->index = i;
		v->connected = 0;
		v->coords = vertices[i];
		v->start_edge = NULL;
		v->new_coords = glm::vec3(0.0, 0.0, 0.0);
		
		if(vertices[i].y > y_max){
			y_max = vertices[i].y;
		}
		vertices_level[0].push_back(v);
	}
	
	for(int i = 0; i < (int)faces.size(); i++){
		Face* new_face = (Face*)malloc(sizeof(Face));
		new_face->start_edge = NULL;
		new_face->face_vertex = glm::vec3(0.0, 0.0, 0.0);
		new_face->adj= faces[i];
		new_face->normal = glm::normalize(glm::cross(vertices_level[0][faces[i].y]->coords-vertices_level[0][faces[i].x]->coords, vertices_level[0][faces[i].z]->coords-vertices_level[0][faces[i].x]->coords));
		vertices_level[0][faces[i].x]->normal += new_face->normal;
		vertices_level[0][faces[i].y]->normal += new_face->normal;
		vertices_level[0][faces[i].z]->normal += new_face->normal;
		vertices_level[0][faces[i].w]->normal += new_face->normal;
		faces_level[0].push_back(new_face);
	}
	
	for(int i = 0; i < (int)vertices.size(); i++){
		vertices_level[0][i]->normal = glm::normalize(vertices_level[0][i]->normal);
		vertices_level[0][i]->tex_coords = glm::vec2((atan2(vertices_level[0][i]->coords.x, vertices_level[0][i]->coords.z)+3.1415926)/(2*3.1415926), vertices_level[0][i]->coords.y/y_max);
	}
	half_edge(0);
	catmull(1);
	catmull(2);
	catmull(3);
	loadTexture("texture.bmp");
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, mode);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, look_at.x, look_at.y, look_at.z, 0.f, 1.f, 0.f);
	glColor3f(0.5, 0.5, 0.5);
	glBindTexture(GL_TEXTURE_2D, tex);
	
	if(mode == GL_FILL){
		glEnable(GL_TEXTURE_2D);
	}
	else{
		glDisable(GL_TEXTURE_2D);
	}
	for(int i=0; i < (int)faces_level[LEVEL].size(); i++){
		int pos[4];
		glm::vec3 n[4];
		glm::vec3 v[4];
		glm::vec2 texc[4];
		for(int j = 0; j <= 3; j++){
			pos[j] = faces_level[LEVEL][i]->adj[j];
			n[j] = vertices_level[LEVEL][pos[j]]->normal;
			v[j] = vertices_level[LEVEL][pos[j]]->coords;
			texc[j] = vertices_level[LEVEL][pos[j]]->tex_coords;
		}
		glBegin(GL_QUADS);
			for(int j = 0; j <= 3; j++){
				glTexCoord2f(texc[j].x, texc[j].y);
				glNormal3f(n[j].x, n[j].y, n[j].z);  
				glVertex3f(v[j].x, v[j].y, v[j].z);
			}
		glEnd();
	}
	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h){
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0,(float)w/h,0.01,10.0);
	glMatrixMode (GL_MODELVIEW);
}

float B1_dir(float time){
	return (1.0f-time)*(1.0f-time)*(1.0f-time);
}
float B2_dir(float time){
	return 3.0f*(1.0f-time)*(1.0f-time)*time;
}
float B3_dir(float time){
	return 3.0f*(1.0f-time)*time*time;
}
float B4_dir(float time){
	return time*time*time;
}

void timer(int v){
	time += 0.0016;
	glm::vec3 P0 = glm::vec3(-6.0f, 0.0f, 6.0f);
	glm::vec3 P1 = glm::vec3(6.0f, 0.0f, 6.0f);
	glm::vec3 P2 = glm::vec3(6.0f, 0.0f, -6.0f);
	glm::vec3 P3 = glm::vec3(-6.f, 0.0f, -6.0f);
	eye = P0*B1_dir(abs(sin(2*3.1415926*time)))+P1*B2_dir(abs(sin(2*3.1415926*time)))+P2*B3_dir(abs(sin(2*3.1415926*time)))+P3*B4_dir(abs(sin(2*3.1415926*time)));
	
	glutPostRedisplay();
	glutTimerFunc(1000/nFPS, timer, v);
}

void keyboard(unsigned char key, int x, int y){
	switch(key){
		case 27:
			for(int j = 0; j < 4; j++){
				for(int i=0; i < (int)faces_level[j].size();i++){
					Edge* e = faces_level[j][i]->start_edge;
					free(e->next->next->next);free(e->next->next);free(e->next);free(e);
				}
				for(int i=0; i < (int)vertices_level[j].size(); i++){
					free(vertices_level[j][i]);
				}
				faces_level[j].clear();
				vertices_level[j].clear();
			}
			exit(0);
			break;
		case 'a':
			LEVEL++;
			LEVEL %= 4;
			break;
		default:
			break;
	}
}

void mouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		
	}
	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		
	}
}

void motion(int x, int y){
	
}

int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("mp4");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(100,timer,nFPS);
	glutMainLoop();
	return 0;
}

