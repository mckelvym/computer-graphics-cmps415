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
// 1 is positive x
// 2 is negative x
// 3 is positive y
// 4 is negative y
// 5 is positive z
// 6 is negative z

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

// global variables for each helicopter based upon the assignment specifications
// the first is stored with euler angles
float helicopter1_data[3];
// the second is stored with a homogeneous transform matrix
gmtl::Matrix44f helicopter2_data;
// the third is also stored with a homogeneous transform matrix
gmtl::Matrix44f helicopter3_data;
// the fourth is stored as a quaternion
gmtl::Quatf helicopter4_data(0,0,0,1);

// this is a helper function to initialize the state of the helicopters
// that are being represented by euler angles. It gives the initial rotation
// a 0.0 for each angle
void initialize_helicopters(){
     for(int i = 0; i < 3; i++){
       helicopter1_data[i] = 0.0;
     }
}

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

void translate4x4world(gmtl::Matrix44f &A, float dx, float dy, float dz){
     gmtl::Matrix44f trans;
     
     trans[0][3] = dx;
     trans[1][3] = dy;
     trans[2][3] = dz;
     
     A = trans * A;
}

void translate4x4local(gmtl::Matrix44f &A, float dx, float dy, float dz){
     gmtl::Matrix44f trans;
     
     trans[0][3] = dx;
     trans[1][3] = dy;
     trans[2][3] = dz;
     
     A = A * trans;
}

void rotate4x4world(gmtl::Matrix44f &A, float degX, float degY, float degZ){
     gmtl::Matrix44f rot;
     float radians = 0.0;
     
     // this cannot be used, but results in the same matrix
     //rot = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(gmtl::Math::deg2Rad(degX), gmtl::Math::deg2Rad(degY) , gmtl::Math::deg2Rad(degZ)));
     
     // only one of the degree values should be non-zero     
     // set the rotation matrix up for a rotation about the x axis     
     if(degX != 0){
         radians = gmtl::Math::deg2Rad(degX);
         rot[1][1] = gmtl::Math::cos(radians);
         rot[1][2] = -gmtl::Math::sin(radians);
         rot[2][1] = gmtl::Math::sin(radians);
         rot[2][2] = gmtl::Math::cos(radians);             
     }
     // set the rotation matrix up for a rotation about the y axis
     else if(degY != 0){
         radians = gmtl::Math::deg2Rad(degY);     
         rot[0][0] = gmtl::Math::cos(radians);
         rot[0][2] = gmtl::Math::sin(radians);
         rot[2][0] = -gmtl::Math::sin(radians);
         rot[2][2] = gmtl::Math::cos(radians);
     }
     // set the rotation matrix up for a rotation about the x axis
     else if(degZ != 0){
         radians = gmtl::Math::deg2Rad(degZ);
         rot[0][0] = gmtl::Math::cos(radians);
         rot[0][1] = -gmtl::Math::sin(radians);
         rot[1][0] = gmtl::Math::sin(radians);
         rot[1][1] = gmtl::Math::cos(radians);
     }               
     
     A = rot * A;
}

void rotate4x4local(gmtl::Matrix44f &A, float degX, float degY, float degZ){
     gmtl::Matrix44f rot;
     float radians = 0.0;
     
     // this cannot be used, but results in the same matrix
     //rot = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(gmtl::Math::deg2Rad(degX), gmtl::Math::deg2Rad(degY) , gmtl::Math::deg2Rad(degZ)));
     
     // only one of the degree values should be non-zero     
     // set the rotation matrix up for a rotation about the x axis     
     if(degX != 0){
         radians = gmtl::Math::deg2Rad(degX);
         rot[1][1] = gmtl::Math::cos(radians);
         rot[1][2] = -gmtl::Math::sin(radians);
         rot[2][1] = gmtl::Math::sin(radians);
         rot[2][2] = gmtl::Math::cos(radians);             
     }
     // set the rotation matrix up for a rotation about the y axis
     else if(degY != 0){
         radians = gmtl::Math::deg2Rad(degY);     
         rot[0][0] = gmtl::Math::cos(radians);
         rot[0][2] = gmtl::Math::sin(radians);
         rot[2][0] = -gmtl::Math::sin(radians);
         rot[2][2] = gmtl::Math::cos(radians);
     }
     // set the rotation matrix up for a rotation about the x axis
     else if(degZ != 0){
         radians = gmtl::Math::deg2Rad(degZ);
         rot[0][0] = gmtl::Math::cos(radians);
         rot[0][1] = -gmtl::Math::sin(radians);
         rot[1][0] = gmtl::Math::sin(radians);
         rot[1][1] = gmtl::Math::cos(radians);
    }               

     A = A * rot;          
}

// this draws a helicopter with a specified scale (typically
// a float somewhere between 0.0 and 1.0)
// xpos, ypos, zpos are coordinates for the local system
// with respect to the world system
// these parameters are pretty much useless since you can make a call to 
// glTranslate, but these were included earlier on and left within the code
void drawchopper(float scale, float xpos, float ypos, float zpos){
    
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
    glVertex3f(0.0+xpos, 0.0+ypos, scale+zpos);
    glVertex3f(scale+xpos, 0.0+ypos, scale+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, scale+zpos);
    glVertex3f(scale+xpos, scale+ypos, scale+zpos);
    
    glColor3f(100.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 100.0/255.0, 0.0/255.0);
    glVertex3f(scale+xpos, scale+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
    glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
    glVertex3f(scale+xpos, 0.0+ypos, 0.0+zpos);
    
  glEnd();
  
  // draw window on right side of cockpit
  glBegin(GL_TRIANGLES);
    glColor3f(250.0/255.0, 250.0/255.0, 50.0/255.0);
    glVertex3f(.5*scale+xpos, .5*scale+ypos, 1.0005*scale+zpos); 
    
    glColor3f(250.0/255.0, 250.0/255.0, 150.0/255.0);
    glVertex3f(.5*scale+xpos, .8*scale+ypos, 1.0005*scale+zpos); 
    
    glColor3f(50.0/255.0, 50.0/255.0, 50.0/255.0);
    glVertex3f(.8*scale+xpos, .8*scale+ypos, 1.0005*scale+zpos); 
  glEnd();
  
  // cockpit front and back
  glBegin(GL_QUADS);
  // front side
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(scale+xpos, 0.0+ypos, 0.0+zpos);
    
    glColor3f(105.0/255.0, 0.0/255.0, 105.0/255.0);
    glVertex3f(scale+xpos, scale+ypos, 0.0+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 105.0/255.0);
    glVertex3f(scale+xpos, scale+ypos, scale+zpos);
    
    glColor3f(100.0/255.0, 100.0/255.0, 100.0/255.0);
    glVertex3f(scale+xpos, 0.0+ypos, scale+zpos);
  // back side    
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
    
    glColor3f(50.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, 0.0+ypos, scale+zpos);
    
    glColor3f(0.0/255.0, 50.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, scale+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 50.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, 0.0+zpos);

  glEnd();
  
  // tail on backside, contains two pieces that make a wedge shape
 glBegin(GL_TRIANGLES);
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, 0.3*scale+zpos);

    glColor3f(150.0/255.0, 100.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, 0.3*scale+ypos, 0.5*scale+zpos);

    glColor3f(100.0/255.0, 0.0/255.0, 50.0/255.0);
    glVertex3f(-scale*1.3+xpos, scale+ypos, 0.5*scale+zpos);    
    
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, scale+ypos, 0.7*scale+zpos);

    glColor3f(150.0/255.0, 100.0/255.0, 0.0/255.0);
    glVertex3f(0.0+xpos, 0.3*scale+ypos, 0.5*scale+zpos);

    glColor3f(100.0/255.0, 0.0/255.0, 50.0/255.0);
    glVertex3f(-scale*1.3+xpos, scale+ypos, 0.5*scale+zpos);    

  glEnd();
 
 // simple prop on backside
 glBegin(GL_QUADS);
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(-scale*1.25+xpos, 1.2*scale+ypos, 0.5*scale+zpos);
    
    glColor3f(50.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(-scale*1.2+xpos, 1.2*scale+ypos, 0.5*scale+zpos);
    
    glColor3f(0.0/255.0, 50.0/255.0, 0.0/255.0);
    glVertex3f(-scale*1.35+xpos, 0.8*scale+ypos, 0.5*scale+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 50.0/255.0);
    glVertex3f(-scale*1.3+xpos, 0.8*scale+ypos, 0.5*scale+zpos);
 glEnd();
  
  // skiis underneath helicopter
  glBegin(GL_QUADS);
    // right ski
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(-scale*0.2+xpos, -scale*0.1+ypos, scale*0.8+zpos);
    glVertex3f(-scale*0.2+xpos, -scale*0.2+ypos, scale*0.8+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 200.0/255.0);
    glVertex3f(scale*1.2+xpos, -scale*0.1+ypos, scale*0.8+zpos);        
    glVertex3f(scale*1.2+xpos, -scale*0.2+ypos, scale*0.8+zpos);        
    
    // left ski
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(-scale*0.2+xpos, -scale*0.1+ypos, scale*0.2+zpos);
    glVertex3f(-scale*0.2+xpos, -scale*0.2+ypos, scale*0.2+zpos);
    
    glColor3f(200.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(scale*1.2+xpos, -scale*0.1+ypos, scale*0.2+zpos);        
    glVertex3f(scale*1.2+xpos, -scale*0.2+ypos, scale*0.2+zpos);        
  glEnd(); 
  
  // ski connectors 
  glBegin(GL_LINES);
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    
    // right ski connector back
    glVertex3f(scale*0.2+xpos, 0.0+ypos, scale*0.8+zpos);
    glVertex3f(scale*0.2+xpos, -scale*0.1+ypos, scale*0.8+zpos);        
    
    // right ski connector front
    glVertex3f(scale*0.8+xpos, 0.0+ypos, scale*0.8+zpos);
    glVertex3f(scale*0.8+xpos, -scale*0.1+ypos, scale*0.8+zpos);        
    
    // left ski connector back
    glVertex3f(scale*0.2+xpos, 0.0+ypos, scale*0.2+zpos);
    glVertex3f(scale*0.2+xpos, -scale*0.1+ypos, scale*0.2+zpos);        
    
    // left ski connector front
    glVertex3f(scale*0.8+xpos, 0.0+ypos, scale*0.2+zpos);
    glVertex3f(scale*0.8+xpos, -scale*0.1+ypos, scale*0.2+zpos);        
  glEnd();  

 // prop connector (a simple line)
  glBegin(GL_LINES);
    glColor3f(100.0/255.0, 100.0/255.0, 0.0/255.0);
    
    glVertex3f(scale*0.5+xpos, scale+ypos, scale*0.5+zpos);
    glVertex3f(scale*0.5+xpos, scale*1.1+ypos, scale*0.5+zpos);    
  glEnd(); 
  
  // prop itself (with a slight 'angle' to them)
  glBegin(GL_TRIANGLES);
    
    // one side
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(scale*0.5+xpos, scale*1.1+ypos, scale*0.5+zpos);
    
    glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
    glVertex3f(scale*1.3+xpos, scale*1.1+ypos, scale*1.4+zpos);    
    
    glColor3f(226.0/255.0, 113.0/255.0, 50.0/255.0);
    glVertex3f(scale*1.4+xpos, scale*1.05+ypos, scale*1.3+zpos);    
   
    // and the other
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    glVertex3f(scale*0.5+xpos, scale*1.1+ypos, scale*0.5+zpos);
    
    glColor3f(0.0/255.0, 113.0/255.0, 226.0/255.0);
    glVertex3f(-scale*0.3+xpos, scale*1.1+ypos, -scale*0.4+zpos);    
    
    glColor3f(50.0/255.0, 113.0/255.0, 226.0/255.0);
    glVertex3f(-scale*0.4+xpos, scale*1.05+ypos, -scale*0.3+zpos);    
  
  glEnd();  
  
   // reset the offset so that the axes are positioned properly
   xpos += 0.5*scale;
   ypos += 0.5*scale;
   zpos += 0.5*scale;  
   
  // draw a local coordinate system 
  glBegin(GL_LINES);
    
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    // draw x axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(scale*2+xpos, ypos, zpos);
    
    glColor3f(255.0/255.0, 0.0/255.0, 0.0/255.0);
    // draw y axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(xpos, scale*2+ypos, zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    // draw z axis
    glVertex3f(xpos, ypos, 0);
    glVertex3f(xpos, ypos, scale*2+zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    // draw x label
    glVertex3f(scale*2+xpos, .3*scale+ypos, .2*scale+zpos);
    glVertex3f(scale*2+xpos, -.3*scale+ypos, -.2*scale+zpos);
    glVertex3f(scale*2+xpos, .3*scale+ypos, -.2*scale+zpos);
    glVertex3f(scale*2+xpos, -.3*scale+ypos, .2*scale+zpos);    
    
    glColor3f(255.0/255.0, 0.0/255.0, 0.0/255.0);
    // draw y label
    glVertex3f(xpos, scale*2+ypos, zpos);
    glVertex3f(.2*scale+xpos, scale*2+ypos, .3*scale+zpos);
    glVertex3f(xpos, scale*2+ypos, zpos);
    glVertex3f(-.2*scale+xpos, scale*2+ypos, .3*scale+zpos);
    glVertex3f(xpos, scale*2+ypos, zpos);
    glVertex3f(xpos, scale*2+ypos, -.5*scale+zpos);    
  
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    // draw z label
    glVertex3f(-.3*scale+xpos, .3*scale+ypos, scale*2+zpos);
    glVertex3f(.3*scale+xpos, .3*scale+ypos, scale*2+zpos);
    glVertex3f(-.3*scale+xpos, -.3*scale+ypos, scale*2+zpos);
    glVertex3f(.3*scale+xpos, -.3*scale+ypos, scale*2+zpos);
    glVertex3f(.3*scale+xpos, .3*scale+ypos, scale*2+zpos);
    glVertex3f(-.3*scale+xpos, -.3*scale+ypos, scale*2+zpos);    
    
  glEnd();   
}

// this just draws a set of axes for the world coordinate system
// with the x,y, and z axes each color coded to the same color
// as in the local coordinate systems of the helicopters
// black - x
// red - y
// blue - z
void drawworldaxes(){
   float scale, xpos, ypos, zpos;   
   
   // draw the world coordinate axes
   // they are color coded different
   // from the local coordinate axes
   xpos = ypos = zpos = 0.0;
   scale = 10.0;   
   glBegin(GL_LINES);
    glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
    // draw x axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(scale*2+xpos, ypos, zpos);
    
    glColor3f(255.0/255.0, 0.0/255.0, 0.0/255.0);
    // draw y axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(xpos, scale*2+ypos, zpos);
    
    glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);
    // draw z axis
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(xpos, ypos, scale*2+zpos); 
    
    glColor3f(200.0/255.0, 200.0/255.0, 0.0/255.0);
    // draw a y=x line (helps with fourth helicopter)
    glVertex3f(xpos, ypos, zpos);
    glVertex3f(scale*2+xpos, scale*2+ypos, zpos);   
   glEnd();  
}

// here is the heart of the program. The differences in the way the helicopters
// rotate can be seen implemented here
// the world axes are drawn, along with each helicopter in its correct translation
// and rotation from the origin
void drawobjects(void)
{
   // makes for easy changes, eh?
   float translation_amount = 1.0;
   float heli_scale = 0.2;
   
   // draw world coordinates, self explanatory
   drawworldaxes();
   
   // **************************************************************************
   // first helicopter
   
   // save modelview state
   glPushMatrix();
   
   // this isn't needed, but used for consistency
   glTranslatef(0,0,0);
   // apply z, then y, then x
   // since glRotate adds matrices to the right
   // and matrices are multiplied right to left
   glRotatef(helicopter1_data[0], 1, 0, 0);   
   glRotatef(helicopter1_data[1], 0, 1, 0);
   glRotatef(helicopter1_data[2], 0, 0, 1);
   // param1 - scale
   // param2-4 - coordinates relative to world axes
   drawchopper(heli_scale, 0, 0, 0);
   // restore state of modelview
   glPopMatrix();
   
   // **************************************************************************
   // second helicopter
   
   // save modelview state
   glPushMatrix();
   // translate to correct spot
   glTranslatef(translation_amount,0,0);
   // pass the homogeneous matrix to multiply by the modelview
   glMultMatrixf(helicopter2_data.getData());
   // param1 - scale
   // param2-4 - coordinates relative to world axes
   drawchopper(heli_scale, 0, 0, 0);   
   // restore state of modelview
   glPopMatrix();
   
   // **************************************************************************
   // third helicopter

   // save modelview state
   glPushMatrix();
   // create a new matrix, this will be used to translate the helicopter
   gmtl::Matrix44f trans;
   // translates the matrix to the required position
   translate4x4world(trans, 0, translation_amount, 0);
   // at the last minute, we multiply by the rotation matrix and send it
   // to the glMultMatrixf function so that the helicopter appears
   // rotated and translated where it should
   glMultMatrixf((trans*helicopter3_data).getData());      
   // param1 - scale
   // param2-4 - coordinates relative to world axes
   drawchopper(heli_scale, 0, 0, 0);   
   // restore state of modelview
   glPopMatrix();
   
   // **************************************************************************
   // fourth helicopter
   
   // save modelview state
   glPushMatrix();   
   // for quaternion values:x,y,z are vector parts, w is scalar part
   float x,y,z,w;
   // for converting to angle axis
   float theta, kx, ky, kz, t;   
   // get the scalar and vector components of the quaternion
   helicopter4_data.get(x,y,z,w);
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
   glTranslatef(translation_amount,translation_amount,0);
   // if the angle is 0, the helicopter will not appear, so this is a check
   if(int(theta)%360 != 0){
      glRotatef(theta, kx, ky, kz);
   }
   // param1 - scale
   // param2-4 - coordinates relative to world axes
   drawchopper(heli_scale, 0, 0, 0); 
   // restore state of modelview
   glPopMatrix();   
}

// tie in to keypresses and do something with them
// such as....rotate the helicopters
// keys 1-6 are used for rotation, see comments below for details
void handleKeys(unsigned char key, int x, int y){

     // positive x rotation
     if(key == '1'){
            helicopter1_data[0] += 15.0;
            rotate4x4local(helicopter2_data, 15, 0, 0); 
            rotate4x4world(helicopter3_data, 15, 0, 0); 
            multiplyQuaternionBy(helicopter4_data, 15, 1);           
     }
     // negative x rotation
     else if(key == '2'){
            helicopter1_data[0] -= 15.0;
            rotate4x4local(helicopter2_data, -15, 0, 0);
            rotate4x4world(helicopter3_data, -15, 0, 0); 
            multiplyQuaternionBy(helicopter4_data, -15, 1);
     }
     // positive y rotation
     else if(key == '3'){
            helicopter1_data[1] += 15.0;
            rotate4x4local(helicopter2_data, 0, 15, 0);
            rotate4x4world(helicopter3_data, 0, 15, 0); 
            multiplyQuaternionBy(helicopter4_data, 15, 2);
     }
     // negative y rotation
     else if(key == '4'){
            helicopter1_data[1] -= 15.0;
            rotate4x4local(helicopter2_data, 0, -15, 0);            
            rotate4x4world(helicopter3_data, 0, -15, 0); 
            multiplyQuaternionBy(helicopter4_data, -15, 2);
     }
     // positive z rotation
     else if(key == '5'){
            helicopter1_data[2] += 15.0;
            rotate4x4local(helicopter2_data, 0, 0, 15);
            rotate4x4world(helicopter3_data, 0, 0, 15); 
            multiplyQuaternionBy(helicopter4_data, 15, 3);
     }
     // negative z rotation
     else if(key == '6'){
            helicopter1_data[2] -= 15.0;
            rotate4x4local(helicopter2_data, 0, 0, -15);            
            rotate4x4world(helicopter3_data, 0, 0, -15); 
            multiplyQuaternionBy(helicopter4_data, -15, 3);
     }
     // cause a redraw of the helicopters
     glutPostRedisplay();
}

void display(void)
{
  // these are for the gluLookAt function
  // to help describe what is going on
  float eye_x, eye_y, eye_z;
  float targ_x, targ_y, targ_z;
  float updir_x, updir_y, updir_z;
  
  // set the gluLookAt coordinates
  eye_x = 2.2;
  eye_y = 2.0;
  eye_z = 3.0;
  targ_x = 0.6;
  targ_y = 0.5;
  targ_z = 0.0;
  updir_x = 0.0;
  updir_y = 1.0;
  updir_z = 0.0;  
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  // identity matrix 
  glLoadIdentity();
  // box coordinate range
  // minX maxX minY maxY minZ maxZ
  glOrtho(-1.0, 1.0, -1.0, 1.0, -6.0, 6.0);

  glMatrixMode(GL_MODELVIEW);
  // identity matrix 
  glLoadIdentity();
  
  // this sets up the camera and is multiplied by the GL_MODELVIEW matrix
  gluLookAt(eye_x, eye_y, eye_z, targ_x, targ_y, targ_z, updir_x, updir_y, updir_z);

  // draw some helicopters
  drawobjects(); 
  
  glFlush();
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

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(W_WIDTH, W_HEIGHT);
  
  glutCreateWindow("SUPER DUPER CRAP PROGRAM");

  // init array for helicopter rotation angles
  initialize_helicopters();
  
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  // my key press handling function
  glutKeyboardFunc(handleKeys);
  
  init();

  // pass the torch to glut
  glutMainLoop();

  return 0;
}
