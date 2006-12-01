// Class    : CMPS 415
// Section  : 001
// Problem  : Programming Assignment #5
// Name     : McKelvy, James Markus
// CLID     : jmm0468

// Using Dev-C++ Version 4.9.9.2
// To compile this program, open assignment5.dev (project file),
//  the files needed:
//   - assignment5.cpp 
//  should be automatically added to the project.

// Press f9 to compile & run

//******************************************************************************
// Particles have a limited lifetime. Some may die out.

// Colors of the particles change as the velocity increases (to a bluer color).

// By default, the emitter, attractor, and k1 values are changed (animated) by
// Bezier curves. The parameters are stored in a sample file called "params.txt"
// On each line, the first three values are x,y,z position for emitter,
// the next three values are x,y,z position for attractor,
// the last value is k1 value.

// Most of the current options include clicking somewhere on the program window.
// For options that include one variable, clicking lower on the window results
// in a lower value of the variable, while clicking higher results in a number
// close to the maximum value of the variable. Such options will be noted with
// a '*'

// press 'a' and click anywhere on the program window to move the attractor.

//*press 'b' and click anywhere on the screen to change the size of the attractor.

// press 'B' to toggle between using the beziers to animate and not.

//*press 'c' and click anywhere to change the restitution coefficient (bounce).

// press 'd' to see objects drawn, such as the emitter, attractor, and sphere.

// press 'e' and click anywhere on the program window to move the emitter.

//*press 'k' and click anywhere on the program window to change the values of
// the k1 and k2 constants. X-axis affects k1, Y-axis affects k2. These values
// are for the force equation, k1 is for the attractor, k2 is for the wind.

//*press 'm' and click anywhere on the program window to change the mass of the
// particles.

// press 'p' to toggle between using lines for the particles and points.

// press 'P' to toggle between drawing the inputs and curve contraints

//*press 'q' and click anywhere on the program window to change the size of the
// collision sphere

// press 'r' to toggle between r and r squared in the force equation.

// press 's' and click anywhere on the program window to move the sphere.

//*press 't' and click anywhere on the program window to change the time delta,
// speeding up or slowing down the animation

//*press 'T' and click anywhere on the program window to change the t-increment
// value for the bezier curves

//*press 'w' and click anywhere to change the vector of the wind. Clicking near
// the center generates a smaller vector and therefore less "wind".

#define GMTL_NO_METAPROG

#include <GL/glut.h>                          //opengl toolkit
#include <iostream>                           //cout
#include <fstream>                            //file reading
#include <gmtl/Vec.h>			              //Vectors
#include <gmtl/VecOps.h>		              //Vector Operations
#include <gmtl/Point.h>			              //Points
#include <gmtl/Matrix.h>		              //Matrices
#include <gmtl/MatrixOps.h>		              //Matrix Operations
#include <gmtl/Sphere.h>                      //Sphere Objects

using namespace std;

#define W_WIDTH  1000                         // window width
#define W_HEIGHT 700                          // window height

#define BOX_SIZE 25.0                         // size of "viewing" box (not really)
#define NUM_PARTICLES 3500                    // total number of particles

#define LIFE_MIN 200                          // min lifespan of particle
#define LIFE_DELTA 700                        // max lifespan of particle 

#define EJECTION_RADIUS 0.1                   // ejectrion radius of particles 
#define EJECTION_STRENGTH 0.5                 // ejection strength of particles 

#define DELTA_MIN 0.01                        // min time step 
#define DELTA_MAX 3.0                         // max time step 

#define EMITTER_RADIUS 0.3                    // radius of the emitter 
#define ATTRACTOR_RADIUS_MIN 0.1              // min radius of the attractor
#define ATTRACTOR_RADIUS_MAX 10.0             // max radius of the attractor 

#define MASS_MAX 50.0                         // max mass of particles 
#define K1_MAX 5.0                            // max value of weight variable 
#define K2_MAX 2.0                            // max value of weight variable 
#define V_WIND_MAX 1.0                        // max wind velocity (one component) 
#define C_MAX 2.0                             // max bounce coefficient 

#define SPHERE_R_MAX 20.0                     // max size of the sphere 

#define NUM_PARAMETERS_PER_LINE 7             // number of params in file per line
#define T_INC_MAX 0.02                        // t_inc value for beziers 

using namespace std;

struct PARTICLE{                              // represents a particle 
    gmtl::Point3f p;                          // position
    gmtl::Vec3f v;                            // velocity
    int timeTillDeath;                        // lifetime
};

struct paramList{                             // maintains a list of file params 
    float param[NUM_PARAMETERS_PER_LINE];
};

PARTICLE particle[NUM_PARTICLES];             // the array of particles
float delta = 1.0;                            // the time delta to update particles
gmtl::Point3f emitter(-10, 15, 0);            // the position of the emitter
gmtl::Point3f attractor(10, -10, 0);          // position of the attractor
float attractor_radius = 3.0;                 // radius of the attractor
float m = 4.0;                                // mass of the particles
float k1 = 3.0;                               // weight variable in force eq
float k2 = 0.1;                               // weight variable in force eq
gmtl::Vec3f v_wind(0.01, 0.01, 0);            // wind vector
bool use_r_squared = true;                    // use r squared in force eq
bool draw_objects = true;                     // draw attractor,sphere,etc
bool draw_params = true;                      // draw control points, etc
bool use_beziers = true;                      // use beziers to animate
bool use_lines = true;                        // use lines or points for particles
float c = 0.5;                                // bounce coefficient
char clickMode = 'a';

gmtl::Point3f spherecenter(0,0,0);            // initial center of sphere
float spherer = 10.0;                         // sphere radius
gmtl::Spheref sphere(spherecenter, spherer);  // sphere to collide with
float sphererotation = 0;                     // a rotation angle of the sphere
const float rot_max = 360.0*500;              // max rotations until reset of 
                                              // previous variable   

int nSetsOfValues = 0;                        // number of sets of parameters
paramList * paramArray;                       // array of parameters

gmtl::Matrix44f Mb;                           // 4x4 Bezier basis matrix
float t = 0.0;                                // t used for beziers
float t_inc = 0.02;                           // increment for t
int i = 0;                                    // set of control points to deal with

// initializes the Bezier basis matrix
void setupBezierBasisMatrix(){
    Mb.set(-1, 3, -3, 1, 3, -6, 3, 0, -3, 3, 0, 0, 1, 0, 0, 0);
}

// generates a c1 continuous bezier curve
// modifies "obj" to have a new position based
// on which file parameters are set to be used
// also the iVal is which points to deal with
// and tVal is used for the beziers
void generateContinuousCurveC1(gmtl::Point3f &obj, int index1, int index2, int index3, int iVal, float tVal){
    int n = nSetsOfValues;
    gmtl::Vec4f T(pow(tVal,3), pow(tVal,2), tVal, 1);
    
    // array of midpoints
    gmtl::Point3f midpoints[n-1];
    
    // some temp points
    gmtl::Point3f temp1, temp2;    
    
    // generate midpoints for each adjacent pair
    for(int x = 0; x < n; x++){
      if(index2 != -1){    
        temp1 = *(new gmtl::Point3f(paramArray[x].param[index1], 
                                    paramArray[x].param[index2], 
                                    paramArray[x].param[index3]));
        temp2 = *(new gmtl::Point3f(paramArray[(x+1)%n].param[index1], 
                                    paramArray[(x+1)%n].param[index2], 
                                    paramArray[(x+1)%n].param[index3]));
      }
      // if there is only one parameter we need then do this, fill in with dummy
      // values
      else{
        temp1 = *(new gmtl::Point3f(paramArray[x].param[index1], 0, 0));
        temp1 = *(new gmtl::Point3f(paramArray[(x+1)%n].param[index1], 0, 0));        
      }
      // calculate the midpoints
      midpoints[x] = (temp1 + temp2) / 2.0;
    }
    
    // Gb matrix
    float G[4][3];
    
    // P1
    G[0][0] = midpoints[iVal][0];
    G[0][1] = midpoints[iVal][1];
    G[0][2] = midpoints[iVal][2];
        
    // P2
    G[1][0] = (1.0/3.0)*midpoints[iVal][0] + (2.0/3.0)*midpoints[(iVal+1)%n][0];
    G[1][1] = (1.0/3.0)*midpoints[iVal][1] + (2.0/3.0)*midpoints[(iVal+1)%n][1];
    G[1][2] = (1.0/3.0)*midpoints[iVal][2] + (2.0/3.0)*midpoints[(iVal+1)%n][2];
        
    // P3
    G[2][0] = (1.0/3.0)*midpoints[(iVal+1)%n][0] + (2.0/3.0)*midpoints[(iVal+1)%n][0];
    G[2][1] = (1.0/3.0)*midpoints[(iVal+1)%n][1] + (2.0/3.0)*midpoints[(iVal+1)%n][1];
    G[2][2] = (1.0/3.0)*midpoints[(iVal+1)%n][2] + (2.0/3.0)*midpoints[(iVal+1)%n][2];
                
    // P4
    G[3][0] = midpoints[(iVal+1)%n][0];
    G[3][1] = midpoints[(iVal+1)%n][1];
    G[3][2] = midpoints[(iVal+1)%n][2];
    
    // this is going to be Mb multiplied by Gb
    float matrix[4][3];
    // multiply 4x4 times 4x3 matrix
    for(int x = 0; x < 4; x++){
        for(int y = 0; y < 3; y++){
            matrix[x][y] = 0;
            for(int z = 0; z < 4; z++){
                matrix[x][y] += Mb[x][z] * G[z][y];
            }
        }
    }
    
    // now multiply T times Mb*Gb and set the object "position"
    for(int x = 0; x < 1; x++){
        for(int y = 0; y < 3; y++){
            obj[y] = 0;
            for(int z = 0; z < 4; z++){
                obj[y] += T[z]*matrix[z][y];
            }
        }
    }    
}

// reads a parameter file and sets the "nSetsOfValues" variable
// and also sets up the paramArray of size nSetsOfValues
void readFile(){
     ifstream fin1, fin2;
     char filename[] = "params.txt";
     
     // a temporary variable to help count number of parameter sets
     paramList temp;       
     
     // find out how many sets of values we have
     fin1.open(filename, ifstream::in);
     while(!fin1.eof()){
         for(int i = 0; i < NUM_PARAMETERS_PER_LINE; i++){
             fin1 >> temp.param[i];
         }
         nSetsOfValues++;
     }
     fin1.close();
     
     // read in the sets of values into the array
     paramArray = new paramList[nSetsOfValues];
     fin2.open(filename, ifstream::in);
     float tempr;
     for(int i = 0; i < nSetsOfValues; i++){
         for(int j = 0; j < NUM_PARAMETERS_PER_LINE; j++){
             fin2 >> paramArray[i].param[j];
         }
     }
     fin2.close();
}

/*
Pseudo-Gaussian pseudo-random generator
by Christoph Borst - Tuesday, 14 November 2006, 07:48 PM
Originally by Donald H. House.
Here's a random-number function that takes a mean and standard deviation.
*/
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

    srand(time(NULL));
    hereb4 = 1;
   }

  i = rand()/(RAND_MAX/RESOLUTION)+1;
  if (i > RESOLUTION)
    i = RESOLUTION;
  return(mean + std * lookup[i]);
}

// calculates distance between two points
float dist(const gmtl::Point3f &p1, const gmtl::Point3f &p2){
      return sqrt( pow(p1[0]-p2[0], 2) + pow(p1[1]-p2[1], 2) + pow(p1[2]-p2[2], 2) );
}

// calculate the force on a particle
gmtl::Vec3f f(const PARTICLE &p, gmtl::Point3f attractor_pos){
     // k1*m*d / r^2         -    k2(v - v_wind)
     // k1 and k2 affect strength
     // d is a normalized vector pointing from p to attractor
     // r is distance between p and attractor
     // v wind is wind velocity
     
     gmtl::Vec3f d(attractor_pos[0]-p.p[0], attractor_pos[1]-p.p[1], attractor_pos[2]-p.p[2]);
     gmtl::normalize(d);
     
     float r = dist(p.p, attractor_pos);
     
     if(use_r_squared)
         return ((k1*m*d)/(r*r)) - (k2*(p.v - v_wind));
     else
         return ((k1*m*d)/(r)) - (k2*(p.v - v_wind));
}

// initialize particle to some random values
void initializeParticle(PARTICLE &particle, gmtl::Point3f c){
     float x,y,z;
     float theta;
     
     theta = 360*(fastgauss(0.0, 1.0));    
     x = EJECTION_RADIUS*gmtl::Math::cos(gmtl::Math::deg2Rad(theta));
     y = EJECTION_RADIUS*gmtl::Math::sin(gmtl::Math::deg2Rad(theta));
     z = fastgauss(0.0, 1.0);
     
     gmtl::Point3f p(x+c[0], y+c[1], z+c[2]);     
     
     theta = 360*(fastgauss(0.0, 1.0));    
     x = EJECTION_STRENGTH*gmtl::Math::cos(gmtl::Math::deg2Rad(theta));
     y = EJECTION_STRENGTH*gmtl::Math::sin(gmtl::Math::deg2Rad(theta));
     z = fastgauss(0.0, 0.05);
     
     gmtl::Vec3f v(x,y,z);
     
     particle.p = p;
     particle.v = v;
     particle.timeTillDeath = rand()%LIFE_DELTA + LIFE_MIN;
}

// initializes all particles
void initializeAllParticles(){
    for(int i = 0; i < NUM_PARTICLES; i++){
        initializeParticle(particle[i], emitter);
    }
}

// check if time for death
void check_DeathOfParticle(PARTICLE &s, gmtl::Point3f emitter_pos){
     if(s.timeTillDeath <= 0)
         initializeParticle(s, emitter_pos);
}

// check if near attractor or too far away
void check_AttractorReset(PARTICLE &s, gmtl::Point3f attractor_pos, gmtl::Point3f emitter_pos, float closestDist, float farthestDist){
     float distance = dist(s.p, attractor_pos);
     if(distance <= closestDist || distance >= farthestDist){
         initializeParticle(s, emitter_pos);
     }
}

// check if we collide with the sphere and do something about it
void check_SphereCollision(PARTICLE &s, gmtl::Spheref asphere){
     gmtl::Point3f ctr = asphere.mCenter;
     float radius = asphere.mRadius;
     // check if distance from particle to center of sphere
     // is within range of radius
     if(dist(s.p, ctr) < radius){
         // calculate the sphere's surface normal
         gmtl::Vec3f n(s.p[0]-ctr[0], s.p[1]-ctr[1], s.p[2]-ctr[2]);
         gmtl::normalize(n);
         
         if(gmtl::dot(n, s.v) < 0){
             // the velocity component normal to the surface
             gmtl::Vec3f v_n = gmtl::dot(s.v, n) * n;
             // the tangential velocity to the surface
             gmtl::Vec3f v_t = s.v - v_n;
             
             // new velocity of the particle, c is the restitution coefficient
             gmtl::Vec3f v_new = v_t - c * v_n;
             
             s.v = v_new;
         }
     }
}

// this moves the particle along one time interval,
// checks if it is time to die
// checks if it is near the attractor
// checks if it collides with the sphere
void updateParticle(PARTICLE &s){
     //s = s + delta*Deriv(s)
     PARTICLE deriv_s;
     // position is actually velocity in this derivative state
     deriv_s.p = delta*s.v;
     // velocity is actually acceleration in this derivative state
     // a = f/m, m is mass, f in net force on the particle
     deriv_s.v = delta*(f(s,attractor)/m);
     
     s.p = s.p + deriv_s.p;
     s.v = s.v + deriv_s.v;
     
     // we now have less time to live :(
     s.timeTillDeath -= 1;
     
     check_DeathOfParticle(s, emitter);
     check_AttractorReset(s, attractor, emitter, attractor_radius, 1.42*2*BOX_SIZE);
     check_SphereCollision(s, sphere);
}

// drawing function draws the particles as lines or points
// also draws the control parameters if chosen
// also draws the sphere, attractor, emitter if chosen
void drawobjects(void){ 
    // draw lines for the particles   
    if(use_lines){
        glBegin(GL_LINES);
        for(int i = 0; i < NUM_PARTICLES; i++){
            float color = particle[i].v[0]*particle[i].v[0] + particle[i].v[1]*particle[i].v[1] + particle[i].v[2]*particle[i].v[2];
            
            // "forward" portion of particle
            glColor3f(1.0-color, 1.0, 1.0);
            glVertex3f(particle[i].p[0],particle[i].p[1],particle[i].p[2]);
            // tail portion
            //glColor3f(0,0,1.0 - float(particle[i].timeTillDeath)/(float(LIFE_MIN+LIFE_DELTA)));
            glColor3f(0.0,0.0,color);
            glVertex3f(particle[i].p[0]-particle[i].v[0],particle[i].p[1]-particle[i].v[1],particle[i].p[2]-particle[i].v[2]);        
        }
        glEnd(); 
    }   
    // draw points for the particles 
    else{
        glBegin(GL_POINTS);
        for(int i = 0; i < NUM_PARTICLES; i++){
            glColor3f(0,float(particle[i].timeTillDeath)/(float(LIFE_MIN+LIFE_DELTA)),1.0 - float(particle[i].timeTillDeath)/(float(LIFE_MIN+LIFE_DELTA)));
            glVertex3f(particle[i].p[0],particle[i].p[1],particle[i].p[2]);    
        }
        glEnd(); 
    }
    
    // draws the sphers, emitter, attractor
    if(draw_objects){
        // draw sphere for particles to collide with
        glPushMatrix();
        glTranslatef(sphere.mCenter[0], sphere.mCenter[1], sphere.mCenter[2]);
        glRotatef(sphererotation*.97, 1, 1, 1);
        glColor3f(0.05, 0.55, 0);
        glutWireSphere(sphere.mRadius, 20, 20);   
        glPopMatrix();
      
        // draw an "emitter"
        glPushMatrix();
        glTranslatef(emitter[0], emitter[1], emitter[2]);
        glColor3f(0, 0, .5);
        glutWireSphere(EMITTER_RADIUS+0.5, 20, 20);   
        glPopMatrix();
        
        // draw an "attractor"
        glPushMatrix();
        glTranslatef(attractor[0], attractor[1], attractor[2]);
        glRotatef(sphererotation*25, .7, -1, 1);
        glColor3f(0.0, 0.0, 0.55);
        glutWireSphere(attractor_radius, 20, 20);   
        glPopMatrix();
        
        // update the rotation of the sphere by the time step value
        if(sphererotation > rot_max)
            sphererotation = 0;
        else
            sphererotation += delta;
    }
    
    // draw the control parameters
    if(draw_params){
        for(int counter = 0; counter < nSetsOfValues; counter++){
            // emitter param points
            glColor3f(1.0, 0.0, 0.0);
            glPushMatrix();
            glTranslatef(paramArray[counter].param[0], paramArray[counter].param[1], paramArray[counter].param[2]);
            glutSolidSphere(0.2, 10, 10);
            glPopMatrix();
            
            // attractor param points
            glColor3f(1.0, 1.0, 0.0);
            glPushMatrix();
            glTranslatef(paramArray[counter].param[3], paramArray[counter].param[4], paramArray[counter].param[5]);
            glutSolidSphere(0.2, 10, 10);
            glPopMatrix();
        }      
        // this part traces out the paths of the objects  
        gmtl::Point3f pt;                
        glColor3f(.5, .5, .5);
        glBegin(GL_POINTS);
            float myT = 0.0;
            int myI = 0;
            while(myI < nSetsOfValues){
                myT = 0.0;
                while(myT <= 1.0){
                    // for the attractor
                    generateContinuousCurveC1(pt, 0, 1, 2, myI, myT);     
                    glVertex3f(pt[0], pt[1],pt[2]); 
                    // for the emitter
                    generateContinuousCurveC1(pt, 3, 4, 5, myI, myT);     
                    glVertex3f(pt[0], pt[1],pt[2]); 
                    myT += t_inc;
                }
                myI++;
            }
        glEnd();
    }
}

// Gets called when OpenGL has nothing else to do.
// This is where the particles get updated.
void idleFunc(){
     // move the simulation ahead one step
     for(int x = 0; x < NUM_PARTICLES; x++){
         updateParticle(particle[x]);         
     }
     
     // if we are using beziers, update the values of relevant parameters
     if(use_beziers){
         // this is for k1
         gmtl::Point3f temppt;
         
         generateContinuousCurveC1(emitter, 0, 1, 2, i, t);     
         generateContinuousCurveC1(attractor, 3, 4, 5, i, t);
         generateContinuousCurveC1(temppt, 6, -1, -1, i, t);
         k1 = temppt[0];
         
         t += t_inc;
         if(t > 1.0){
              t = 0.0;
              i++;
              if(i >= nSetsOfValues){
                  i = 0;
              }
         }
     }     
     glutPostRedisplay();
}

// moves the attractor
void moveAttractor(int x, int y){
    int w = W_WIDTH;
    int h = W_HEIGHT;
    
    gmtl::Point3f newpos((((2*BOX_SIZE)/w)*(x - w/2)), (((-2*BOX_SIZE)/h)*(y - h/2)), 0.0);
    attractor = newpos;
    cout << "New Attractor Position: " << newpos[0] << " x " << newpos[1] << endl;
}

// resizes the attractor
void resizeAttractor(int y){
    int h = W_HEIGHT;
    attractor_radius = ATTRACTOR_RADIUS_MAX*((float)(h-y)/h);
    if(attractor_radius < ATTRACTOR_RADIUS_MIN)
        attractor_radius = ATTRACTOR_RADIUS_MIN;
    cout << "New Attractor Radius: " << attractor_radius << endl;
}

// changes bounce
void changeRestitutionCoefficient(int y){
    int h = W_HEIGHT;
    c = C_MAX*((float)(h-y)/h);
    cout << "New Restitution Coefficient: " << c << endl;
}

// moves emitter
void moveEmitter(int x, int  y){
    int w = W_WIDTH;
    int h = W_HEIGHT;
    
    gmtl::Point3f newpos((((2*BOX_SIZE)/w)*(x - w/2)), (((-2*BOX_SIZE)/h)*(y - h/2)), 0.0);
    emitter = newpos;
    cout << "New Emitter Position: " << newpos[0] << " x " << newpos[1] << endl;
}

// scales k1 and k2 simultaneously
void change_k1_k2(int x, int y){
    int w = W_WIDTH;
    int h = W_HEIGHT;
    //k1 = K1_MAX*((float)x/w;
    // allow for repelling force.
    k1 = (((2*K1_MAX)/w)*(x - w/2));
    k2 = K2_MAX*((float)(h-y)/h);
    cout << "New k1 value: " << k1 << endl;
    cout << "New k2 value: " << k2 << endl;
}

// changes the mass
void changeMass(int y){
    int h = W_HEIGHT;
    m = MASS_MAX*((float)(h-y)/h);
    cout << "New Mass: " << m << endl;
}

// resizes the sphere
void resizeSphere(int y){
    int h = W_HEIGHT;
    spherer = SPHERE_R_MAX*((float)(h-y)/h);
    cout << "New Sphere Radius: " << spherer << endl;
    gmtl::Spheref temp(sphere.mCenter, spherer);
    sphere = temp;
}

// changes the time step value
void changeTimeDelta(int y){
    int h = W_HEIGHT;
    delta = DELTA_MAX*((float)(h-y)/h);
    if(delta < DELTA_MIN)
        delta = DELTA_MIN;
    cout << "New Time delta: " << delta << endl;
}

// changes the incrementing value for t (beziers t)
void changeTincrement(int y){
    int h = W_HEIGHT;
    t_inc = T_INC_MAX*((float)(h-y)/h);
    cout << "New T-increment: " << t_inc << endl;
}

// moves the sphere around
void moveSphere(int x, int  y){
    int w = W_WIDTH;
    int h = W_HEIGHT;
    
    gmtl::Point3f newpos((((2*BOX_SIZE)/w)*(x - w/2)), (((-2*BOX_SIZE)/h)*(y - h/2)), 0.0);
    sphere.mCenter = newpos;
    cout << "New Sphere Position: " << newpos[0] << " x " << newpos[1] << endl;
}

// changes the wind vector
void changeWind(int x, int y){
    int w = W_WIDTH;
    int h = W_HEIGHT;
    
    gmtl::Point3f newpos((((2*V_WIND_MAX)/w)*(x - w/2)), (((-2*V_WIND_MAX)/h)*(y - h/2)), 0.0);
    v_wind = newpos;
    cout << "New Wind Vector (velocity): " << newpos[0] << " x " << newpos[1] << endl;
}

// ties into mouse presses
void mouseFunc(int button, int state, int x, int y){
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        if(clickMode == 'a')
                     moveAttractor(x,y);
        if(clickMode == 'b')
                     resizeAttractor(y);
        if(clickMode == 'c')
                     changeRestitutionCoefficient(y);
        // if clickMode == 'd', then draw_objects is changed (true/false)
        if(clickMode == 'e')
                     moveEmitter(x,y);
        if(clickMode == 'k')
                     change_k1_k2(x,y);
        if(clickMode == 'm')
                     changeMass(y);
        //if clickMode == 'p', then use_lines is changed (true/false)
        if(clickMode == 'q')
                     resizeSphere(y);
        //if clickMode == 'r', then use_r_squared is changed (true/false)
        if(clickMode == 's')
                     moveSphere(x,y);
        if(clickMode == 't')
                     changeTimeDelta(y);
        if(clickMode == 'T')
                     changeTincrement(y);
        if(clickMode == 'w')
                     changeWind(x,y);

    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        moveEmitter(x,y);
    }
    
}

// ties into key presses
void keyboardFunc(unsigned char key, int x, int y){
     
     if(key == 'a')
            clickMode = 'a';
     else if(key == 'b')
            clickMode = 'b';
     else if(key == 'B')
            use_beziers = !use_beziers;
     else if(key == 'c')
            clickMode = 'c';
     else if(key == 'd')
            draw_objects = !draw_objects;
     else if(key == 'e')
            clickMode = 'e';
     else if(key == 'k')
            clickMode = 'k';
     else if(key == 'm')
            clickMode = 'm'; 
     else if(key == 'p')
            use_lines = !use_lines;
     else if(key == 'P')
            draw_params = !draw_params;
     else if(key == 'q')
            clickMode = 'q';
     else if(key == 'r')
            use_r_squared = !use_r_squared;
     else if(key == 's')
            clickMode = 's';
     else if(key == 't')
            clickMode = 't';
     else if(key == 'T')
            clickMode = 'T';
     else if(key == 'w')
            clickMode = 'w'; 
        
}

// setup the display stuff
void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-BOX_SIZE, BOX_SIZE, -BOX_SIZE, BOX_SIZE, -BOX_SIZE, BOX_SIZE*2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();  
    gluLookAt(0.0, 0.0, BOX_SIZE, 0.0, 0.0, 0.0, 0.0, BOX_SIZE, 0.0);
    drawobjects();   
    glutSwapBuffers();
}

// if a resize of the window happens
void reshape(int w, int h){
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

// initialization stuff
void init(void){
    setupBezierBasisMatrix();
    readFile();
    initializeAllParticles();
    
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
}

int main(int argc, char **argv){ 
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(W_WIDTH, W_HEIGHT);  
    glutCreateWindow("OH WHERE OH WHERE HAVE MY PARTICLES GONE?");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardFunc);
    glutMouseFunc(mouseFunc);
    glutIdleFunc(idleFunc);
  
    init();
    glutMainLoop();

    return 0;
}
