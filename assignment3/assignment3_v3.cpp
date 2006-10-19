#define GMTL_NO_METAPROG
#define W_WIDTH  800
#define W_HEIGHT 800

#include <string>
#include <cmath>
#include <GL/glut.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>

using namespace std;

GLuint tex = -1;
GLuint depth_texture = -1;
gmtl::Matrix44f scale_and_bias;
gmtl::Matrix44f cproj;
gmtl::Matrix44f lproj; // the light projection matrix
gmtl::Matrix44f cview;
gmtl::Matrix44f lview; // the light view matrix
gmtl::Matrix44f cview_inv; // camera view inverse matrix
gmtl::Matrix44f coords;
float plane_s[4];
float plane_t[4];
float plane_r[4];
float plane_q[4];
float light_pos[3];
float camera_pos[3];
float scene_bounding_radius = 100.0;

float color_specular[] = { 0.0, 0.0, 0.0, 1.0 };
float color_ambient[]  = { 0.2, 0.2, 0.2, 1.0 };
float color_low_ambient[] = { 0.01, 0.01, 0.01, 1.0 };
float color_diffuse[]  = { 0.7, 0.7, 0.7, 1.0 };
float color_low_diffuse[] = { 0.25, 0.25, 0.25, 1.0 };
float color_none[] = { 0.0, 0.0, 0.0, 1.0 };

void print_matrix (gmtl::Matrix44f m, string info);
void reshape(int w, int h);
void handleKeys(unsigned char key, int x, int y);
void draw_box (double size);
void draw_plane (double size);
void draw ();
void init ();
GLuint empty_texture (GLint type);
void render_from_light ();
void display(void);
void display_texture (GLuint tex, GLenum format, double size);

bool print_matrix_used = false;
void print_matrix (gmtl::Matrix44f m, string info)
{
	printf ("%s:\n", info.c_str ());
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(j==0) printf ("[\t");
			printf ("%f\t", m[j][i]);
		}
		printf ("]\n");
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void handleKeys(unsigned char key, int x, int y){
	switch(key){
	case 'q':
		exit (0);
		break;
        case 'a':
		camera_pos[0] -= 1.0;
		break;
        case 's':
		camera_pos[0] += 1.0;
		break;
        case 'd':
		camera_pos[1] -= 1.0;
		break;
        case 'f':
		camera_pos[1] += 1.0;
		break;
        case 'g':
		camera_pos[2] -= 1.0;
		break;
        case 'h':
		camera_pos[2] += 1.0;
		break;
        case 'z':
		light_pos[0] += 0.5;
		break;
        case 'x':
		light_pos[0] -= 0.5;
		break;
        case 'c':
		light_pos[1] += 0.5;
		break;
        case 'v':
		light_pos[1] -= 0.5;
		break;
        case 'b':
		light_pos[2] += 0.5;
		break;
        case 'n':
		light_pos[2] -= 0.5;
		break;
	}
	glutPostRedisplay();
}

void draw_box (double size){
	glColor3f (0.0, 0.0, 1.0);

	glPushMatrix ();
	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(size, 0.0, 0.0);
	glVertex3f(size, 0.0, size);
	glVertex3f(0.0, 0.0, size);

	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, size, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, 0.0);
	glVertex3f(0.0, size, 0.0);

	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, size);
	glVertex3f(0.0, size, size);
	glVertex3f(0.0, size, 0.0);

	glNormal3f(size, 0.0, 0.0);
	glVertex3f(size, 0.0, 0.0);
	glVertex3f(size, size, 0.0);
	glVertex3f(size, size, size);
	glVertex3f(size, 0.0, size);

	glNormal3f( 0.0,  0.0, -1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(size, 0.0, 0.0);
	glVertex3f(size, size, 0.0);
	glVertex3f(0.0, size, 0.0);

	glNormal3f( 0.0,  0.0, 1.0);
	glVertex3f(size, 0.0, size);
	glVertex3f(0.0, 0.0, size);
	glVertex3f(0.0, size, size);
	glVertex3f(size, size, size);

	glEnd();
	glPopMatrix ();
}

void draw_plane (double size){
	glColor3f(0.0,1.0,0.0);
	glPushMatrix ();
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-size, 0.0, -size); glTexCoord2f (-1.0, -1.0);
	glVertex3f(-size, 0.0, size); glTexCoord2f (-1.0, 1.0);
	glVertex3f(size, 0.0, size); glTexCoord2f (1.0, 1.0);
	glVertex3f(size, 0.0, -size); glTexCoord2f (1.0, -1.0);
	glEnd();
	glPopMatrix ();
}

void draw ()
{
	glEnable (GL_LIGHTING);
	glEnable (GL_COLOR_MATERIAL);
	glPushMatrix ();
	glPushMatrix ();
	glTranslatef (0.0, 2.0, 0.0);
	draw_box (1.0);
	glPopMatrix ();
	glPushMatrix ();
	glTranslatef (light_pos[0], light_pos[1], light_pos[2]);
	draw_box (0.5);
	glPopMatrix ();
	draw_plane (5.0);
	glPopMatrix ();
	glDisable (GL_COLOR_MATERIAL);
	glDisable (GL_LIGHTING);
}

void init ()
{
	light_pos[0] = 0.0;
	light_pos[1] = 5.0;
	light_pos[2] = 0.0;
// 	light_pos[0] = 10.0;
// 	light_pos[1] = 10.0;
// 	light_pos[2] = 10.0;
	camera_pos[0] = 10.0;
	camera_pos[1] = 10.0;
	camera_pos[2] = 10.0;

	for (int i = 0; i < 3; i++)
	{
		scale_and_bias[i][i] = 0.5;
		scale_and_bias[3][i] = 0.5;
	}
	scale_and_bias[3][3] = 1.0;

	glEnable (GL_LIGHT0);
	glClearColor (0.9,0.9,1.0,0);

	depth_texture = empty_texture (GL_INTENSITY);
	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	tex = empty_texture (GL_INTENSITY);

	render_from_light ();
}

GLuint empty_texture (GLint type)
{
	GLuint tex;
	unsigned int *data;
	data = (unsigned int*)new GLuint[((W_WIDTH * W_HEIGHT)* 4 * sizeof(unsigned int))];
	glGenTextures (1, &tex);
	glBindTexture (GL_TEXTURE_2D, tex);
	glTexImage2D (GL_TEXTURE_2D, 0, 4, W_WIDTH, W_HEIGHT, 0,
		      type, GL_UNSIGNED_BYTE, data);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	delete [] data;

	return tex;
}

void render_from_light ()
{
	glEnable (GL_DEPTH_TEST);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glShadeModel (GL_FLAT);
	glDisable (GL_LIGHTING);
	glDisable (GL_COLOR_MATERIAL);
	glDisable (GL_NORMALIZE);
	glColorMask (false, false, false, false);

	static float light_to_scene_dist = sqrt (light_pos[0] * light_pos[0] +
						 light_pos[1] * light_pos[1] +
						 light_pos[2] * light_pos[2]);
	static float near_plane = light_to_scene_dist - scene_bounding_radius;
	static float fov = gmtl::Math::deg2Rad (2.0 *
						atan (scene_bounding_radius /
						      light_to_scene_dist));
	glMatrixMode (GL_PROJECTION);	glLoadIdentity ();
 	gluPerspective (60.0, 1.0, 1.0, scene_bounding_radius);
//	gluPerspective (fov, 1.0, near_plane, near_plane + (2.0*scene_bounding_radius));
	glGetFloatv (GL_PROJECTION_MATRIX, lproj.mData); // save the projection view matrix

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	gluLookAt (light_pos[0], light_pos[1], light_pos[2],
		   0.0, 0.0, 0.0,
		   0.0, 1.0, 0.0);
	glGetFloatv (GL_MODELVIEW_MATRIX, lview.mData); // save the modelview matrix from light

	glPushMatrix ();
	draw_box (1.0);
	glPopMatrix ();

	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, W_WIDTH, W_HEIGHT, 0);
}

void display ()
{

	glLightfv (GL_LIGHT0, GL_POSITION, light_pos);

	render_from_light ();
	glColorMask (true, true, true, true);

 	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel (GL_SMOOTH);
	glDepthFunc (GL_LEQUAL);

	glMatrixMode (GL_PROJECTION);	glLoadIdentity ();
 	gluPerspective (60.0, 1.0, 1.0, scene_bounding_radius);
	glGetFloatv (GL_PROJECTION_MATRIX, cproj.mData);

	glMatrixMode (GL_MODELVIEW);	glLoadIdentity ();
	gluLookAt (camera_pos[0], camera_pos[1], camera_pos[2],
		   0.0, 0.0, 0.0,
		   0.0, 1.0, 0.0);

	glGetFloatv (GL_MODELVIEW_MATRIX, cview.mData);
	gmtl::invertFull_GJ (cview_inv, cview);

	coords = scale_and_bias * lproj * lview * cview_inv;
	plane_s[0] = coords[0][0]; plane_s[1] = coords[1][0]; plane_s[2] = coords[2][0]; plane_s[3] = coords[3][0];
	plane_t[0] = coords[0][1]; plane_t[1] = coords[1][1]; plane_t[2] = coords[2][1]; plane_t[3] = coords[3][1];
	plane_r[0] = coords[0][2]; plane_r[1] = coords[1][2]; plane_r[2] = coords[2][2]; plane_r[3] = coords[3][2];
	plane_q[0] = coords[0][3]; plane_q[1] = coords[1][3]; plane_q[2] = coords[2][3]; plane_q[3] = coords[3][3];

	print_matrix (scale_and_bias, "scale_and_bias");
	print_matrix (lproj, "lproj");
	print_matrix (cproj, "cproj");
	print_matrix (lview, "lview");
	print_matrix (cview, "cview");
	print_matrix (cview_inv, "cview_inv");
	print_matrix (coords, "coords");

// 	plane_s[0] = 1.0; plane_s[1] = 0.0; plane_s[2] = 0.0; plane_s[3] = 0.0;
// 	plane_t[0] = 0.0; plane_t[1] = 1.0; plane_t[2] = 0.0; plane_t[3] = 0.0;
// 	plane_r[0] = 0.0; plane_r[1] = 0.0; plane_r[2] = 1.0; plane_r[3] = 0.0;
// 	plane_q[0] = 0.0; plane_q[1] = 0.0; plane_q[2] = 0.0; plane_q[3] = 1.0;

	glAlphaFunc (GL_GREATER, 0.9);
	glEnable (GL_ALPHA_TEST);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, depth_texture);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.5);

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

	glLightfv (GL_LIGHT0, GL_AMBIENT, color_low_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, color_low_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, color_none);
	draw ();

	glMatrixMode (GL_MODELVIEW);
	glLightfv (GL_LIGHT0, GL_AMBIENT, color_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, color_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, color_none);
	draw ();

	glDisable (GL_ALPHA_TEST);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
	glDisable (GL_TEXTURE_GEN_R);
	glDisable (GL_TEXTURE_GEN_Q);

	double rect_size = 3.0;
	glPushMatrix ();
	glRotatef (90, 0, 1, 0);
	glTranslatef (0.0, rect_size, -15.0);
	glRotatef (270, 1, 0, 0);
	glRotatef (180, 0, 0, 1);
	display_texture (tex, GL_RGBA, rect_size);
	glPopMatrix ();

	glPushMatrix ();
	glRotatef (60, 0, 1, 0);
	glTranslatef (0.0, rect_size, -15.0);
	glRotatef (270, 1, 0, 0);
	glRotatef (180, 0, 0, 1);
	display_texture (tex, GL_DEPTH_COMPONENT, rect_size);
	glPopMatrix ();

	glPushMatrix ();
	glRotatef (30, 0, 1, 0);
	glTranslatef (0.0, rect_size, -15.0);
	glRotatef (270, 1, 0, 0);
	glRotatef (180, 0, 0, 1);
	display_texture (tex, GL_LUMINANCE, rect_size);
	glPopMatrix ();

	glPushMatrix ();
	glRotatef (0, 0, 1, 0);
	glTranslatef (0.0, rect_size, -15.0);
	glRotatef (270, 1, 0, 0);
	glRotatef (180, 0, 0, 1);
	display_texture (tex, GL_INTENSITY, rect_size);
	glPopMatrix ();

	glutSwapBuffers ();
}

void display_texture (GLuint tex, GLenum format, double size)
{
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, tex);
	glCopyTexImage2D (GL_TEXTURE_2D, 0, format, 0, 0, W_WIDTH, W_HEIGHT, 1);

	glPushMatrix ();
	glNormal3f(0.0, 1.0, 0.0);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
 	glTexCoord2f(1.0, 1.0);
	glVertex3f(-size, 0.0, size);
 	glTexCoord2f(1.0, 0.0);
	glVertex3f(-size, 0.0, -size);
 	glTexCoord2f(0.0, 0.0);
	glVertex3f(size, 0.0, -size);
 	glTexCoord2f(0.0, 1.0);
	glVertex3f(size, 0.0, size);
	glEnd();
	glDisable (GL_TEXTURE_2D);

	glColor3f (0.0, 0.0, 0.0);
	glBegin (GL_LINE_STRIP);
	glVertex3f(-size, 0.0, size);
	glVertex3f(-size, 0.0, -size);
	glVertex3f(size, 0.0, -size);
	glVertex3f(size, 0.0, size);
	glVertex3f(-size, 0.0, size);
	glEnd ();

	glPopMatrix ();
}

int main(int argc, char **argv)
{
	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (W_WIDTH, W_HEIGHT);
	glutCreateWindow("No shadows :(");

	glutDisplayFunc (display);
	glutReshapeFunc (reshape);
	glutKeyboardFunc (handleKeys);
	init();
	glutMainLoop();

	return 0;
}
