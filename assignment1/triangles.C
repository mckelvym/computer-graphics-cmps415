#include <cstdio>
#include <iostream>
#include <math.h>
#include <GL/glut.h>

//////////////////////////
// ** BEGIN DEFINITIONS **
//////////////////////////

// define window width
#define WIDTH 800
// define window height
#define HEIGHT 600
// create simulated frame buffer
static GLubyte frame_buffer[HEIGHT][WIDTH][3];
// define number of mouse coordinates to hold
#define NUM_COORDINATES 3

// create a struct to hold mouse coordinates
// an x and y coordinate
struct coord{
	int x;
	int y;
};

// create a struct to hold a set of color values
struct color{
	int r;
	int g;
	int b;
};

// array for mouse coordinates
coord mousecoordinates[NUM_COORDINATES];

// amount of coordinates currently in the array
int coordinates_total = 0;

// clear all mouse coordinates from the array
void clearCoordinates(){
	for(int i = 0; i < NUM_COORDINATES; i++){
		mousecoordinates[i].x = -1;
		mousecoordinates[i].y = -1;
	}
}

// set a coordinates in the array specified by index
void setCoordinates(int x, int y, int index){
	if(x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT && index >= 0 && index < NUM_COORDINATES){
		mousecoordinates[index].x = x;
		mousecoordinates[index].y = y;
	}
}

// use a simple bubble sort to sort the coordinates
// descending order
void sortCoordinates(){
	int i, j;
	coord temp;

	for(i = (NUM_COORDINATES - 1); i >= 0; i--){
		for(j = 1; j <= i; j++){
			if(mousecoordinates[j-1].y < mousecoordinates[j].y){
				temp = mousecoordinates[j-1];
				mousecoordinates[j-1] = mousecoordinates[j];
				mousecoordinates[j] = temp;
			}
		}
	}
}

// color the set of coordinates in the framebuffer
// that is held in the coordinates array
// the parameters specify which colors are at each vertex
// and what colors to shade between
void colorCoordinates(color c0, color c1, color c2){
	// sort coordinates in descending order
	sortCoordinates();

	// simplify some typing
	int x0 = mousecoordinates[0].x;
	int x1 = mousecoordinates[1].x;
	int x2 = mousecoordinates[2].x;
	int y0 = HEIGHT-mousecoordinates[0].y;
	int y1 = HEIGHT-mousecoordinates[1].y;
	int y2 = HEIGHT-mousecoordinates[2].y;

	// used in checking for divide by 0 cases
	float denom20 = ((float)(y2-y0));
	float denom10 = ((float)(y1-y0));
	float denom21 = ((float)(y2-y1));

	// used in precalculation to make the program more efficient
	float float20 = 0.0;
	float float10 = 0.0;
	float float21 = 0.0;
	float floati20 = 0.0;
	float floati10 = 0.0;
	float floati21 = 0.0;

	// for the left side of the triangle, and right side, respectively
	float X_L;
	float X_R;

	// for counting through a scanline left to right
	int x;

	// for counting the scanlines of the triangle, bottom to top (basically y)
	int i;

	//colors
	int r_l, r_r, r, r0, r1, r2; // for reds
	int g_l, g_r, g, g0, g1, g2; // for greens
	int b_l, b_r, b, b0, b1, b2; // for blues

	// now we will initialize our color values
	// for the vertices based on the parameters
	r0 = c0.r;
	g0 = c0.g;
	b0 = c0.b;

	r1 = c1.r;
	g1 = c1.g;
	b1 = c1.b;

	r2 = c2.r;
	g2 = c2.g;
	b2 = c2.b;

	if(r0 <= 255 && g0 <= 255 && b0 <= 255 && r0 >= 0 && g0 >= 0 && b0 >= 0 && coordinates_total == (NUM_COORDINATES)){

		// initialize the x coordinates
		X_L = X_R = x0;

		// initialize the color bounds
		r_r = r_l = r0;
		g_r = g_l = g0;
		b_r = b_l = b0;

		// precompute some ratios to save calculation time
		if(denom10 != 0)
			float10 = ((float)(x1-x0))/(denom10);
		if(denom20 != 0)
			float20 = ((float)(x2-x0))/(denom20);
		if(denom21 != 0)
			float21 = ((float)(x2-x1))/(denom21);

		// bottom half of triangle will now be colored
		for(i = (HEIGHT-mousecoordinates[0].y); i < (HEIGHT-mousecoordinates[1].y); i++){
			// check for divide by zero
			if(denom10 != 0 && denom20 != 0){

				// precalulate some ratios
				floati10 = ((float)(i-y0)/(denom10));
				floati20 = ((float)(i-y0)/(denom20));

				// midpoint is to left or even with top
				if(x1 <= x2){
					// use correct slope for condition
					X_L += float10;
					X_R += float20;

					// set the left and right color bounds
					// and their correct conditions for updating

					r_l = floati10 * r1 + (1-floati10) * r0;
					r_r = floati20 * r2 + (1-floati20) * r0;

					g_l = floati10 * g1 + (1-floati10) * g0;
					g_r = floati20 * g2 + (1-floati20) * g0;

					b_l = floati10 * b1 + (1-floati10) * b0;
					b_r = floati20 * b2 + (1-floati20) * b0;

				}
				// midpoint is to right of top
				else{
					// use correct slope for condition
					X_L += float20;
					X_R += float10;

					// set the left and right color bounds
					// and their correct conditions for updating

					r_l = floati20 * r2 + (1-floati20) * r0;
					r_r = floati10 * r1 + (1-floati10) * r0;

					g_l = floati20 * g2 + (1-floati20) * g0;
					g_r = floati10 * g1 + (1-floati10) * g0;

					b_l = floati20 * b2 + (1-floati20) * b0;
					b_r = floati10 * b1 + (1-floati10) * b0;
				}

				// now lets color the pixels?
				for(x = ceil(X_L); x < floor(X_R); x++){

					// calculate the r,g,b values along this scanline
					r = ((float)(x-X_L)/(X_R - X_L)) * r_r + (1-((float)(x-X_L)/(X_R - X_L))) * r_l;
					g = ((float)(x-X_L)/(X_R - X_L)) * g_r + (1-((float)(x-X_L)/(X_R - X_L))) * g_l;
					b = ((float)(x-X_L)/(X_R - X_L)) * b_r + (1-((float)(x-X_L)/(X_R - X_L))) * b_l;

					// set the colors in the frame buffer
					frame_buffer[i][x][0] = r;
					frame_buffer[i][x][1] = g;
					frame_buffer[i][x][2] = b;
				}
			}
		}

		// top half of the triangle will now be colored
		for(i = (HEIGHT-mousecoordinates[1].y); i <= (HEIGHT-mousecoordinates[2].y); i++){

			// check for divide by zero
			if(denom21 != 0 && denom20 != 0){

				// precalulate some values
				floati20 = ((float)(i-y0)/(denom20));
				floati21 = ((float)(i-y1)/(denom21));

				// midpoint is to left or even with top
				if(x1 <= x2){
					// use correct slope for condition
					X_L += float21;
					X_R += float20;

					// set the left and right color bounds
					// and their correct conditions for updating

					r_l = floati21 * r2 + (1-floati21) * r1;
					r_r = floati20 * r2 + (1-floati20) * r0;

					g_l = floati21 * g2 + (1-floati21) * g1;
					g_r = floati20 * g2 + (1-floati20) * g0;

					b_l = floati21 * b2 + (1-floati21) * b1;
					b_r = floati20 * b2 + (1-floati20) * b0;

				}
				// midpoint is to right of top
				else{
					// use correct slope for condition
					X_L += float20;
					X_R += float21;

					// set the left and right color bounds
					// and their correct conditions for updating

					r_l = floati20 * r2 + (1-floati20) * r0;
					r_r = floati21 * r2 + (1-floati21) * r1;

					g_l = floati20 * g2 + (1-floati20) * g0;
					g_r = floati21 * g2 + (1-floati21) * g1;

					b_l = floati20 * b2 + (1-floati20) * b0;
					b_r = floati21 * b2 + (1-floati21) * b1;

				}

				// now lets color the pixels?
				for(x = ceil(X_L); x < floor(X_R); x++){

					// calculate the r,g,b values along this scanline
					r = ((float)(x-X_L)/(X_R - X_L)) * r_r + (1-((float)(x-X_L)/(X_R - X_L))) * r_l;
					g = ((float)(x-X_L)/(X_R - X_L)) * g_r + (1-((float)(x-X_L)/(X_R - X_L))) * g_l;
					b = ((float)(x-X_L)/(X_R - X_L)) * b_r + (1-((float)(x-X_L)/(X_R - X_L))) * b_l;

					// set the colors in the frame buffer
					frame_buffer[i][x][0] = r;
					frame_buffer[i][x][1] = g;
					frame_buffer[i][x][2] = b;
				}
			}
		}
	}
}

// handles mouse clicks and when to draw
void handleMouseAndDrawing(int button, int state, int x, int y)
{
	// when left mouse clicks occur, it is stored and
	// then if conditions are met, the figure is colored
	// and then drawn.
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){

		if(coordinates_total < NUM_COORDINATES){

			// save the mouse coordinates
			setCoordinates(x, y, coordinates_total);

			// increment number of coordinates saved
			coordinates_total++;
		}

		if(coordinates_total == NUM_COORDINATES){

			// create some color structs
			// and give the values for colors
			// that we want to have for vertices in the triangle
			color c0, c1, c2;
			c0.r = c1.g = c2.b = 255;
			c0.g = c0.b = c1.r = c1.b = c2.r = c2.g = 0;

			// go color the triangle in the framebuffer
			colorCoordinates(c0, c1, c2);

			// reset the number of coordinates
			coordinates_total = 0;

			// cause a display event to occur for GLUT:
			glutPostRedisplay();

			// clear mouse coordinates from the array
			clearCoordinates();
		}
	}
}

// Called by GLUT when a display event occurs:
void display(void) {

	//	Set the raster position to the lower-left corner to avoid a problem
	//	(with glDrawPixels) when the window is resized to smaller dimensions.
	glRasterPos2i(-1,-1);

	// Write the information stored in "frame_buffer" to the color buffer
	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, frame_buffer);
	glFlush();
}

int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Mark McKelvy - Assn 1 - Frame Buffer Simulator");

	// Specify which functions get called for display and mouse events:
	glutDisplayFunc(display);
	glutMouseFunc(handleMouseAndDrawing);

	glutMainLoop();

	return 1;
}

