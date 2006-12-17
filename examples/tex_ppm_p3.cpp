/* CMPS 415/515 University of Louisiana at Lafayette

   NOTE: No permission is given for distribution beyond
   the 415/515 class, of this file or any derivative works.
*/

/* --------------------------------------------------------------
   This example draws a texture-mapped quad.
   The file "Image.ppm" is also needed to run this.

   If you want to load an image in a format other than PPM, you can
   convert it to PPM using free tools such as IrfanView on Windows or
   Gimp on Unix. Or, replace LoadPPM with a suitable loading function.

   NOTE: it is strongly recommended to only use images that have widths
   and heights that are powers of 2. Many systems expect this.
   (This requirement was relaxed in OpenGL 2)
*/

#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
//#include <cstdio>
//#include <iostream>

#define W_WIDTH 800
#define W_HEIGHT 600

#define FOVY 20.0
#define ASPECT (W_WIDTH/(double)W_HEIGHT)
#define NEAR 1.0
#define FAR 50.0


using namespace std;
static unsigned int hereb4, texwidth, texheight;
static unsigned char *imagedata;

void load_ppm_p3(char *fname,
	     unsigned int *w,
	     unsigned int *h,
	     unsigned char **data,
	     bool &load_ok)
{
	FILE *fp;
	int max_color_val = 0;
	char P, num;
	char dummy_text[1000];
	unsigned long counter = 0;
	unsigned int val;
	load_ok = false;

	// open file
	if (!(fp = fopen (fname, "r")))
	{
		fprintf (stderr, "Can't open file '%s'\n", fname);
		return;
	}

	// get type of PPM image.
	fscanf(fp, "%c%c\n", &P, &num);
	if ((P != 'P') || (num != '3'))
	{
		fprintf (stderr, "PPM format unknown, should be P3.\n");
		return;
	}

	// get rid of comments in ppm
	do
	{
		fgets(dummy_text, 999, fp);
	}
	while (dummy_text[0] == '#' ||
	       (dummy_text[0] == '/' &&
		(dummy_text[1] == '/' || dummy_text[1] == '*')
		)
	       );


	// get width and height of ppm image
	sscanf(dummy_text, "%d%d", w, h);

	// get max color range, program doesn't do anything with this currently.
	fgets(dummy_text, 999, fp);
	sscanf(dummy_text, "%d", &max_color_val);

	data[0] = new unsigned char[*w * *h * 3];

	while(!feof (fp) || counter <= (*w * *h * 3))
	{
		// stored as contiguous sets of rgb values
		fscanf (fp, "%u", &val); // read in a color value
		data[0][counter] = (char) val; // store in data array
		counter++;
	}

	if (counter > (*w * *h * 3 + 1))
		fprintf (stderr, "PPM file data is corrupted (counter = %d, WxHx3 = %d).\n",
			 counter, (*w * *h * 3));

	fclose(fp);
}

void setlights(void)
{
	static const GLfloat position[] = { 6.0, 8.0, 3.0 },
		white[] = { 1.0, 1.0, 1.0, 1.0 };

		// light properties are set for one light.

		glLightfv(GL_LIGHT0, GL_POSITION, position);
		glLightfv(GL_LIGHT0, GL_AMBIENT, white);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT0, GL_SPECULAR, white);
}

void drawobjects(void)
{
	static GLuint texture;

	const GLfloat color_specular[] = { 0.6, 0.6, 0.6, 1.0 },
                color_ambient[]  = { 0.5, 0.5, 0.5, 1.0 },
			color_diffuse[]  = { 0.8, 0.8, 0.8, 1.0 };

			glMaterialf(GL_FRONT, GL_SHININESS, 80.0); /* set material properties */
			glMaterialfv(GL_FRONT, GL_AMBIENT, color_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, color_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, color_specular);

			if (!hereb4)
			{
				/* NOTE: Only do these setup steps once! (note use of static variables.)
				   Setup could be done in an initialization procedure. */

				// describe how data will be stored in memory
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				// generate a new "texture object" and select it for setup
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);

				// load an image into memory


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

				hereb4 = 1;
			}

			// turn on texture mapping and specify which texture will be used:
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture);

			// For each vertex of a quad, specify a 2D texture coord and 3D vertex coord
			glBegin(GL_QUADS);
			glNormal3f( 0.0,  0.0, 1.0);

			glTexCoord2f(1.0, 0.0);
			glVertex3f( 0.8,  0.0, 0.0);

			glTexCoord2f(0.0, 0.0);
			glVertex3f(0.0,  0.8, 0.0);

			glTexCoord2f(0.0, 1.0);
			glVertex3f(-0.8, 0.0, 0.0);

			glTexCoord2f(1.0, 1.0);
			glVertex3f( 0.0, -0.8, 0.0);
			glEnd();

			glDisable(GL_TEXTURE_2D); // not really needed if everything is texture mapped
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, -10.0, 0.0, 1.0, 0.0);

	setlights();

	glEnable(GL_LIGHTING);
	drawobjects();

	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOVY, ASPECT, NEAR, FAR);
}

void init(void)
{
	glClearColor(0.6, 0.6, 0.6, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	// NOTE: for specular reflections, the "local viewer" model produces better
	// results than the default, but is slower. The default would not use the correct
	// vertex-to-eyepoint vector, treating it as always parallel to Z.
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	bool load_ok;
	load_ppm_p3("Image.ppm", &texwidth, &texheight, &imagedata, load_ok);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(W_WIDTH, W_HEIGHT);

	glutCreateWindow("Bavarian guy");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	init();
	glutMainLoop();

	return 0;
}
