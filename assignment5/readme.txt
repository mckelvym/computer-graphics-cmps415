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
