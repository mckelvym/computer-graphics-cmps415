// Class    : CMPS 415
// Section  : 001
// Problem  : Programming Assignment #4
// Name     : McKelvy, James Markus
// CLID     : jmm0468

// Using Dev-C++ Version 4.9.9.2
// To compile this program, open assignment4.dev (project file),
//  the files needed:
//   - assignment4.cpp 
//  should be automatically added to the project.

// Press f9 to compile & run

// for file parameters, here are the examples:
// SPHERE 0 0 -1 1.0 20 20 20 0 150 0 255 255 255 50
// CUBE   -1 0 0 1.5 20 20 20 0 0 150 255 255 255 100
// up to 4 objects of each type (in any order) are currently supported

#define GMTL_NO_METAPROG

#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>                
#include <string>
#include <gmtl/Vec.h>			//Vectors
#include <gmtl/VecOps.h>		//Vector Operations
#include <gmtl/Point.h>			//Points
#include <gmtl/Matrix.h>		//Matrices
#include <gmtl/Generate.h>		//inversions
#include <gmtl/MatrixOps.h>		//Matrix Operations
#include <gmtl/Sphere.h>        //Sphere Objects
#include <gmtl/AABox.h>         //Box Objects
#include <gmtl/Intersection.h>  //Intersection Routines
#include <gmtl/Ray.h>           //Rays

using namespace std;

// struct for a sphere
// spheref is a gmtl sphere with center and radius
// amb_[r,b,g] is ambient color amount
// diff_[r,b,g] is diffuse coloramount
// spec_[r,b,g] is specular color amount
// spec_exp is the specular exponent
struct sphere{
    gmtl::Spheref spheref;
    float amb_r;
    float amb_g;
    float amb_b;
    float diff_r;
    float diff_g;
    float diff_b;
    float spec_r;
    float spec_g;
    float spec_b;
    float spec_exp;  
};

// struct for a cube
// boxf is a gmtl box with min and max points
// amb_[r,b,g] is ambient color amount
// diff_[r,b,g] is diffuse coloramount
// spec_[r,b,g] is specular color amount
// spec_exp is the specular exponent
struct box{
    gmtl::AABoxf boxf;
    float amb_r;
    float amb_g;
    float amb_b;
    float diff_r;
    float diff_g;
    float diff_b;
    float spec_r;
    float spec_g;
    float spec_b;
    float spec_exp;         
};

GLubyte * frame_buffer;            // frame buffer array
int W_WIDTH = 200;                 // window width
int W_HEIGHT = 200;                // window height
float l,r,b,t,n;                   // "frustum" coordinates for virtual window
gmtl::Vec3f C_eye, C_center, C_up; // "gluLookAt" parameters
gmtl::Point3f light;               // location of the light source
int sphere_count = 0;              // how many spheres do we have
int box_count = 0;                 // how many boxes do we have
// these arrays could be dynamically allocated, but to simplify things they
// are just limited to 4 elements
sphere sphere_array[4];            // array of spheres
box box_array[4];                  // array of boxes   

// calculates an index into the frame_buffer array to color the correct pixel
// xcoord is an x coordinate
// ycoord is a y coordinate
// color is 0 for red part of pixel
// color is 1 for green part of pixel
// color is 2 for blue part of pixel
int calcPos(int xcoord, int ycoord, int color){
  return (color + 3*xcoord + 3*ycoord*W_WIDTH);
}

// make a camera pose from given vectors
// this is similar to gluLookAt
gmtl::Matrix44f composeCamera(gmtl::Vec3f eye, gmtl::Vec3f center, gmtl::Vec3f up){
    gmtl::Matrix44f cam;    
    gmtl::Vec3f Xeye, Yeye, Zeye;
    float x,y,z;
    
    Zeye = eye - center;
    gmtl::normalize(Zeye);
    
    // Xeye = cross(up, Zeye);
    gmtl::cross(Xeye, up, Zeye);
    gmtl::normalize(Xeye);
    
    // Yeye = cross(Zeye, Xeye);
    gmtl::cross(Yeye, Zeye, Xeye);
    gmtl::normalize(Yeye);

    for(int i = 0; i < 3; i++){
        cam[i][0] = Xeye[i];
        cam[i][1] = Yeye[i];
        cam[i][2] = Zeye[i];
        cam[i][3] = eye[i];
    }
 
    return cam;
}

// initializes a sphere
// moves it according to a camera transformation
void readSphereFromFile(ifstream &fin, int sphere_index){
     float x, y, z, r;
     fin >> x;
     fin >> y;
     fin >> z;
     fin >> r;
     fin >> sphere_array[sphere_index].amb_r;
     fin >> sphere_array[sphere_index].amb_g;
     fin >> sphere_array[sphere_index].amb_b;
     fin >> sphere_array[sphere_index].diff_r;
     fin >> sphere_array[sphere_index].diff_g;
     fin >> sphere_array[sphere_index].diff_b;
     fin >> sphere_array[sphere_index].spec_r;
     fin >> sphere_array[sphere_index].spec_g;
     fin >> sphere_array[sphere_index].spec_b;
     fin >> sphere_array[sphere_index].spec_exp; 
     gmtl::Point3f center(x,y,z);
     
     // lets compute a camera pose and apply it to the object
     gmtl::Matrix44f C = composeCamera(C_eye, C_center, C_up);
     // yields the correct inverse
     C.mState = 2;
     C = gmtl::invert(C);
     center = C * center;

     gmtl::Spheref spheref(center, r);
     sphere_array[sphere_index].spheref = spheref;
}

// initializes a cube
// moves it according to a camera transformation
void readCubeFromFile(ifstream &fin, int box_index){     
     float x, y, z, s;
     fin >> x;
     fin >> y;
     fin >> z;
     fin >> s;     
     fin >> box_array[box_index].amb_r;
     fin >> box_array[box_index].amb_g;
     fin >> box_array[box_index].amb_b;
     fin >> box_array[box_index].diff_r;
     fin >> box_array[box_index].diff_g;
     fin >> box_array[box_index].diff_b;
     fin >> box_array[box_index].spec_r;
     fin >> box_array[box_index].spec_g;
     fin >> box_array[box_index].spec_b;
     fin >> box_array[box_index].spec_exp; 
     gmtl::Point3f center(x,y,z);
     
     // lets compute a camera pose and apply it to the object
     gmtl::Matrix44f C = composeCamera(C_eye, C_center, C_up);
     // yields the correct inverse
     C.mState = 2;
     C = gmtl::invert(C);
     center = C * center;
     
     x = center[0];
     y = center[1];
     z = center[2];
     
     gmtl::Point3f minP(x-(s/2.0), y-(s/2.0), z-(s/2.0));
     gmtl::Point3f maxP(x+(s/2.0), y+(s/2.0), z+(s/2.0));
     gmtl::AABoxf boxf(minP, maxP);
     box_array[box_index].boxf = boxf;      
}

// reads the data file for
// w, h
// l,r,b,t,n
// eyex eyey eyez centerx centery centerz upx upy upz
// lightx lighty lightz
// [OBJECTTYPE] centerx centery centerz r/s ambr ambg ambb diffr diffg diffb specr specg specb specexp
// examples:
// SPHERE 0 0 -1 1.0 20 20 20 0 150 0 255 255 255 50
// CUBE   -1 0 0 1.5 20 20 20 0 0 150 255 255 255 100
// up to 4 objects of each type (in any order) are currently supported
void readFile(){
     ifstream fin;
     char fname[30];
     string spherestr = "SPHERE";
     string cubestr = "CUBE";
     string tempstr;
     float x,y,z;
     
     cout << "Welcome to my simple ray tracer!\nJames McKelvy\nNov 4, 2006\nComputer Graphics (CMPS415)\n\n";
     cout << "Specify filename please (ex. sample.txt): ";
     cin >> fname;
     fin.open(fname, ifstream::in);
     
     fin >> W_WIDTH >> W_HEIGHT;
     fin >> l >> r >> b >> t >> n;
     fin >> x >> y >> z;
     C_eye.set(x,y,z);
     fin >> x >> y >> z;
     // the values needed to be passed as negative because it wasn't 
     // "looking" in the right direction
     C_center.set(-x,-y,-z);
     fin >> x >> y >> z;
     C_up.set(x,y,z);
     fin >> x >> y >> z;
     light.set(x,y,z);
     fin >> tempstr;
     do{
         if(tempstr == spherestr)
             readSphereFromFile(fin, sphere_count++);
         else if(tempstr == cubestr)
             readCubeFromFile(fin, box_count++);
         fin >> tempstr;
     }while(!fin.eof());       
     
     fin.close();     
}

// clamping function for color values so that artifacts don't occur
float clamp(float value, const float largestAllowed){
    if(value > largestAllowed)
      value = largestAllowed;
    return value;     
}

// ray tracing routine, calculates intersections and colors pixels according
// to an illumination equation
void simpleRayTracer(){
    // for each scanline in image
    for(int j = 0; j < W_HEIGHT; j++){
       // for each pixel in scanline
       for(int i = 0; i < W_WIDTH; i++){
          // determine ray origin and direction for this pixel
          gmtl::Point3f p_eye(0,0,0);
          gmtl::Point3f p_pixel(l + (i + 0.5) * (r - l) / W_WIDTH, b + (j + 0.5) * (t - b) / W_HEIGHT, -n);
          // ray direction
          gmtl::Rayf d(p_eye, p_pixel);
          
          int numhits;        // for sphere intersection
          float t0, t1;       // entry and exit points
          int object = 0;     // keep track of which type of object was intersected
          int index = -1;     // which index in the object array
          float s = 1000.0;   // the distance along the ray that it was intersected
          
          // go through each sphere and see if it intersected the ray from the eye
          // record intersection if needed and type of object intersected
          for(int obj = 0; obj < sphere_count; obj++){
             if(gmtl::intersect(sphere_array[obj].spheref, d, numhits, t0, t1)){
                if(t0 < t1 && t0 >= 0 && t0 < s){
                  s = t0;
                  object = 1;
                  index = obj;
                }
                else if(t1 < t0 && t1 >= 0 && t1 < s){
                  s = t1;
                  object = 1;
                  index = obj;
                }                
             }
          }
          // go through each box and see if it intersected the ray from the eye
          // record intersection if needed and type of object intersected
          for(int obj = 0; obj < box_count; obj++){
             if(gmtl::intersectAABoxRay(box_array[obj].boxf, d, t0, t1)){
                if(t0 < t1 && t0 >= 0 && t0 < s){
                  s = t0;
                  object = 2;
                  index = obj;
                }
                else if(t1 < t0 && t1 >= 0 && t1 < s){
                  s = t1;
                  object = 2;
                  index = obj;
                }                
             }
          }

          // set up the point of intersection
          gmtl::Point3f intersection = d.mOrigin + d.mDir * s; 
          
          gmtl::Rayf shadow(intersection, light);  // set up a shadow ray
          int S = 1;                               // shadow term
          s = 1000.0;                              // reset s 
          
          // go through each sphere and see if it intersected the ray for shadowing
          // record intersection spot
          for(int obj = 0; obj < sphere_count; obj++){
             if(gmtl::intersect(sphere_array[obj].spheref, shadow, numhits, t0, t1) && s > 1.0){
                if(t0 < t1 && t0 >= 0.0001f && t0 < s){
                  s = t0;                  
                }
                else if(t1 < t0 && t1 >= 0.0001f && t1 < s){
                  s = t1;
                }                
             }
             if(s <= 1.0)
                break;
          }
          // go through each box and see if it intersected the ray for shadowing
          // record intersection spot
          for(int obj = 0; obj < box_count; obj++){
             if(gmtl::intersectAABoxRay(box_array[obj].boxf, shadow, t0, t1) && s > 1.0){
                if(t0 < t1 && t0 >= 0.0001f && t0 < s){
                  s = t0;
                }
                else if(t1 < t0 && t1 >= 0.0001f && t1 < s){
                  s = t1;
                }            
                if(s <= 1.0)
                  break;    
             }
          }
          
          // set the shadow term
          // 0 is for shaded
          // 1 is for not shaded
          if(s <= 1.0 && s >= 0)
             S = 0;
          
          // precompute an index into the frame_buffer array
          int pos = calcPos(i,j,0);
          
          // object intersected is a sphere
          if(object == 1){
            // L is light reflection vector, normalized
            // V is vector from intersection to eye, normalized
            // N is the surface normal vector, normalized
            // Rf is the ideal reflection vector, normalized
            // the illumination equation is:
            // I = Oamb + S( Odiff*(max(0, NL)) + Ospec*(max(0, RV))^n)
            // where I is intensity, O is object, S is shadow term
            gmtl::Vec3f L = light - intersection;
            gmtl::Vec3f V = p_eye - intersection;
            gmtl::Vec3f N = (intersection - sphere_array[index].spheref.mCenter)/sphere_array[index].spheref.mRadius;
            gmtl::normalize(L);
            gmtl::normalize(V);            
            gmtl::Vec3f Rf = 2*N*(gmtl::dot(N, L)) - L;    
            
            // calculate the individual r,g,b values now  
            //red
            frame_buffer[pos] = 
                              clamp(sphere_array[index].amb_r + 
                              S*(sphere_array[index].diff_r*max(0.0f, gmtl::dot(N, L)) + 
                              sphere_array[index].spec_r*pow(max(0.0f, gmtl::dot(Rf, V)), sphere_array[index].spec_exp)), 255);
            //green
            frame_buffer[pos+1] = 
                              clamp(sphere_array[index].amb_g + 
                              S*(sphere_array[index].diff_g*max(0.0f, gmtl::dot(N, L)) + 
                              sphere_array[index].spec_g*pow(max(0.0f, gmtl::dot(Rf, V)), sphere_array[index].spec_exp)), 255);
            //blue     
            frame_buffer[pos+2] = 
                              clamp(sphere_array[index].amb_b + 
                              S*(sphere_array[index].diff_b*max(0.0f, gmtl::dot(N, L)) + 
                              sphere_array[index].spec_b*pow(max(0.0f, gmtl::dot(Rf, V)), sphere_array[index].spec_exp)), 255);                
          }
          // object intersected is a cube
          if(object == 2){
            // L is light reflection vector, normalized
            // V is vector from intersection to eye, normalized
            // N is the surface normal vector, normalized
            // Rf is the ideal reflection vector, normalized
            // the illumination equation is:
            // I = Oamb + S( Odiff*(max(0, NL)) + Ospec*(max(0, RV))^n)
            // where I is intensity, O is object, S is shadow term
            gmtl::Vec3f L = light - intersection;
            gmtl::Vec3f V = p_eye - intersection;
            gmtl::Vec3f N = (intersection - ((box_array[index].boxf.mMin + box_array[index].boxf.mMax)/2.0));
            gmtl::normalize(L);
            gmtl::normalize(V);            
            gmtl::normalize(N);
            gmtl::Vec3f Rf = 2*N*(gmtl::dot(N, L)) - L;   
            
            // calculate the individual r,g,b values now  
            //red
            frame_buffer[pos] = 
                              clamp(box_array[index].amb_r + 
                              S*(box_array[index].diff_r*max(0.0f, gmtl::dot(N, L)) + 
                              box_array[index].spec_r*pow(max(0.0f, gmtl::dot(Rf, V)), box_array[index].spec_exp)), 255);
            //green
            frame_buffer[pos+1] = 
                              clamp(box_array[index].amb_g + 
                              S*(box_array[index].diff_g*max(0.0f, gmtl::dot(N, L)) + 
                              box_array[index].spec_g*pow(max(0.0f, gmtl::dot(Rf, V)), box_array[index].spec_exp)), 255);
            //blue
            frame_buffer[pos+2] = 
                              clamp(box_array[index].amb_b + 
                              S*(box_array[index].diff_b*max(0.0f, gmtl::dot(N, L)) + 
                              box_array[index].spec_b*pow(max(0.0f, gmtl::dot(Rf, V)), box_array[index].spec_exp)), 255);            
          }   
       }
    }
}

// Called by GLUT when a display event occurs: 
void display(void) {

	//	Set the raster position to the lower-left corner to avoid a problem 
	//	(with glDrawPixels) when the window is resized to smaller dimensions.
	glRasterPos2i(-1,-1);
 
	// Write the information stored in "frame_buffer" to the color buffer
	glDrawPixels(W_WIDTH, W_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, frame_buffer);
	
	glFlush();
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

int main(int argc, char **argv)
{ 
  readFile();                                          // read in relevant data
  frame_buffer = new GLubyte[W_HEIGHT * W_WIDTH * 3];  // initialize frame buffer
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(W_WIDTH, W_HEIGHT);  
  glutCreateWindow("RAY TRACER SUPREME");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  cout << "\nNow Computing...";
  simpleRayTracer();                                   // start computation
  cout << "DONE!";
  //glutKeyboardFunc(keyboardFunc);
  //glutMouseFunc(mouseFunc);
  
  glutMainLoop();

  return 1;
}
