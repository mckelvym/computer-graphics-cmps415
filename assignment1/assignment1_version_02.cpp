// Second version of assignment 1 for CMPS415
// Every three mouse clicks (left clicks) it will
// draw a triangle connecting those points.


#include <stdio.h>
#include <iostream.h>
#include <math.h>
#include <GL/glut.h>

#define WIDTH 400			// define window width
#define HEIGHT 300			// define window height
static GLubyte frame_buffer[HEIGHT][WIDTH][3];	// create simulated frame buffer

#define NUM_COORDINATES 3	// define number of mouse coordinates to hold

struct coord{				// create a struct to hold mouse coordinates
	int x;					// an x coordinate
	int y;					// a y coordinate
};

// array for mouse coordinates
coord mousecoordinates[NUM_COORDINATES];
// amount of coordinates currently in the array
int coordinates_total = 0;	

// clear a specific set of coordinates in the array
void clearCoordinates(){
	for(int i = 0; i < NUM_COORDINATES; i++){
		mousecoordinates[i].x = -1;
		mousecoordinates[i].y = -1;
	}
}

// set a specific set of coordinates in the array
void setCoordinates(int x, int y, int index){
	if(x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT && index >= 0 && index < NUM_COORDINATES){
		mousecoordinates[index].x = x;
		mousecoordinates[index].y = y;
	}
}

// use a simple bubble sort to sort the coordinates
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
void colorCoordinates(){
	float X_L, X_R; // for the left side of the triangle, and right side, respectively
	int x; // for counting through the individual pixels left to right
	int i; // for counting the scanlines of the triangle, bottom to top
	float denom20, denom10, denom21; // used in checking for divide by 0 cases
	int x0, x1, x2; // simplify some typing

	//colors
	int r_l, r_r, r, r0; // for reds
	int g_l, g_r, g, g0; // for greens
	int b_l, b_r, b, b0; // for blues

	r = 255;
	g = 255;
	b = 255;
	r0 = 255;
	g0 = 255;
	b0 = 255;

	if(r0 <= 255 && g0 <= 255 && b0 <= 255 && r0 >= 0 && g0 >= 0 && b0 >= 0 && coordinates_total == (NUM_COORDINATES)){
		// sort coordinates in descending order
		sortCoordinates();
	
		denom20 = ((float)((HEIGHT-mousecoordinates[2].y) - (HEIGHT-mousecoordinates[0].y)));
		denom10 = ((float)((HEIGHT-mousecoordinates[1].y) - (HEIGHT-mousecoordinates[0].y)));
		denom21 = ((float)((HEIGHT-mousecoordinates[2].y) - (HEIGHT-mousecoordinates[1].y)));
		x0 = mousecoordinates[0].x;
		x1 = mousecoordinates[1].x;
		x2 = mousecoordinates[2].x;

		// initialize the x coordinates
		X_L = X_R = x0;

		// bottom half of triangle
		for(i = (HEIGHT-mousecoordinates[0].y); i < (HEIGHT-mousecoordinates[1].y); i++){
			if(denom10 != 0 && denom20 != 0){	
				if(x1 <= x2){
					X_L += ((float)(x1-x0))/(denom10);
					X_R += ((float)(x2-x0))/(denom20);			
				}
				else{
					X_L += ((float)(x2-x0))/(denom20);
					X_R += ((float)(x1-x0))/(denom10);
				}

				// now lets color the pixels?
				for(x = ceil(X_L); x < floor(X_R); x++){
					frame_buffer[i][x][0] = r;
					frame_buffer[i][x][1] = g;
					frame_buffer[i][x][2] = b;
				}
			}
		}

		for(i = (HEIGHT-mousecoordinates[1].y); i <= (HEIGHT-mousecoordinates[2].y); i++){
			if(denom21 != 0 && denom20 != 0){
				if(x1 <= x2){
					X_L += ((float)(x2-x1))/(denom21);
					X_R += ((float)(x2-x0))/(denom20);			
				}
				else{
					X_L += ((float)(x2-x0))/(denom20);
					X_R += ((float)(x2-x1))/(denom21);
				}

				// now lets color the pixels?
				for(x = ceil(X_L); x < floor(X_R); x++){
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
  // when left mouse clicks occur, it is stores and
  // then if conditions are met, the figure is colored
  // and then drawn.
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
	  if(coordinates_total < NUM_COORDINATES){
		setCoordinates(x, y, coordinates_total);
		coordinates_total++;
	  }
	  if(coordinates_total == NUM_COORDINATES){
		colorCoordinates();
		coordinates_total = 0;
		// cause a display event to occur for GLUT:
  		glutPostRedisplay();
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
	glutCreateWindow("Frame Buffer Example");

	// Specify which functions get called for display and mouse events:
	glutDisplayFunc(display);
    glutMouseFunc(handleMouseAndDrawing);

	glutMainLoop();

	return 1;
}
