//*******************************************************
//*******************************************************
//*****************CMPS 415 Assignment 5*****************
//********************Particle System********************
//*********************William Zhang*********************
//************************wxz1067************************
//***********************Fall 2006***********************
//*******************************************************
//*******************************************************
// Compiled with GCC 4.0 under Mac OSX with XCode
// Makes use of glui
// 
//*******************************************************
// Instructions******************************************
//*******************************************************
// Start it and you can modify most values with the 
// included gui. For those controls you cannot use the gui 
// you can click on the window to change.  This is used
// for controlling the placement of the elements.  The
// Attractor, Emitter, and Obstacle can be controlled.
// To toggle the placement of each one, press the first
// letter of the element.  So, you only have a choice
// between e, a, and o.


using namespace std;
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <glui.h>
#include <gmtl/Matrix.h>
#include <gmtl/Math.h>
#include <gmtl/gmtl.h>
#include <string>


#define W_WIDTH  640
#define W_HEIGHT 480

#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP   3
#  define GLUT_WHEEL_DOWN 4
#endif

bool which = false;
int lines = false;

struct color{
	float r;
	float g;
	float b;
};

struct particle{
	gmtl::Point3f p;
	gmtl::Vec3f vec;
	color col;
	int life;
	int lifemax;
	float alpha;
};

unsigned char movemode = 'a';

float delta = .25;
float kattract = .25;
float kwind = .05;
float krestitution = .5;

particle particles[20000];
int numPart = 500;
gmtl::Spheref attractSphere, obstacle;
gmtl::Point3f emitter;
gmtl::Vec3f wind;

float partmass;
int main_window;
GLuint texture = 0;

//Given Load PPM function
void LoadPPM(char *fname, unsigned int *w, unsigned int *h, unsigned char **data, int mallocflag)
{
	FILE *fp;
	char P, num;
	int max;
	char s[1000];

	if (!(fp = fopen(fname, "rb")))
	{
		perror("cannot open image file\n");
		exit(0);
	}
	fscanf(fp, "%c%c\n", &P, &num);

	if ((P != 'P') || (num != '6'))
	{
		perror("unknown file format for image\n");
		exit(0);
	}
	do
	{
		fgets(s, 999, fp);
	}while (s[0] == '#');
	sscanf(s, "%d%d", w, h);
	fgets(s, 999, fp);
	sscanf(s, "%d", &max);
	if (mallocflag)
		if (!(*data = (unsigned char *)malloc(*w * *h * 3)))
		{
			perror("cannot allocate memory for image data\n");
			exit(0);
		}
	fread(*data, 3, *w * *h, fp);
	fclose(fp);
}

//Load the texture and alpha map
void loadtexture(){
	static unsigned int texwidth, texheight;
  	static unsigned char *imagedata;
	
	LoadPPM("particle.ppm", &texwidth, &texheight, &imagedata, 1);
  	
	//new array allowing position for the alpha channel
	static unsigned char *texdata = new unsigned char[4*texwidth*texheight];
	
	//convert texture to texture+alpha
	for(int i=0;i<texheight*texwidth;i++){
		texdata[i*4] = imagedata[i*3];
		texdata[i*4+1] = imagedata[i*3+1];
		texdata[i*4+2] = imagedata[i*3+2];
		texdata[i*4+3] = imagedata[i*3];
	}

	glGenTextures(1, &texture); 
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texwidth, texheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
}

//Given Fastgauss algorithm
float fastgauss(float mean, float std)
{
#define RESOLUTION 2500
	static float lookup[RESOLUTION+1];

#define itblmax    20    
	/* length - 1 of table describing F inverse */
#define didu    40.0    
	/* delta table position / delta ind. variable           itblmax / 0.5 */

	static float tbl[] =
	{0.00000E+00, 6.27500E-02, 1.25641E-01, 1.89000E-01,
		2.53333E-01, 3.18684E-01, 3.85405E-01, 4.53889E-01,
		5.24412E-01, 5.97647E-01, 6.74375E-01, 7.55333E-01,
		8.41482E-01, 9.34615E-01, 1.03652E+00, 1.15048E+00,
		1.28167E+00, 1.43933E+00, 1.64500E+00, 1.96000E+00,
		3.87000E+00};

	static int hereb4;
	static struct timeval tv;

	float u, di, delta, result;
	int i, index, minus = 1;

	if (!hereb4)
	{
		for (i = 0; i <= RESOLUTION; i++)
		{
			if ((u = i/(float)RESOLUTION) > 0.5) 
			{
				minus = 0;
				u -= 0.5;
			}

			/* interpolate gaussian random number using table */

			index = (int)(di = (didu * u));
			di -= (float)index;
			delta =  tbl[index] + (tbl[index + 1] - tbl[index]) * di;
			lookup[i] = (minus ? -delta : delta);
		} 

		//gettimeofday(&tv, NULL);
		//srand((unsigned int)tv.tv_usec);
		srand (time (NULL));
		
		hereb4 = 1;
	}

	i = rand()/(RAND_MAX/RESOLUTION)+1;
	if (i > RESOLUTION)
		i = RESOLUTION;
	return(mean + std * lookup[i]);
}

//Calculate the distance between two points
double distance3d(gmtl::Point3f a, gmtl::Point3f b){
	float f = (a[0]-b[0])*(a[0]-b[0]);
	f = f + (a[1]-b[1])*(a[1]-b[1]);
	f = f + (a[2]-b[2])*(a[2]-b[2]);
	return sqrt((double)(f));
}

//Generate a new Particle
particle getParticle(){
	particle part;

	part.p = emitter;
	part.col.r = fastgauss(.5, .18);
	part.col.g = fastgauss(.5, .18);
	part.col.b = fastgauss(.5, .18);
	part.alpha = 1;

	part.vec[0] = 5*fastgauss(0, .04);//(float)(rand()%150)/(rand()%100000)*getSign();
	part.vec[1] = 5*fastgauss(0, .04);//(float)(rand()%150)/(rand()%100000)*getSign();
	part.vec[2] = 5*fastgauss(0, .03);//(float)(rand()%150)/(rand()%100000)*getSign();

	part.life=(int)(5000/delta*fastgauss(1.0,.4));

	return part;
}

//Initialize the Particles
void initParticles(){
	for(int i=0; i<numPart; i++){
		particles[i]=getParticle();
	}
}

//Display the Particle
void displayParticles(){
	gmtl::Point3f tail;
	for(int i=0; i<numPart; i++){
		float x = particles[i].p[0];
		float y = particles[i].p[1];
		float z = particles[i].p[2];
		//Draw Lines
		if(lines){
			//Calculate the point of the tail
			tail = particles[i].p-particles[i].vec;
			//Draw the Lines
			glBegin(GL_LINES);
			glColor4f(particles[i].col.r,particles[i].col.g,particles[i].col.b, particles[i].alpha);
			glVertex3f(x,y,z);
			glVertex3f(tail[0],tail[1],tail[2]);
			glEnd();
		}
		//Draw Textured Billboards
		else{
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBegin(GL_TRIANGLE_STRIP);
			  glColor4f(particles[i].col.r,particles[i].col.g,particles[i].col.b, particles[i].alpha);
			  glTexCoord2f(1.0, 1.0); glVertex3f(x+.7f,y+.7f,z); // Top Right
			  glTexCoord2f(0.0, 1.0); glVertex3f(x-.7f,y+.7f,z); // Top Left
			  glTexCoord2f(1.0, 0.0); glVertex3f(x+.7f,y-.7f,z); // Bottom Right
			  glTexCoord2f(0.0, 0.0); glVertex3f(x-.7f,y-.7f,z); // Bottom Left
			glEnd();
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		}

	}
}
float angle = 0;

//Main display function
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Draw the Attractor
	gmtl::Point3f temppoint = attractSphere.getCenter();
	glTranslatef(temppoint[0], temppoint[1], temppoint[2]);
	glColor3f(.5109f,.1412f,.8824f);
	glutSolidSphere(attractSphere.getRadius(),18,18);
	glTranslatef(-temppoint[0], -temppoint[1], -temppoint[2]);	

	//Draw the Obstacle
	temppoint = obstacle.getCenter();
	glTranslatef(temppoint[0], temppoint[1], temppoint[2]);
	glColor3f(1.0f, 0.0f,0.0f);
	glutSolidSphere(obstacle.getRadius(),18,18);
	glTranslatef(-temppoint[0], -temppoint[1], -temppoint[2]);

	//Draw the Particles
	displayParticles();
	//Swap Double Buffer thingie
	glutSwapBuffers();
}

// Some initialization stuff
void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0); 
	glEnable(GL_DEPTH_TEST); 
}

//Return the force of the Attractor on the particle
gmtl::Vec3f getAttract(particle part){
	float dist = distance3d(part.p, attractSphere.getCenter());
	gmtl::Vec3f retVec;
	retVec = attractSphere.getCenter() - part.p;
	gmtl::normalize(retVec);
	retVec = kattract*partmass/dist*retVec;

	return retVec;
}

// Return the force of the Wind Vector
gmtl::Vec3f getWind(particle part){
	return (-1*kwind*(part.vec-wind));
}


//Update the Particle with a new force and new equation
void updatePart(){
	//Check for the main window;
	if ( glutGetWindow() != main_window ) 
		glutSetWindow(main_window); 
	particle tempPart;
	gmtl::Vec3f attractor, windforce;
	gmtl::Vec3f n, newvelocnorm, newveloc;
	gmtl::Point3f center;
	double distance;

	//For each particle update;
	for(int i=0; i<numPart; i++){
		//Copy Over old particle info
		tempPart.col = particles[i].col; //same color
		tempPart.life = particles[i].life - 1; //decrement life
		tempPart.alpha = particles[i].life / 20000; //new alpha based on life

		// Update the position
		tempPart.p[0] = particles[i].p[0] + delta*particles[i].vec[0];
		tempPart.p[1] = particles[i].p[1] + delta*particles[i].vec[1];
		tempPart.p[2] = particles[i].p[2] + delta*particles[i].vec[2];

		//Get the forces of wind and attractor
		attractor = getAttract(particles[i])/partmass;
		windforce = getWind(particles[i])/partmass;
		
		//Combine forces and old velocity
		tempPart.vec[0] = particles[i].vec[0] + delta*(attractor[0] + windforce[0]);
		tempPart.vec[1] = particles[i].vec[1] + delta*(attractor[1] + windforce[1]);
		tempPart.vec[2] = particles[i].vec[2] + delta*(attractor[2] + windforce[2]);
		
		//Collision Detection for Obstacle
		center = obstacle.getCenter();
		distance = distance3d(tempPart.p, center);
		if( distance < obstacle.getRadius()){
			n = tempPart.p - center;
			gmtl::normalize(n);
			if(gmtl::dot(n, tempPart.vec)){
				newvelocnorm = gmtl::dot(n,tempPart.vec) * n;
				newveloc = tempPart.vec - newvelocnorm;
				tempPart.vec = newveloc - (krestitution * newvelocnorm);
			}
		}

		//Collision Detection for Attractor
		distance = distance3d(tempPart.p, attractSphere.getCenter());
		if( distance < attractSphere.getRadius()){
			tempPart = getParticle();
		}

		//Set the particle to the new particle
		particles[i] = tempPart;
		if(particles[i].life<=0){
			particles[i] = getParticle();
		}
	}
	glutPostRedisplay();
}


void move_element(float x, float y){
	gmtl::Point3f setpoint = gmtl::Point3f(x,y,0);
	switch(movemode){
		case 'e':
			setpoint[2] = emitter[2];
			emitter = setpoint;
			break;
		case 'a':
			setpoint[2] = attractSphere.mCenter[2];
			attractSphere.setCenter(setpoint);
			break;
		case 'o':
			setpoint[2] = obstacle.mCenter[2];
			obstacle.setCenter(setpoint);
			break;
	}
}


void mousebuttonhandler(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		move_element(-25 + (x)*(50)/640, 25 - (y)*(50)/480);
	}
}

void processNormalKeys(unsigned char key, int x, int y){
	if(key == 'e' || key == 'a' || key == 'o'){
		movemode = key;
	}
}

void reshape (int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	GLUI_Master.auto_set_viewport();
}

void particlesysteminit(){
	obstacle = gmtl::Spheref(gmtl::Point3f(0, 0, 1), 8);
	attractSphere = gmtl::Spheref(gmtl::Point3f(13, 13, 1), 1.5);
	emitter = gmtl::Point3f(-20,-20,0);
	wind = gmtl::Vec3f(0, .1, 0);
	partmass = 3;
	initParticles();
}
void gluiinit(){
	GLUI *glui = GLUI_Master.create_glui("Control Panel", 0);

	GLUI_Panel *attractpanel = glui->add_panel("Attractor");
	glui->add_edittext_to_panel(attractpanel, "Z:",GLUI_EDITTEXT_FLOAT, &attractSphere.mCenter[2]);
	glui->add_edittext_to_panel(attractpanel, "Radius:",GLUI_EDITTEXT_FLOAT, &attractSphere.mRadius);
	glui->add_column(true);
	GLUI_Panel *obstaclepanel = glui->add_panel("Obstacle");
	glui->add_edittext_to_panel(obstaclepanel, "Z:",GLUI_EDITTEXT_FLOAT, &obstacle.mCenter[2]);
	glui->add_edittext_to_panel(obstaclepanel, "Radius:",GLUI_EDITTEXT_FLOAT, &obstacle.mRadius);
	glui->add_column(true);
	GLUI_Panel *windpanel = glui->add_panel("Wind Vector");
	GLUI_Spinner *xwind = glui->add_spinner_to_panel(windpanel, "X: ", GLUI_SPINNER_FLOAT, &wind[0]);
	xwind->set_float_limits(-3, 3, GLUI_LIMIT_CLAMP);
	GLUI_Spinner *ywind = glui->add_spinner_to_panel(windpanel, "y: ", GLUI_SPINNER_FLOAT, &wind[1]);
	ywind->set_float_limits(-3, 3, GLUI_LIMIT_CLAMP);
	GLUI_Spinner *zwind = glui->add_spinner_to_panel(windpanel, "Z: ", GLUI_SPINNER_FLOAT, &wind[2]);
	zwind->set_float_limits(-3, 3, GLUI_LIMIT_CLAMP);
	glui->add_column(true);

	GLUI_Spinner *attrspinner = glui->add_spinner("kattract: ", GLUI_SPINNER_FLOAT, &kattract);
	attrspinner->set_float_limits(0, 5, GLUI_LIMIT_CLAMP);
	GLUI_Spinner *windspinner = glui->add_spinner("kwind: ", GLUI_SPINNER_FLOAT, &kwind);
	windspinner->set_float_limits(0, 5, GLUI_LIMIT_CLAMP);
	GLUI_Spinner *deltaspinner = glui->add_spinner("delta: ", GLUI_SPINNER_FLOAT, &delta);
	deltaspinner->set_float_limits(0, 5, GLUI_LIMIT_CLAMP);
	glui->add_column(true);
	GLUI_Spinner *restspinner = glui->add_spinner("restituion: ", GLUI_SPINNER_FLOAT, &krestitution);
	restspinner->set_float_limits(0, 1, GLUI_LIMIT_CLAMP);

	glui->add_checkbox("Lines", &lines);

	glui->add_button( "Quit", 0,(GLUI_Update_CB)exit ); 	
	glui->set_main_gfx_window(main_window);
	GLUI_Master.set_glutIdleFunc(updatePart);

}


int main(int argc, char **argv)
{ 

	glutInit(&argc, argv);
	srand ( time(NULL) );
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(W_WIDTH, W_HEIGHT);
	main_window = glutCreateWindow("William Zhang CMPS415 Assignment 5");
	init();
	particlesysteminit();
	gluiinit();
	loadtexture();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-25, 25, -25, 25, -10, 25);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 25.0f, 0.0, 0.0, 0.0, 0.0, 25.0, 0.0);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mousebuttonhandler);
	glutKeyboardFunc(processNormalKeys);

	glutMainLoop();
	return 0;
}
