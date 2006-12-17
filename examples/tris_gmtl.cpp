// CMPS 415/515, University of Louisiana at Lafayette
//
// tris.cpp : see the class handout on OpenGL for
//            documentation of this example program
//
// NOTE: No permission is given for distribution beyond
//       the 415/515 class, of this file or any derivative works.
//
// NOTE: This version adds rotation of the right triangle using GMTL's Matrix 
//       class.

#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <gmtl/Matrix.h>
#include <gmtl/Output.h>
#include <gmtl/Generate.h>
#include <iostream>

using namespace std;

#define W_WIDTH  600
#define W_HEIGHT 600

gmtl::Matrix44f right_rotate;		//Initialized to identity, we will change it in init()


void setlights(void)
{
  GLfloat position[] = { 3.0, 3.0, 3.0, 0.0 },
             white[] = { 1.0, 1.0, 1.0, 1.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  white);
}

void drawobjects(void)
{
  float color_diffuse[]  = { 0.8, 0.5, 0.5, 1.0 },
                   p0[3] = {0.75, -0.75, 0.5},
                   p1[3] = {0.75,  0.75, 0.5},
                   p2[3] = {0.0,   0.0,  0.5};
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color_diffuse);


  glEnable(GL_LIGHTING);

  //We're going to rotate the right triangle.
  //First, save our current matrix.
  glPushMatrix();
  //Now multiply our rotation onto the modelview stack
  //OpenGL expects a pointer to a 16 element array of floats. 
  //That's the mData member for a GMTL Matrix.
  glMultMatrixf(right_rotate.mData);

  glBegin(GL_TRIANGLES);
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3fv(p0);
	glVertex3fv(p1);
	glVertex3fv(p2);
  glEnd();

  //Restore our saved Matrix so that the left triangle is not drawn rotated.
  glPopMatrix();

  

  glDisable(GL_LIGHTING);
  glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-0.75, -0.75, 0.0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(-0.75, 0.75, 0.0);
  glEnd();
  
  

}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0, 1.0,
	        0.0, 0.0, 0.0, 
	 	    0.0, 1.0, 0.0);

  setlights(); 
 
  drawobjects();

  glFlush();
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void init(void)
{
  glClearColor(0.7, 0.7, 0.7, 0.0); 
  glEnable(GL_DEPTH_TEST); 
  glEnable(GL_LIGHT0);
  glShadeModel(GL_SMOOTH);

  //To get a 45 degree rotation about Z our matrix should look like:
  //| 0.707107 -0.707107 0.000000 0.000000 |
  //| 0.707107 0.707107 0.000000 0.000000 |
  //| 0.000000 0.000000 1.000000 0.000000 |
  //| 0.000000 0.000000 0.000000 1.000000 |

  //Right now right_rotate is identity, so here's how we'll change it:


  right_rotate[0][0] = 0.707107;
  right_rotate[0][1] = -0.707107;
  right_rotate[1][0] = 0.707107;
  right_rotate[1][1] = 0.707107;

  cout << "Rotation: " << endl << fixed << right_rotate << endl;
	
  //Normally you don't create a matrix this way with GMTL. 
  //But since we did we have to tell GMTL what sort of Matrix it is,
  //in case we want to invert it later.

  right_rotate.mState = gmtl::Matrix44f::ORTHOGONAL;


}

int main(int argc, char **argv)
{ 
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(W_WIDTH, W_HEIGHT);
  
  glutCreateWindow("CMPS 415/515");

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  
  init();

  glutMainLoop();

  return 0;
}
