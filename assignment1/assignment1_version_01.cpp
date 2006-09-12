// Early version of assignment 1 for CMPS415
// Every three mouse clicks (left clicks) it will
// draw those points in red.


#include <stdio.h>
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

// color the set of coordinates in the framebuffer
// that is held in the coordinates array
void colorCoordinates(int r, int g, int b){
	if(r <= 255 && g <= 255 && b <= 255 && r >= 0 && g >= 0 && b >= 0 && coordinates_total == (NUM_COORDINATES)){
		for(int i = 0; i < coordinates_total; i++){
			frame_buffer[HEIGHT-mousecoordinates[i].y][mousecoordinates[i].x][0] = r;
			frame_buffer[HEIGHT-mousecoordinates[i].y][mousecoordinates[i].x][1] = g;
			frame_buffer[HEIGHT-mousecoordinates[i].y][mousecoordinates[i].x][2] = b;
		}		
	}
}


// Called when mouse button pressed: 
void mousebuttonhandler(int button, int state, int x, int y)
{
  printf("Mouse button event, button=%d, state=%d, x=%d, y=%d\n", button, state, x, y);

  // set a pixel's red color value when left mouse button is pressed down:
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
	  if(coordinates_total < NUM_COORDINATES){
		setCoordinates(x, y, coordinates_total);
		coordinates_total++;
	  }
	  if(coordinates_total == NUM_COORDINATES){
		colorCoordinates(255, 0, 0);
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
    glutMouseFunc(mousebuttonhandler);

	glutMainLoop();

	return 1;
}
