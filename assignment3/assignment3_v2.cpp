// See keymapping information at the bottom of file...

#define GMTL_NO_METAPROG
#define W_WIDTH  800
#define W_HEIGHT 600

#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>

#include <gmtl/Matrix.h>	//Matrices
#include <gmtl/MatrixOps.h>	//Matrix Operations
#include <gmtl/Quat.h>		//Quaternions
#include <gmtl/EulerAngle.h>	//Euler Angles
#include <gmtl/Generate.h>	//makeRot/makeTrans/etc.
#include <gmtl/Vec.h>		//Vectors
#include <gmtl/VecOps.h>	//Vector Operations

using namespace std;

// shadow mapping stuff
GLuint g_depthTexture = -1;
GLuint depth_texture = -1;
gmtl::Matrix44f scale_and_bias;
gmtl::Matrix44f lproj; // the light projection matrix
gmtl::Matrix44f lview; // the light view matrix
gmtl::Matrix44f coords;
GLfloat plane_s[4], plane_t[4], plane_r[4], plane_q[4];

// frustum view values
static double l; static double r;
static double b; static double t;
static double n; static double f;

// lighting values
static const GLfloat color_specular[] = { 0.6, 0.6, 0.6, 1.0 };
static const GLfloat color_ambient[]  = { 0.2, 0.2, 0.2, 1.0 };
static const GLfloat color_diffuse[]  = { 0.1, 0.1, 0.1, 1.0 };

// helicopter variables
gmtl::Matrix44f helicopter1_main;
gmtl::Matrix44f helicopter1_topprop;
gmtl::Matrix44f helicopter1_rearprop;
gmtl::Matrix44f helicopter1_window;
gmtl::Matrix44f helicopter2_main;
gmtl::Matrix44f helicopter2_topprop;
gmtl::Matrix44f helicopter2_rearprop;
gmtl::Matrix44f helicopter2_window;

// light variables
gmtl::Matrix44f lightsource;
bool ambient = false;
bool diffuse = false;
bool specular = false;

// camera variables
gmtl::Matrix44f camera_translation_z_inverse;
gmtl::Matrix44f camera_rotation_x_inverse;
gmtl::Matrix44f camera_rotation_y_inverse;

// axes variables
bool drawminiaxes = false;
bool drawheliaxes = false;
bool drawmainaxes = true;
bool helicopter1_hingeopen = false;
bool helicopter2_hingeopen = false;

// texture variables
static GLuint texture_heli_front = -1;
static GLuint texture_heli_side = -1;
static GLuint texture_grass = -1;
static GLuint texture_bricks = -1;

// cone variables
struct norm{
	float x;
	float y;
	float z;
};
const float degrees_precision = 5.0;
//norm coneArray[360/degrees_precision];
norm coneArray[360/5];
bool coneArray_initialized = false;

// some rotation matrices for precomputing
gmtl::Matrix44f x_rototation_matrix_pos15deg;
gmtl::Matrix44f x_rototation_matrix_neg15deg;
gmtl::Matrix44f y_rototation_matrix_pos15deg;
gmtl::Matrix44f y_rototation_matrix_neg15deg;
gmtl::Matrix44f z_rototation_matrix_pos15deg;
gmtl::Matrix44f z_rototation_matrix_neg15deg;

// functions
void make_shadow_matrix(GLfloat planeCoeff[4], GLfloat lightPos[4], GLfloat destMat[4][4]);
void load_ppm_p6(char *fname, unsigned int *w, unsigned int *h, unsigned char **data, int mallocflag);
void setup_texture(char *filename, GLuint &texture);
void display_depth_texture ();
void convert_GLfloat44_to_Matrix44f(GLfloat source[4][4], gmtl::Matrix44f &output);
void set_matrix_position(gmtl::Matrix44f &A, float x, float y, float z);
void get_matrix_position(gmtl::Matrix44f A, float &x, float &y, float &z);
void translate_world(gmtl::Matrix44f &A, float dx, float dy, float dz);
void multiply_world(gmtl::Matrix44f &source, gmtl::Matrix44f mult);
void multiply_local(gmtl::Matrix44f &source, gmtl::Matrix44f mult);
void translate_local(gmtl::Matrix44f &A, float dx, float dy, float dz);
void rotate_world(gmtl::Matrix44f &A, float degX, float degY, float degZ);
void rotate_local(gmtl::Matrix44f &A, float degX, float degY, float degZ);
void draw_helicopter_cockpit();
void draw_helicopter_tail();
void draw_helicopter_rear_propeller();
void draw_helicopter_skiis();
void draw_helicopter_main_propeller();
void draw_helicopter_window();
void draw_helicopter_axes();
void draw_ground();
void draw_axes_world();
void draw_cone_lighted(float height, float radius, float degrees_precision, norm array[], bool &array_initialized);
void draw_helicopter_1();
void draw_helicopter_2();
void draw_cone();
void draw_light_source();
void draw(void);
void handleKeys(unsigned char key, int x, int y);
void set_light_position(gmtl::Matrix44f &lightsource);
void set_camera_position(gmtl::Matrix44f &referencePoint);
GLuint empty_texture ();
void display(void);
void reshape(int w, int h);
void init ();
void render_from_light ();

// This code, taken from the "OpenGl Superbible", calculates
// a shadow matrix (a matrix that can be added to modelview to
// project vertices to a ground plane based on a specified light point).
//
// planeCoeff is for the four coefficients of the plane equation with
//    respect to your current coordinate system.
//    The plane equation form is Ax + By + Cz + D = 0,
//        where (A,B,C) is a vector normal to the plane,
//        and D can be found be substituting in a point on the plane
//        (D will be the negated dot product of a point coord and the normal).
//        For example, for the horizontal ground plane y = -2:
//                    A = 0, B = 1, C = 0, and D = 2.
// lightPos is the light coordinate with respect to your current coordinate
//    system (note it has 4 elements, like an OpenGL light position).
// destMat is where the shadow matrix will be placed
void make_shadow_matrix(GLfloat planeCoeff[4], GLfloat lightPos[4], GLfloat destMat[4][4]){
	GLfloat dot;

	// Dot product of planeCoeff and light position
	dot =  lightPos[0] * planeCoeff[0] +
		lightPos[1] * planeCoeff[1] +
		lightPos[2] * planeCoeff[2] +
		lightPos[3] * planeCoeff[3];

	// Now do the projection
	// First column
	destMat[0][0] = dot - planeCoeff[0] * lightPos[0];
	destMat[1][0] = 0.0f - planeCoeff[0] * lightPos[1];
	destMat[2][0] = 0.0f - planeCoeff[0] * lightPos[2];
	destMat[3][0] = 0.0f - planeCoeff[0] * lightPos[3];

	// Second column
	destMat[0][1] = 0.0f - planeCoeff[1] * lightPos[0];
	destMat[1][1] = dot - planeCoeff[1] * lightPos[1];
	destMat[2][1] = 0.0f - planeCoeff[1] * lightPos[2];
	destMat[3][1] = 0.0f - planeCoeff[1] * lightPos[3];

	// Third Column
	destMat[0][2] = 0.0f - planeCoeff[2] * lightPos[0];
	destMat[1][2] = 0.0f - planeCoeff[2] * lightPos[1];
	destMat[2][2] = dot - planeCoeff[2] * lightPos[2];
	destMat[3][2] = 0.0f - planeCoeff[2] * lightPos[3];

	// Fourth Column
	destMat[0][3] = 0.0f - planeCoeff[3] * lightPos[0];
	destMat[1][3] = 0.0f - planeCoeff[3] * lightPos[1];
	destMat[2][3] = 0.0f - planeCoeff[3] * lightPos[2];
	destMat[3][3] = dot - planeCoeff[3] * lightPos[3];
}

// this code is from the class example given by Dr. Borst
/* load_ppm_p6 - a minimal Portable Pixelformat image file loader
   fname: name of file to load (input)
   w: width of loaded image in pixels (output)
   h: height of loaded image in pixels (output)
   data: image data address (input or output depending on mallocflag)
   mallocflag: 1 if memory not pre-allocated, 0 if data already points
   to allocated memory that can hold the image.
   Note that if new memory is allocated, free() should be used
   to deallocate when it is no longer needed.
*/
void load_ppm_p6(char *fname, unsigned int *w, unsigned int *h,
	     unsigned char **data, int mallocflag)
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
	}
	while (s[0] == '#');


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

// the majority of this code also comes from the class example by Dr. Borst
// setup steps for a texture map
// basically send this a filename and a texture reference
// and the image is loaded and ready to be drawn
void setup_texture(char *filename, GLuint &texture){
	static unsigned int texwidth, texheight;
	static unsigned char *imagedata;
	//static GLuint texture;

	/* NOTE: Only do these setup steps once per texture!
	   (note use of static variables.)
	   Setup could be done in an initialization procedure. */

	// describe how data will be stored in memory
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// generate a new "texture object" and select it for setup
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// load an image into memory
	load_ppm_p6(filename, &texwidth, &texheight, &imagedata, 1);

	// describe the image to the graphics system as a texture map
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0,
	             GL_RGB, GL_UNSIGNED_BYTE, imagedata);

	// select methods for "scaling" a texture region to a pixel
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// select the method for combining texture color with the lighting equation
	// (look up the third parameter)
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// NOTE: to have another texture map, generate another texture object and
	//       repeat the setup steps. To select which texture is being applied
	//       during drawing, use glBindTexture() to select.
}

void display_depth_texture ()
{
	glPushMatrix ();
	// paint the depth texture
	glTranslatef (10.0, 10.0, 25.0);
	glRotatef (270, 1, 0, 0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glNormal3f(0.0, 1.0, 0.0);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
 	glTexCoord2f(1.0, 1.0);
	glVertex3f(-10.0, 0.0, 10.0);
 	glTexCoord2f(1.0, 0.0);
	glVertex3f(-10.0, 0.0, -10.0);
 	glTexCoord2f(0.0, 0.0);
	glVertex3f(10.0, 0.0, -10.0);
 	glTexCoord2f(0.0, 1.0);
	glVertex3f(10.0, 0.0, 10.0);
	glEnd();
	glDisable (GL_TEXTURE_2D);
	glPopMatrix ();
}

// helper function to convert a 4x4 GLfloat array into a gmtl 4x4 matrix
void convert_GLfloat44_to_Matrix44f (GLfloat source[4][4], gmtl::Matrix44f &output){
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			output[i][j] = source[i][j];
		}
	}
}

// helper function that replaces the first three values in the last row
// of a 4x4 homogeneous matrix, specified by the parameters
void set_matrix_position(gmtl::Matrix44f &A, float x, float y, float z){
	A[0][3] = x;
	A[1][3] = y;
	A[2][3] = z;
}

// helper function that gets the first three values in the last row
// of a 4x4 homogeneous matrix
void get_matrix_position(gmtl::Matrix44f A, float &x, float &y, float &z){
	x = A[0][3];
	y = A[1][3];
	z = A[2][3];
}

// translates a 4x4 homogeneous matrix with respect to world
// coordinate frame (the transformation is on the left of the multiplication)
void translate_world(gmtl::Matrix44f &A, float dx, float dy, float dz){
	gmtl::Matrix44f trans;

	trans[0][3] = dx;
	trans[1][3] = dy;
	trans[2][3] = dz;

	A = trans * A;
}

// just to keep my ducks in a row, multiplies a matrix such that it is
// "about the world axis"
void multiply_world(gmtl::Matrix44f &source, gmtl::Matrix44f mult){
	source = mult * source;
}

// just to keep my ducks in a row, multiplies a matrix such that it is
// "about the local axis"
void multiply_local(gmtl::Matrix44f &source, gmtl::Matrix44f mult){
	source = source * mult;
}

// translates a 4x4 homogeneous matrix with respect to local
// coordinate frame (the transformation is on the right of the multiplication)
void translate_local(gmtl::Matrix44f &A, float dx, float dy, float dz){
	gmtl::Matrix44f trans;

	trans[0][3] = dx;
	trans[1][3] = dy;
	trans[2][3] = dz;

	A = A * trans;
}

// rotates a 4x4 homogeneous matrix with respect to world
// coordinate frame (the transformation is on the left of the multiplication)
void rotate_world(gmtl::Matrix44f &A, float degX, float degY, float degZ){
	gmtl::Matrix44f rot;
	float radians = 0.0;
	float cos, sin;

	if(degX != 0){
		radians = gmtl::Math::deg2Rad(degX);
	}
	else if(degY != 0){
		radians = gmtl::Math::deg2Rad(degY);
	}
	else if(degZ != 0){
		radians = gmtl::Math::deg2Rad(degZ);
	}

	// precalc
	cos = gmtl::Math::cos(radians);
	sin = gmtl::Math::sin(radians);

	// this cannot be used, but results in the same matrix
	//rot = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(gmtl::Math::deg2Rad(degX), gmtl::Math::deg2Rad(degY) , gmtl::Math::deg2Rad(degZ)));

	// only one of the degree values should be non-zero
	// set the rotation matrix up for a rotation about the x axis
	if(degX != 0){
		rot[1][1] = cos;
		rot[1][2] = -sin;
		rot[2][1] = sin;
		rot[2][2] = cos;
	}
	// set the rotation matrix up for a rotation about the y axis
	else if(degY != 0){
		rot[0][0] = cos;
		rot[0][2] = sin;
		rot[2][0] = -sin;
		rot[2][2] = cos;
	}
	// set the rotation matrix up for a rotation about the x axis
	else if(degZ != 0){
		rot[0][0] = cos;
		rot[0][1] = -sin;
		rot[1][0] = sin;
		rot[1][1] = cos;
	}

	A = rot * A;
}

// rotates a 4x4 homogeneous matrix with respect to local
// coordinate frame (the transformation is on the right of the multiplication)
void rotate_local(gmtl::Matrix44f &A, float degX, float degY, float degZ){
	gmtl::Matrix44f rot;
	float radians = 0.0;
	float cos, sin;

	if(degX != 0){
		radians = gmtl::Math::deg2Rad(degX);
	}
	else if(degY != 0){
		radians = gmtl::Math::deg2Rad(degY);
	}
	else if(degZ != 0){
		radians = gmtl::Math::deg2Rad(degZ);
	}

	// precalc
	cos = gmtl::Math::cos(radians);
	sin = gmtl::Math::sin(radians);

	// this cannot be used, but results in the same matrix
	//rot = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(gmtl::Math::deg2Rad(degX), gmtl::Math::deg2Rad(degY) , gmtl::Math::deg2Rad(degZ)));

	// only one of the degree values should be non-zero
	// set the rotation matrix up for a rotation about the x axis
	if(degX != 0){
		rot[1][1] = cos;
		rot[1][2] = -sin;
		rot[2][1] = sin;
		rot[2][2] = cos;
	}
	// set the rotation matrix up for a rotation about the y axis
	else if(degY != 0){
		rot[0][0] = cos;
		rot[0][2] = sin;
		rot[2][0] = -sin;
		rot[2][2] = cos;
	}
	// set the rotation matrix up for a rotation about the x axis
	else if(degZ != 0){
		rot[0][0] = cos;
		rot[0][1] = -sin;
		rot[1][0] = sin;
		rot[1][1] = cos;
	}

	A = A * rot;
}


// ***********************
// draw cockpit
// ***********************
void draw_helicopter_cockpit(){
	// legacy variables
	float xpos = 0.0;
	float ypos = 0.0;
	float zpos = 0.0;

	if(texture_heli_front == -1)
		setup_texture("heli_front.ppm", texture_heli_front);
	if(texture_heli_side == -1)
		setup_texture("heli_side.ppm", texture_heli_side);

	// bottom
	glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
	glVertex3f(1.0+xpos, 0.0+ypos, 0.0+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 150.0/255.0);
	glVertex3f(1.0+xpos, 0.0+ypos, 1.0+zpos);
	glVertex3f(0.0+xpos, 0.0+ypos, 1.0+zpos);
	glEnd ();

	// top
	glBegin(GL_QUADS);
	glColor3f(0.0/255.0, 0.0/255.0, 20.0/255.0);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 1.0+zpos);
	glVertex3f(1.0+xpos, 1.0+ypos, 1.0+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 150.0/255.0);
	glVertex3f(1.0+xpos, 1.0+ypos, 0.0+zpos);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.0+zpos);
	glEnd();

	// back side
	glBegin (GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
	glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 150.0/255.0);
	glVertex3f(0.0+xpos, 0.0+ypos, 1.0+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 20.0/255.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 1.0+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 150.0/255.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.0+zpos);
	glEnd();

	// front side
	glPushMatrix ();
	// turn on texture mapping and specify which texture will be used:
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_heli_front);
	glColor3f(255.0, 255.0, 255.0);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0+xpos, 0.0+ypos, 0.0+zpos);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0+xpos, 1.0+ypos, 0.0+zpos);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0+xpos, 1.0+ypos, 1.0+zpos);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0+xpos, 0.0+ypos, 1.0+zpos);
	glEnd();
	glDisable(GL_TEXTURE_2D); // not really needed if everything is texture mapped
	glPopMatrix ();

	// left side
	glPushMatrix ();
	// turn on texture mapping and specify which texture will be used:
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_heli_side);
	glColor3f(255.0, 255.0, 255.0);
	glBegin(GL_QUADS);
	glNormal3f( 0.0,  0.0, -1.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0+xpos, 0.0+ypos, 0.0+zpos);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0+xpos, 0.0+ypos, 0.0+zpos);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0+xpos, 1.0+ypos, 0.0+zpos);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.0+zpos);
	glEnd();
	glDisable(GL_TEXTURE_2D); // not really needed if everything is texture mapped
	glPopMatrix ();

	// right side
	glPushMatrix ();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_heli_side);
	glBegin(GL_QUADS);
	glNormal3f( 0.0,  0.0, 1.0);
	glColor3f(255.0, 255.0, 255.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0+xpos, 0.0+ypos, 1.0+zpos);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0+xpos, 0.0+ypos, 1.0+zpos);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 1.0+zpos);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0+xpos, 1.0+ypos, 1.0+zpos);
	glEnd();
	glDisable(GL_TEXTURE_2D); // not really needed if everything is texture mapped
	glPopMatrix ();
}



// ***********************
// draw tail
// ***********************
void draw_helicopter_tail(){
	// legacy variables
	float xpos = 0.0;
	float ypos = 0.0;
	float zpos = 0.0;

	// tail on backside, contains two pieces that make a wedge shape
	glBegin(GL_TRIANGLES);
	// left side of tail, from rear
	glNormal3f(0.0, -3/gmtl::Math::sqrt(34), -5/gmtl::Math::sqrt(34));
	glColor3f(0.0/255.0, 200.0/255.0, 150.0/255.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.3+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
	glVertex3f(0.0+xpos, 0.3+ypos, 0.5+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(-1.3+xpos, 1.0+ypos, 0.5+zpos);

	// right side of tail, from rear
	glNormal3f(0.0, -3/gmtl::Math::sqrt(34), 5/gmtl::Math::sqrt(34));
	glColor3f(200.0/255.0, 200.0/255.0, 150.0/255.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.7+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 150.0/255.0);
	glVertex3f(0.0+xpos, 0.3+ypos, 0.5+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(-1.3+xpos, 1.0+ypos, 0.5+zpos);

	// top side of tail
	glNormal3f(0.0, 1.0, 0.0);
	glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.3+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 100.0/255.0);
	glVertex3f(0.0+xpos, 1.0+ypos, 0.7+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(-1.3+xpos, 1.0+ypos, 0.5+zpos);
	glEnd();
}

// ***********************
// draw rear prop
// ***********************
void draw_helicopter_rear_propeller(){
	// legacy variables
	float xpos = 0.0;
	float ypos = 0.0;
	float zpos = 0.0;

	// simple prop on backside
	glBegin(GL_TRIANGLES);

	// connector for prop
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(xpos, ypos, zpos);
	glVertex3f(-0.02+xpos, ypos, 0.1+zpos);
	glVertex3f(0.02+xpos, ypos, 0.1+zpos);

	// top end of prop
	glNormal3f(0.0, 0.0, 1.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(0.05+xpos, 0.2+ypos, 0.1+zpos);
	glColor3f(50.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(0.1+xpos, 0.2+ypos, 0.1+zpos);
	glColor3f(0.0/255.0, 50.0/255.0, 0.0/255.0);
	glVertex3f(xpos, ypos, 0.1+zpos);

	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(0.05+xpos, 0.2+ypos, 0.11+zpos);
	glColor3f(50.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(0.1+xpos, 0.2+ypos, 0.11+zpos);
	glColor3f(0.0/255.0, 50.0/255.0, 0.0/255.0);
	glVertex3f(xpos, ypos, 0.1+zpos);

	// bottom end of prop
	glNormal3f(0.0, 0.0, -1.0);
	glColor3f(0.0/255.0, 50.0/255.0, 0.0/255.0);
	glVertex3f(xpos, ypos, 0.1+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 50.0/255.0);
	glVertex3f(-0.1+xpos, -0.2+ypos, 0.1+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 50.0/255.0);
	glVertex3f(-0.05+xpos, -0.2+ypos, 0.1+zpos);

	glColor3f(0.0/255.0, 50.0/255.0, 0.0/255.0);
	glVertex3f(xpos, ypos, 0.11+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 50.0/255.0);
	glVertex3f(-0.1+xpos, -0.2+ypos, 0.11+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 50.0/255.0);
	glVertex3f(-0.05+xpos, -0.2+ypos, 0.1+zpos);
	glEnd();
}

// ***********************
// draw skiis
// ***********************
void draw_helicopter_skiis(){
	// legacy variables
	float xpos = 0.0;
	float ypos = 0.0;
	float zpos = 0.0;

	// skiis underneath helicopter
	glBegin(GL_QUADS);
	// right ski
	glNormal3f(0.0, 0.0, 1.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(-0.2+xpos, -0.1+ypos, 0.8+zpos);
	glVertex3f(-0.2+xpos, -0.2+ypos, 0.8+zpos);
	glColor3f(140.0/255.0, 140.0/255.0, 140.0/255.0);
	glVertex3f(1.2+xpos, -0.1+ypos, 0.8+zpos);
	glVertex3f(1.2+xpos, -0.2+ypos, 0.8+zpos);

	// left ski
	glNormal3f(0.0, 0.0, -1.0);
	glColor3f(140.0/255.0, 140.0/255.0, 140.0/255.0);
	glVertex3f(-0.2+xpos, -0.1+ypos, 0.2+zpos);
	glVertex3f(-0.2+xpos, -0.2+ypos, 0.2+zpos);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(1.2+xpos, -0.1+ypos, 0.2+zpos);
	glVertex3f(1.2+xpos, -0.2+ypos, 0.2+zpos);
	glEnd();

	// ski connectors
	glBegin(GL_LINES);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);

	// right ski connector back
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(0.2+xpos, 0.0+ypos, 0.8+zpos);
	glVertex3f(0.2+xpos, -0.1+ypos, 0.8+zpos);

	// right ski connector front
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(0.8+xpos, 0.0+ypos, 0.8+zpos);
	glVertex3f(0.8+xpos, -0.1+ypos, 0.8+zpos);

	// left ski connector back
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(0.2+xpos, 0.0+ypos, 0.2+zpos);
	glVertex3f(0.2+xpos, -0.1+ypos, 0.2+zpos);

	// left ski connector front
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(0.8+xpos, 0.0+ypos, 0.2+zpos);
	glVertex3f(0.8+xpos, -0.1+ypos, 0.2+zpos);
	glEnd();
}

// ***********************
// draw top prop
// ***********************
void draw_helicopter_main_propeller(){
	// legacy variables
	float xpos = 0.0;
	float ypos = 0.0;
	float zpos = 0.0;

	glBegin(GL_TRIANGLES);
	// prop connector
	glNormal3f(0.0, 0.0, 1.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, ypos, zpos); //bottom
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.1+xpos, 0.15+ypos, -0.1+zpos); // top
	glVertex3f(0.1+xpos, 0.15+ypos, 0.1+zpos); // top

	// one side
	glNormal3f(0.0, 1.0, 0.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, 0.15+ypos, zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(0.8+xpos, 0.15+ypos, 0.9+zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 50.0/255.0);
	glVertex3f(0.9+xpos, 0.05+ypos, 0.8+zpos);

	glNormal3f(0.0, -3/gmtl::Math::sqrt(34), 5/gmtl::Math::sqrt(34));
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, 0.15+ypos, zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(0.8+xpos, 0.15+ypos, 0.9+zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 50.0/255.0);
	glVertex3f(0.9+xpos, 0.15+ypos, 0.8+zpos);

	glNormal3f(0.0, -3/gmtl::Math::sqrt(34), -5/gmtl::Math::sqrt(34));
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, 0.15+ypos, zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(0.9+xpos, 0.05+ypos, 0.8+zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 50.0/255.0);
	glVertex3f(0.9+xpos, 0.15+ypos, 0.8+zpos);

	glNormal3f(0.0, 0.0, 1.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(0.8+xpos, 0.15+ypos, 0.9+zpos);
	glVertex3f(0.9+xpos, 0.05+ypos, 0.8+zpos);
	glVertex3f(0.9+xpos, 0.15+ypos, 0.8+zpos);

	// and the other
	glNormal3f(0.0, 1.0, 0.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, 0.15+ypos, zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.8+xpos, 0.15+ypos, -0.9+zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.9+xpos, 0.05+ypos, -0.8+zpos);

	glNormal3f(0.0, -3/gmtl::Math::sqrt(34), 5/gmtl::Math::sqrt(34));
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, 0.15+ypos, zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.8+xpos, 0.15+ypos, -0.9+zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.9+xpos, 0.15+ypos, -0.8+zpos);

	glNormal3f(0.0, -3/gmtl::Math::sqrt(34), -5/gmtl::Math::sqrt(34));
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, 0.15+ypos, zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.9+xpos, 0.05+ypos, -0.8+zpos);
	glColor3f(226.0/255.0, 113.0/255.0, 0.0/255.0);
	glVertex3f(-0.9+xpos, 0.15+ypos, -0.8+zpos);

	glNormal3f(0.0, 0.0, -1.0);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(-0.8+xpos, 0.15+ypos, -0.9+zpos);
	glVertex3f(-0.9+xpos, 0.05+ypos, -0.8+zpos);
	glVertex3f(-0.9+xpos, 0.15+ypos, -0.8+zpos);
	glEnd();
}

// ***********************
// draw window
// ***********************
void draw_helicopter_window(){
	// legacy variables
	float xpos = 0.0;
	float ypos = 0.0;
	float zpos = 0.0;

	// draw window on right side of cockpit
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	// bottom left corner
	glColor3f(30.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(-1.0+xpos, ypos, zpos);

	// bottom right corner
	glColor3f(30.0/255.0, 0.0/255.0, 0.0/255.0);
	glVertex3f(xpos, ypos, zpos);

	// top right corner
	glColor3f(50.0/255.0, 50.0/255.0, 50.0/255.0);
	glVertex3f(xpos, .22+ypos, zpos);

	// top left corner
	glColor3f(50.0/255.0, 50.0/255.0, 50.0/255.0);
	glVertex3f(-1.0+xpos, 0.7+ypos, zpos);
	glEnd();
}

void draw_helicopter_axes(){
	glBegin(GL_LINES);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);	// draw x axis
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(2, 0.0, 0.0);
	glColor3f(255.0/255.0, 0.0/255.0, 0.0/255.0);	// draw y axis
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 2, 0.0);
	glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);	// draw z axis
	glVertex3f(0.0, 0.0, 0);
	glVertex3f(0.0, 0.0, 2);
	glColor3f(0.0/255.0, 0.0/255.0, 0.0/255.0);	// draw x label
	glVertex3f(2, .3, .2);
	glVertex3f(2, -.3, -.2);
	glVertex3f(2, .3, -.2);
	glVertex3f(2, -.3, .2);
	glColor3f(255.0/255.0, 0.0/255.0, 0.0/255.0);	// draw y label
	glVertex3f(0.0, 2, 0.0);
	glVertex3f(.2, 2, .3);
	glVertex3f(0.0, 2, 0.0);
	glVertex3f(-.2, 2, .3);
	glVertex3f(0.0, 2, 0.0);
	glVertex3f(0.0, 2, -.5);
	glColor3f(0.0/255.0, 0.0/255.0, 255.0/255.0);	// draw z label
	glVertex3f(-.3, .3, 2);
	glVertex3f(.3, .3, 2);
	glVertex3f(-.3, -.3, 2);
	glVertex3f(.3, -.3, 2);
	glVertex3f(.3, .3, 2);
	glVertex3f(-.3, -.3, 2);
	glEnd();
}

void draw_ground(){
	if(texture_grass == -1)
		setup_texture("grass.ppm", texture_grass);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_grass);
	glColor3f(1,1,1);
	glNormal3f(0.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 1.0);	glVertex3f(-50.0, -0.0005, 50.0);
	glTexCoord2f(1.0, 0.0);	glVertex3f(-50.0, -0.0005, -50.0);
	glTexCoord2f(0.0, 0.0);	glVertex3f(50.0, -0.0005, -50.0);
	glTexCoord2f(0.0, 1.0);	glVertex3f(50.0, -0.0005, 50.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

// this just draws a set of axes for the world coordinate system
// with the x,y, and z axes each color coded to the same color
// as in the local coordinate systems of the helicopters
// black - x
// red - y
// blue - z
void draw_axes_world(){
	float scale, xpos, ypos, zpos;

	// draw the world coordinate axes
	// they are color coded different
	// from the local coordinate axes
	xpos = ypos = zpos = 0.0;
	scale = 50.0;
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
	glEnd();
}

// draws a cone with specified height and radius
// degrees_precision means how accurate an approximation
// you would like it to a perfect cone
// height cannot be zero
void draw_cone_lighted(float height, float radius, float degrees_precision, norm array[], bool &array_initialized){
	float radians, x, y, theta = degrees_precision, total;
	float oldx = radius, oldy = 0.0;
	int counter = 0;
	float counter_total = (360.0/degrees_precision);
	bool total_calculated = false;

	if(texture_bricks)
		setup_texture("bricks.ppm", texture_bricks);

	// turn on texture mapping and specify which texture will be used:
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_bricks);

	// begin drawing
	glBegin(GL_TRIANGLES);
	while(theta <= 360.0){
		// calculate radians from degrees theta
		radians = gmtl::Math::deg2Rad(float(theta));
		// calculate the new x, y coordinates
		x = (radius*gmtl::Math::cos(radians));
		y = (radius*gmtl::Math::sin(radians));

		if(!array_initialized){
			gmtl::Vec3f A(x-oldx,0.0,y-oldy), B(-oldx,height,-oldy), C;
			gmtl::cross(C,B,A);
			if(!total_calculated){
				total = gmtl::Math::sqrt(C[0]*C[0] + C[1]*C[1] + C[2]*C[2]);
				total_calculated = true;
			}
			array[counter].x = C[0]/total;
			array[counter].y = C[1]/total;
			array[counter].z = C[2]/total;
		}
		glNormal3f(array[counter].x, array[counter].y, array[counter].z);
		counter++;

		glTexCoord2f(counter/counter_total, 0.0);
		glVertex3f(oldx, 0.0, oldy);

		glTexCoord2f(counter/counter_total, 1.0);
		glVertex3f(0.0, height, 0.0);

		glTexCoord2f((counter+1)/counter_total, 0.0);
		glVertex3f(x, 0.0, y);

		// update theta
		theta += degrees_precision;
		oldx = x;
		oldy = y;
	}
	glEnd();
	array_initialized = true;
	glDisable(GL_TEXTURE_2D);
}

void draw_helicopter_1(){
	// **************************************************************************
	// first helicopter

	// save modelview state
	glPushMatrix();
	// translate to correct spot
	glTranslatef(0.0,0.7,0.0);
	// pass the homogeneous matrix to multiply by the modelview
	glMultMatrixf(helicopter1_main.getData());

	glPushMatrix();
	glTranslatef(-0.5, -0.5, -0.5);
	draw_helicopter_cockpit();
	draw_helicopter_tail();
	draw_helicopter_skiis();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.5, -0.2, 0.5005);
	glMultMatrixf(helicopter1_window.getData());
	draw_helicopter_window();
	if(drawminiaxes)
		draw_helicopter_axes();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.8,0.5,0.0);
	glMultMatrixf(helicopter1_rearprop.getData());
	draw_helicopter_rear_propeller();
	if(drawminiaxes)
		draw_helicopter_axes();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.5, 0.0);
	glMultMatrixf(helicopter1_topprop.getData());
	draw_helicopter_main_propeller();
	if(drawminiaxes)
		draw_helicopter_axes();
	glPopMatrix();

	if(drawheliaxes)
		draw_helicopter_axes();

	// restore state of modelview
	glPopMatrix();
}

void draw_helicopter_2(){
	// **************************************************************************
	// second helicopter

	// spacing between the two helicopters
	float translation_amount = 4.0;

	// save modelview state
	glPushMatrix();
	// translate to correct spot
	glTranslatef(translation_amount,3.7,3.0);
	// pass the homogeneous matrix to multiply by the modelview
	glMultMatrixf(helicopter2_main.getData());

	glPushMatrix();
	glTranslatef(-0.5, -0.5, -0.5);
	draw_helicopter_cockpit();
	draw_helicopter_tail();
	draw_helicopter_skiis();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.5, -0.2, 0.5005);
	glMultMatrixf(helicopter2_window.getData());
	draw_helicopter_window();
	if(drawminiaxes)
		draw_helicopter_axes();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.8,0.5,0.0);
	glMultMatrixf(helicopter2_rearprop.getData());
	draw_helicopter_rear_propeller();
	if(drawminiaxes)
		draw_helicopter_axes();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.5, 0.0);
	glMultMatrixf(helicopter2_topprop.getData());
	draw_helicopter_main_propeller();
	if(drawminiaxes)
		draw_helicopter_axes();
	glPopMatrix();

	if(drawheliaxes)
		draw_helicopter_axes();

	// restore state of modelview
	glPopMatrix();
}

// draw a lighted cone with specified shading, etc
void draw_cone(){
	// for the cone lighting
	const GLfloat color_ambient1[]  = { 0.1, 0.2, 0.2, 1.0 };
	const GLfloat color_diffuse1[]  = { 0.7, 0.7, 0.8, 1.0 };
	const GLfloat color_specular1[] = { 0.0, 1.0, 1.0, 1.0 };
	const GLfloat color_ambient2[]  = { 0.0, 0.0, 0.0, 1.0 };
	const GLfloat color_diffuse2[]  = { 0.0, 0.0, 0.0, 1.0 };
	const GLfloat color_specular2[] = { 0.0, 0.0, 0.0, 1.0 };

	/* set material properties for cone */
	glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
	if(ambient)
		glMaterialfv(GL_FRONT, GL_AMBIENT, color_ambient1);
	else
		glMaterialfv(GL_FRONT, GL_AMBIENT, color_ambient2);
	if(diffuse)
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color_diffuse1);
	else
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color_diffuse2);
	if(specular)
		glMaterialfv(GL_FRONT, GL_SPECULAR, color_specular1);
	else
		glMaterialfv(GL_FRONT, GL_SPECULAR, color_specular2);

	glPushMatrix();
	glTranslatef(1.2, 0.0, -1.2);
	draw_cone_lighted(2.0, 0.5, degrees_precision, coneArray, coneArray_initialized);
	glPopMatrix();

	glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, color_ambient2);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, color_diffuse2);
	glMaterialfv(GL_FRONT, GL_SPECULAR, color_specular2);
}

// draws a sphere where the lightsource originates from
void draw_light_source(){
	static float x,y,z;
	glPushMatrix();
	get_matrix_position(lightsource, x, y, z);
	glTranslatef(x, y, z);
	glColor3f(1.0, 1.0, 0.9);
	glutSolidSphere(0.3, 50, 50);
	glPopMatrix();
}

// here is the heart of the program. The differences in the way the helicopters
// rotate can be seen implemented here
// the world axes are drawn, along with each helicopter in its correct translation
// and rotation from the origin
void draw ()
{
}

// tie in to keypresses and do something with them
// such as....rotate the helicopters
// see comments below for details
void handleKeys(unsigned char key, int x, int y){
	switch(key){
		// positive x, h1
        case '1':
		multiply_local(helicopter1_main, x_rototation_matrix_pos15deg);
		break;
		// negative x, h1
	case 'Q':
		exit (0);
		break;
	case 'q':
		multiply_local(helicopter1_main, x_rototation_matrix_neg15deg);
		break;
		// positive y, h1
        case '2':
		multiply_local(helicopter1_main, y_rototation_matrix_pos15deg);
		break;
		// negative y, h1
        case 'w':
		multiply_local(helicopter1_main, y_rototation_matrix_neg15deg);
		break;
		// positive z, h1
        case '3':
		multiply_local(helicopter1_main, z_rototation_matrix_pos15deg);
		break;
		// negative z, h1
        case 'e':
		multiply_local(helicopter1_main, z_rototation_matrix_neg15deg);
		break;
		// move h1 forward
        case '4':
		translate_local(helicopter1_main, 0.5, 0, 0);
		break;
		// rotate top prop, h1
        case 'r':
		multiply_local(helicopter1_topprop, y_rototation_matrix_neg15deg);
		break;
		// rotate rear prop, h1
        case '5':
		multiply_local(helicopter1_rearprop, z_rototation_matrix_neg15deg);
		break;
		// open/shut window, h1
        case 't':
		if(helicopter1_hingeopen)
			rotate_local(helicopter1_window, 0, -70, 0);
		else
			rotate_local(helicopter1_window, 0, 70, 0);
		helicopter1_hingeopen = !helicopter1_hingeopen;
		break;

		// positive x, h2
        case '8':
		multiply_local(helicopter2_main, x_rototation_matrix_pos15deg);
		break;
		// negative x, h2
        case 'u':
		multiply_local(helicopter2_main, x_rototation_matrix_neg15deg);
		break;
		// positive y, h2
        case '9':
		multiply_local(helicopter2_main, y_rototation_matrix_pos15deg);
		break;
		// negative y, h2
        case 'i':
		multiply_local(helicopter2_main, y_rototation_matrix_neg15deg);
		break;
		// positive z, h2
        case '0':
		multiply_local(helicopter2_main, z_rototation_matrix_pos15deg);
		break;
		// negative z, h2
        case 'o':
		multiply_local(helicopter2_main, z_rototation_matrix_neg15deg);
		break;
		// move h2 forward
        case '7':
		translate_local(helicopter2_main, 0.5, 0, 0);
		break;
		// rotate top prop h2
        case 'y':
		multiply_local(helicopter2_topprop, y_rototation_matrix_neg15deg);
		break;
		// rotate rear prop
        case '6':
		multiply_local(helicopter2_rearprop, z_rototation_matrix_neg15deg);
		break;
		// open/shut window, h2
        case 'p':
		if(helicopter2_hingeopen)
			rotate_local(helicopter2_window, 0, -70, 0);
		else
			rotate_local(helicopter2_window, 0, 70, 0);
		helicopter2_hingeopen = !helicopter2_hingeopen;
		break;

		// turn the drawing of mini axes on/off
        case '-':
		drawminiaxes = !drawminiaxes;
		break;
		// turn the drawing of heli axes on/off
        case '=':
		drawheliaxes = !drawheliaxes;
		break;
		// turn the drawing of main axes on/off
        case '[':
		drawmainaxes = !drawmainaxes;
		break;

		// rotate camera by positive 15 degrees, y
        case 'a':
		multiply_local(camera_rotation_y_inverse, y_rototation_matrix_neg15deg);
		break;
		// rotate camera by negative 15 degrees, y
        case 's':
		multiply_local(camera_rotation_y_inverse, y_rototation_matrix_pos15deg);
		break;
		// rotate camera by positive 15 degrees, x
        case 'd':
		multiply_local(camera_rotation_x_inverse, x_rototation_matrix_neg15deg);
		break;
		// rotate camera by negative 15 degrees, x
        case 'f':
		multiply_local(camera_rotation_x_inverse, x_rototation_matrix_pos15deg);
		break;
		// zoom camera in to helicopter
        case 'g':
		translate_local(camera_translation_z_inverse, 0, 0, 0.5);
		break;
		// zoom camera out from helicopter
        case 'h':
		translate_local(camera_translation_z_inverse, 0, 0, -0.5);
		break;

		// translate lightsource +x
        case 'z':
		translate_world(lightsource, 0.5, 0.0, 0.0);
		break;
		// translate lightsource -x
        case 'x':
		translate_world(lightsource, -0.5, 0.0, 0.0);
		break;
		// translate lightsource +y
        case 'c':
		translate_world(lightsource, 0.0, 0.5, 0.0);
		break;
		// translate lightsource -y
        case 'v':
		translate_world(lightsource, 0.0, -0.5, 0.0);
		break;
		// translate lightsource +z
        case 'b':
		translate_world(lightsource, 0.0, 0.0, 0.5);
		break;
		// translate lightsource -z
        case 'n':
		translate_world(lightsource, 0.0, 0.0, -0.5);
		break;

		// toggle ambient lighting
        case 'j':
		ambient = !ambient;
		break;
		// toggle diffuse lighting
        case 'k':
		diffuse = !diffuse;
		break;
		// toggle specular lighting
        case 'l':
		specular = !specular;
		break;
	}

	// cause a redraw of the helicopters
	glutPostRedisplay();
}

void set_light_position(gmtl::Matrix44f &lightsource)
{
	float x,y,z;
	get_matrix_position(lightsource, x, y, z);
	GLfloat position[] = { x, y, z };
	static const GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };

	// light properties are set for one light.
	glLightfv (GL_LIGHT0, GL_POSITION, position);
	glLightfv (GL_LIGHT0, GL_AMBIENT, white);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv (GL_LIGHT0, GL_SPECULAR, white);
}

// helper function to set the camera positionn
void set_camera_position (gmtl::Matrix44f &referencePoint){
	// these are temp variables to help get helicopter position to update camera
	// position
	float x,y,z;
	gmtl::Matrix44f referencePoint_temp;
	// first get the position data from the first helicopter
	get_matrix_position(referencePoint, x,y,z);
	// and since we're not worrying about doing matrix inversions, just setting
	// them up directly, we'll pass the inverted info to this temp matrix
	// to be passed along to the next function below
	set_matrix_position(referencePoint_temp, -x, -y, -z);

	// this takes a set of camera transposes and multiplies it by the helicopter1's
	// position data to update the camera position
	glMultMatrixf((camera_translation_z_inverse * camera_rotation_x_inverse * camera_rotation_y_inverse * referencePoint_temp ).getData());
}

GLuint empty_texture ()							// Create An Empty Texture
{
	// Texture ID
	GLuint txtnumber;
	// Stored Data
	unsigned int * data;

	// Create Storage Space For Texture Data
	data = (unsigned int *)new GLuint[((W_WIDTH * W_HEIGHT)* 3 * sizeof(unsigned int))];
	// Clear Storage Memory
	glGenTextures (1, &txtnumber);					// Create 1 Texture
	glBindTexture (GL_TEXTURE_2D, txtnumber);			// Bind The Texture
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, W_WIDTH, W_HEIGHT, 0,
		      GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);	// Build Texture Using Information In data

	delete [] data;							// Release data
	return txtnumber;						// Return The Texture ID
}


void display ()
{
	render_from_light ();

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	glMatrixMode (GL_PROJECTION);	glLoadIdentity ();
	glFrustum(l,r,b,t,n,f);
	glMatrixMode (GL_MODELVIEW);	glLoadIdentity ();
 	set_camera_position (helicopter1_main); // set the camera position with respect to some reference point

	glAlphaFunc (GL_GREATER, 0.9);
	glEnable (GL_ALPHA_TEST);

	glEnable (GL_TEXTURE_2D);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glEnable (GL_TEXTURE_GEN_S);
	glEnable (GL_TEXTURE_GEN_T);
	glEnable (GL_TEXTURE_GEN_R);
	glEnable (GL_TEXTURE_GEN_Q);
	glTexGenfv (GL_S, GL_EYE_PLANE, plane_s);
	glTexGenfv (GL_T, GL_EYE_PLANE, plane_t);
	glTexGenfv (GL_R, GL_EYE_PLANE, plane_r);
	glTexGenfv (GL_Q, GL_EYE_PLANE, plane_q);

//	set_light_position (lightsource);

	glMaterialf (GL_FRONT, GL_SHININESS, 50.0);
	glMaterialfv (GL_FRONT, GL_AMBIENT, color_ambient);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, color_diffuse);
	glMaterialfv (GL_FRONT, GL_SPECULAR, color_specular);
	glEnable (GL_LIGHTING);
	glEnable (GL_COLOR_MATERIAL);
	draw_helicopter_1 ();
	draw_helicopter_2 ();
	draw_ground ();
	glDisable (GL_COLOR_MATERIAL);
	glDisable (GL_LIGHTING);
//	draw_light_source ();
	glDisable (GL_ALPHA_TEST);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
	glDisable (GL_TEXTURE_GEN_R);
	glDisable (GL_TEXTURE_GEN_Q);

//	display_depth_texture ();

	glutSwapBuffers (); // not glFlush ();
}

/*
  The shadow matrix belongs on modelview. Even though it does a projection, this
  projection is not related to viewing. Instead, it is used as a modeling matrix
  to transform an object to create a shadow object.

  The exact position of the shadow matrix in the sequence depends on which
  coordinate system you use to define your ground plane and light position.
  Most likely, you describe the plane equation and light position in world coords.
  In that case, the value of modelview right before you add the shadow matrix
  should be the same as the value of modelview when you draw the world axes. After
  adding the shadow matrix, you can traverse the scene from the world down to draw
  those objects that you want transformed into shadows.
*/

void render_from_light ()
{
	glShadeModel (GL_FLAT);
	glDisable (GL_LIGHTING);
	glDisable (GL_COLOR_MATERIAL);
	glDisable (GL_NORMALIZE);
//	glColorMask (0, 0, 0, 0);

	glEnable (GL_DEPTH_TEST);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (l,r,b,t,n,f);
	glGetFloatv (GL_PROJECTION_MATRIX, lproj.mData); // save the projection view matrix

	// render scene from light's pov
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	set_camera_position (lightsource);
	glGetFloatv (GL_MODELVIEW_MATRIX, lview.mData); // save the modelview matrix from light

	glPushMatrix ();
	draw_helicopter_1 ();
	draw_helicopter_2 ();
	glPopMatrix ();

	static float x,y,z;
	static gmtl::Matrix44f refpt;
	get_matrix_position(helicopter1_main, x,y,z);
	set_matrix_position(refpt, x, y, z);
	static gmtl::Matrix44f cam_inv = (camera_translation_z_inverse
				   * camera_rotation_x_inverse
				   * camera_rotation_y_inverse
				   * refpt );
	for (int i = 0; i < 3; i++)
	{
		scale_and_bias[i][i] = 0.5;
		scale_and_bias[3][i] = 0.5;
	}
	scale_and_bias[3][3] = 1.0;
	coords = scale_and_bias * lproj * lview * cam_inv;
	plane_s[0] = coords[0][0]; plane_s[1] = coords[1][0]; plane_s[2] = coords[2][0]; plane_s[3] = coords[3][0];
	plane_t[0] = coords[0][1]; plane_t[1] = coords[1][1]; plane_t[2] = coords[2][1]; plane_t[3] = coords[3][1];
	plane_r[0] = coords[0][2]; plane_r[1] = coords[1][2]; plane_r[2] = coords[2][2]; plane_r[3] = coords[3][2];
	plane_q[0] = coords[0][3]; plane_q[1] = coords[1][3]; plane_q[2] = coords[2][3]; plane_q[3] = coords[3][3];

	// capture the current "view" in a depth texture
	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable (GL_TEXTURE_GEN_S);
	glEnable (GL_TEXTURE_GEN_T);
	glEnable (GL_TEXTURE_GEN_R);
	glEnable (GL_TEXTURE_GEN_Q);
	glTexGenfv (GL_S, GL_EYE_PLANE, plane_s);
	glTexGenfv (GL_T, GL_EYE_PLANE, plane_t);
	glTexGenfv (GL_R, GL_EYE_PLANE, plane_r);
	glTexGenfv (GL_Q, GL_EYE_PLANE, plane_q);

	glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni (GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni (GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, W_WIDTH, W_HEIGHT, 0);

	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
	glDisable (GL_TEXTURE_GEN_R);
	glDisable (GL_TEXTURE_GEN_Q);
	glDisable (GL_DEPTH_TEST);

}

void init ()
{
	// set up the initial camera position
	rotate_local (camera_rotation_x_inverse, 15.0, 0.0, 0.0);
	rotate_local (camera_rotation_y_inverse, 0.0, -15.0, 0.0);
	translate_local (camera_translation_z_inverse, 0.0, 0.0, -15.0);

	// set up the initial light position
	translate_world (lightsource, 13.0, 13.0, 13.0);

	// precompute some matrices
	rotate_local (x_rototation_matrix_pos15deg, 15, 0, 0);
	rotate_local (x_rototation_matrix_neg15deg, -15, 0, 0);
	rotate_local (y_rototation_matrix_pos15deg, 0, 15, 0);
	rotate_local (y_rototation_matrix_neg15deg, 0, -15, 0);
	rotate_local (z_rototation_matrix_pos15deg, 0, 0, 15);
	rotate_local (z_rototation_matrix_neg15deg, 0, 0, -15);

	// setup frustum view values
	// left, right, bottom, top, near, far
	/* set up the "lens" for the camera
	  left, right - Specify the coordinates for the left and right vertical clipping planes.
	  bottom, top - Specify the coordinates for the bottom and top horizontal clipping planes.
	  near, far - Specify the distances to the near and far depth clipping planes.
	  Both distances must be positive.
	*/
	l = b = -1.0;
	r = t = n = 1.0;
	f = 500.0;

	depth_texture = empty_texture ();
	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

// 	glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,
// 				   GL_TEXTURE_2D, depth_texture, 0)
	render_from_light ();

	// background is white
	glClearColor (0.9,0.9,1.0,0);
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);
	glShadeModel (GL_SMOOTH);
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

int main(int argc, char **argv)
{
	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (W_WIDTH, W_HEIGHT);
	glutCreateWindow("CRAP PROGRAM");

	glutDisplayFunc (display);
	glutReshapeFunc (reshape);
	glutKeyboardFunc (handleKeys);
	init();
	glutMainLoop();

	return 0;
}

// helicopter 1:
// 1, rotate positive x
// 2, rotate positive y
// 3, rotate positive z
// q, rotate negative x
// w, rotate negative y
// e, rotate negative z
// 4, move helicopter 'forward'
// r, rotate top prop
// 5, rotate rear prop
// t, open/shut window

// helicopter 2:
// 8, rotate positive x
// 9, rotate positive y
// 0, rotate positive z
// u, rotate negative x
// i, rotate negative y
// o, rotate negative z
// 7, movie helicopter 'forward'
// y, rotate top prop
// 6, rotate rear prop
// p, open/shut window

// axes:
// -, toggle object axes on/off
// =, toggle helicopter axes on/off
// [, toggle world axes on/off

// lightsource:
// z, translate lightsource +x
// x, translate lightsource -x
// c, translate lightsource +y
// v, translate lightsource -y
// b, translate lightsource +z
// n, translate lightsource -z
// j, toggle ambient lighting
// k, toggle diffuse lighting
// l, toggle specular lighting

// camera
// a, rotate camera by positive 15 degrees, y
// s, rotate camera by negative 15 degrees, y
// d, rotate camera by positive 15 degrees, x
// f, rotate camera by negative 15 degrees, x
// g, zoom camera in to helicopter
// h, zoom camera out from helicopter

