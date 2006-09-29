// Class    : CMPS 415
// Section  : 001
// Problem  : Programming Assignment #2
// Name     : McKelvy, James Markus
// CLID     : jmm0468

//  --* Certification of Authenticity *--
//  I certify that this assignment is entirely my own work.

// Using Dev-C++ Version 4.9.9.2
// To compile this program, open assignment2.dev (project file),
//  the files needed:
//   - assignment2.cpp 
//  should be automatically added to the project.

// Press f9 to compile & run

//[from Chris Best's explanation, needed for dev-c++]
//This was necessary for me to compile scalar multiplication on Mac OS X
//Try commenting it out on your compiler. You should get better performance 
//without it.
#define GMTL_NO_METAPROG

#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>

//First include all of the headers we'll need for our data types
#include <gmtl/Matrix.h>		//Matrices
#include <gmtl/MatrixOps.h>		//Matrix Operations
#include <gmtl/Quat.h>			//Quaternions
#include <gmtl/EulerAngle.h>	//Euler Angles
#include <gmtl/Generate.h>		//makeRot/makeTrans/etc.

#define W_WIDTH  800
#define W_HEIGHT 600

using namespace std;

const int XAXIS = 1;
const int YAXIS = 2;
const int ZAXIS = 3;
const float tranlationAmount = 0.01;
// degrees
const float rotationAmount = 10.0;
const float scaleAmount = 0.01;
// these are for the gluLookAt function
// to help describe what is going on
const float eye_x = 0.0;
const float eye_y = 0.0;
const float eye_z = 2.0;
const float targ_x = 0.0;
const float targ_y = 0.0;
const float targ_z = 0.0;
const float updir_x = 0.0;
const float updir_y = 1.0;
const float updir_z = 0.0;    

gmtl::Quatf cube;
float translate[] = {0.0, 0.0, 0.0};
float scale = 0.2;

// this takes a quaternion, a degree, and an axis of rotation
// a quaternion is then created and multiplied by the 'orig' quaternion
// such that the rotation is as if it is about the local axis
// the allowed values for 'axis' is as follows:
// axis 1 is x
// axis 2 is y
// axis 3 is z
void multiplyQuaternionBy(gmtl::Quatf &orig, float degAngle, unsigned int axis){
     gmtl::Quatf mult;
     // w is the scalar part of the quaternion, as seen below it is calculated
     // by the formula w = cos(theta/2)
     float w = gmtl::Math::cos(gmtl::Math::deg2Rad(degAngle) / 2);
     float x, y, z;
     x = y = z = 0;

     // depending on the parameter, an axis of rotation will be 'figured in' to
     // the quaternion
     if(axis == 1){
          x = gmtl::Math::sin(gmtl::Math::deg2Rad(degAngle) / 2);
     }
     else if(axis == 2){
          y = gmtl::Math::sin(gmtl::Math::deg2Rad(degAngle) / 2);
     }
     else if(axis == 3){
          z = gmtl::Math::sin(gmtl::Math::deg2Rad(degAngle) / 2);
     }
     
     // now set the quaternion based upon our parameters and calculations
     mult.set(x, y, z, w);
     
     // this multiplies the original quaternion by a rotation quaternion.
     // the order of multiplication is important.
     // in this case, the order of multiplication results in a rotation about
     // a local coordinate system.
     orig = orig * mult;     
}

// this draws a helicopter with a specified scale (typically
// a float somewhere between 0.0 and 1.0)
// xpos, ypos, zpos are coordinates for the local system
// with respect to the world system
// these parameters are pretty much useless since you can make a call to 
// glTranslate, but these were included earlier on and left within the code
void drawobj(float scale){
    
   float xpos = 0.0; 
   float ypos = 0.0; 
   float zpos = 0.0;
   
   // we need to offset so that the center of the coordinate system is in
   // the center of the helicopter cockpit
   xpos -= 0.5*scale;
   ypos -= 0.5*scale;
   zpos -= 0.5*scale;  
   
   // strip around cockpit
   glBegin(GL_QUAD_STRIP);
   
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);    
    glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
	glVertex3f(scale+xpos, 0.0+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);    
    glVertex3f(0.0+xpos, 0.0+ypos, scale+zpos);
    glVertex3f(scale+xpos, 0.0+ypos, scale+zpos);
    
    glColor3f(255.0/255.0, 255.0/255.0, 255.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, scale+zpos);
    glVertex3f(scale+xpos, scale+ypos, scale+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, 0.0+zpos);
    glVertex3f(scale+xpos, scale+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
    glVertex3f(scale+xpos, 0.0+ypos, 0.0+zpos);
    
  glEnd();  

  // cockpit front and back
  glBegin(GL_QUADS);
  // front side
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(scale+xpos, 0.0+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    glVertex3f(scale+xpos, scale+ypos, 0.0+zpos);
    
    glColor3f(255.0/255.0, 255.0/255.0, 255.0/255.0);
    glVertex3f(scale+xpos, scale+ypos, scale+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    glVertex3f(scale+xpos, 0.0+ypos, scale+zpos);
  // back side    
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    glVertex3f(0.0+xpos, 0.0+ypos, scale+zpos);
    
    glColor3f(255.0/255.0, 255.0/255.0, 255.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, scale+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, 0.0+zpos);

  glEnd();
 
   // reset the offset so that the axes are positioned properly
   xpos += 0.5*scale;
   ypos += 0.5*scale;
   zpos += 0.5*scale;  
   
   float length = 1.01;
  /* 
  // draw a local coordinate system 
  glBegin(GL_LINES);
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    
    // draw x axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(scale*length+xpos, ypos, zpos);
    
    // draw y axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(xpos, scale*length+ypos, zpos);
    
    // draw z axis
    glVertex3f(xpos, ypos, 0);
    glVertex3f(xpos, ypos, scale*length+zpos);
    
    // draw x label
    glVertex3f(scale*length+xpos, .3*scale+ypos, .2*scale+zpos);
    glVertex3f(scale*length+xpos, -.3*scale+ypos, -.2*scale+zpos);
    glVertex3f(scale*length+xpos, .3*scale+ypos, -.2*scale+zpos);
    glVertex3f(scale*length+xpos, -.3*scale+ypos, .2*scale+zpos);    
    
    // draw y label
    glVertex3f(xpos, scale*length+ypos, zpos);
    glVertex3f(.2*scale+xpos, scale*length+ypos, .3*scale+zpos);
    glVertex3f(xpos, scale*length+ypos, zpos);
    glVertex3f(-.2*scale+xpos, scale*length+ypos, .3*scale+zpos);
    glVertex3f(xpos, scale*length+ypos, zpos);
    glVertex3f(xpos, scale*length+ypos, -.5*scale+zpos);    
  
    // draw z label
    glVertex3f(-.3*scale+xpos, .3*scale+ypos, scale*length+zpos);
    glVertex3f(.3*scale+xpos, .3*scale+ypos, scale*length+zpos);
    glVertex3f(-.3*scale+xpos, -.3*scale+ypos, scale*length+zpos);
    glVertex3f(.3*scale+xpos, -.3*scale+ypos, scale*length+zpos);
    glVertex3f(.3*scale+xpos, .3*scale+ypos, scale*length+zpos);
    glVertex3f(-.3*scale+xpos, -.3*scale+ypos, scale*length+zpos);    
    
  glEnd();   
  */
}

// here is the heart of the program. The differences in the way the helicopters
// rotate can be seen implemented here
// the world axes are drawn, along with each helicopter in its correct translation
// and rotation from the origin
void drawobjects(void)
{
   // save modelview state
   glPushMatrix();   
   // for quaternion values:x,y,z are vector parts, w is scalar part
   float x,y,z,w;
   // for converting to angle axis
   float theta, kx, ky, kz, t;   
   // get the scalar and vector components of the quaternion
   cube.get(x,y,z,w);
   // get an angle in degrees from the quaternion in angle-axis form
   // theta will be in degrees
   theta = gmtl::Math::rad2Deg(2 * gmtl::Math::aCos(w));
   // t will be used to calculate kx,ky,kz
   t = gmtl::Math::sqrt(1 - (w*w));
   // calculate kx, ky, kz from the quaternion to get axes
   kx = x/t;
   ky = y/t;
   kz = z/t;   
   
   // translate then rotate
   glTranslatef(translate[0], translate[1], translate[2]);
   
   // if the angle is 0, the helicopter will not appear, so this is a check
   if(int(theta)%360 != 0){
      glRotatef(theta, kx, ky, kz);
   }
   // param1 - scale
   // param2-4 - coordinates relative to world axes
   drawobj(scale);
   // restore state of modelview
   glPopMatrix();      
}

// tie in to keypresses and do something with them
// such as....rotate the helicopters
// keys 1-6 are used for rotation, see comments below for details
void handleKeys(unsigned char key, int x, int y){

     // positive x rotation
     if(key == '1')
            multiplyQuaternionBy(cube, rotationAmount, XAXIS);           
     // negative x rotation
     if(key == '2')
            multiplyQuaternionBy(cube, -rotationAmount, XAXIS);          
     // positive y rotation
     if(key == '3')
            multiplyQuaternionBy(cube, rotationAmount, YAXIS);           
     // negative y rotation
     if(key == '4')
            multiplyQuaternionBy(cube, -rotationAmount, YAXIS);           
     // positive z rotation
     if(key == '5')
            multiplyQuaternionBy(cube, rotationAmount, ZAXIS);           
     // negative z rotation
     if(key == '6')
            multiplyQuaternionBy(cube, -rotationAmount, ZAXIS); 
     if(key == 'd')
            translate[0] += tranlationAmount;
     if(key == 'a')
            translate[0] += -tranlationAmount;
     if(key == 'w')       
            translate[1] += tranlationAmount;     
     if(key == 's')
            translate[1] += -tranlationAmount;
     if(key == 'z'){
            translate[2] += tranlationAmount;     
            scale += scaleAmount;
     }
     if(key == 'x'){
            translate[2] += -tranlationAmount;  
            if(scale >= (0.0 + scaleAmount))   
                scale += -scaleAmount;
     }
                      
     // cause a redraw of the helicopters
     glutPostRedisplay();
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // box coordinate range
  // minX maxX minY maxY minZ maxZ
  glOrtho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);
  
  glMatrixMode(GL_MODELVIEW);
  // identity matrix 
  glLoadIdentity();
  
  // this sets up the camera and is multiplied by the GL_MODELVIEW matrix
  //gluLookAt(eye_x, eye_y, eye_z, targ_x, targ_y, targ_z, updir_x, updir_y, updir_z);

  // draw some helicopters
  drawobjects(); 
  
  glutSwapBuffers();
  //glFlush();
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void init(void)
{
  // background is white
  glClearColor(1,1,1,0);
  glEnable(GL_DEPTH_TEST); 
  glEnable(GL_LIGHT0);
  glShadeModel(GL_SMOOTH);
}

int main(int argc, char **argv)
{ 
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(W_WIDTH, W_HEIGHT);
  
  glutCreateWindow("SUPER DUPER CRAP PROGRAM");

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  // my key press handling function
  glutKeyboardFunc(handleKeys);
  
  init();

  // pass the torch to glut
  glutMainLoop();

  return 0;
}
