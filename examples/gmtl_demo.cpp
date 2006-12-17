/************************************************
 *	CMPS 415/515								*
 *	GMTL Tutorial								*
 *												*
 *	Author: Chris Best							*
 *	University of Louisiana at Lafayette		*
 *												*
 ************************************************/


//GMTL is at: <a href="http://ggt.sourceforge.net" target="_blank">http://ggt.sourceforge.net</a>

//This was necessary for me to compile scalar multiplication on Mac OS X
//Try commenting it out on your compiler. You should get better performance 
//without it.
#define GMTL_NO_METAPROG

//First include all of the headers we'll need for our data types
#include <gmtl/Vec.h>			//Vectors
#include <gmtl/Point.h>			//Points
#include <gmtl/Matrix.h>		//Matrices
#include <gmtl/Quat.h>			//Quaternions
#include <gmtl/EulerAngle.h>	//Euler Angles
#include <gmtl/Output.h>		//cout support
#include <gmtl/Generate.h>		//makeRot/makeTrans/etc.
#include <gmtl/VecOps.h>		//Vector Operations
#include <gmtl/MatrixOps.h>		//Matrix Operations


//I don't like having to type std::cout
using namespace std;


int main(int argc, char* argv[]) {

	//Create a few vectors
	//A={1,0,0}
	//B={0,1,0}
	//C={0,0,0}
	gmtl::Vec3f A(1,0,0), B(0,1,0), C;
	
	cout << "Vector Demo: " << endl;
	cout << "A = " << A << endl;
	cout << "B = " << B << endl;
	//Scalar Multiplications
	cout << "2 * A = " << 2.0 * A << endl;
	//Arithmetic
	cout << "A + B = " << A + B << endl;
	cout << "A - B = " << A - B << endl;
	//Dot Product
	cout << "A * B = " << gmtl::dot(A,B) << endl;
	//Cross Product
	//Note that the result is stored in C
	//C = gmtl::cross(A,B); should work as well, but some compilers get confused.
	gmtl::cross(C,A,B);
	cout << "A X B = " << C << endl << endl;
	
	
	
	gmtl::Matrix44f D,E,F;		//Initialized to identity
	cout << "Matrix Demo: " << endl;
	
	//First make E a 90 degree rotation about Y
	//Notice the deg2rad helper function. You may find it useful.
	//GMTL expects everything to be in radians.
	E = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(0,gmtl::Math::deg2Rad(90.0f),0));
	//makeRot is a templated function. The template parameter specifies what format 
	//you want the resulting rotation to be in (Matrix, Quaternion, Euler Angle)
	
	cout << "Matrix E: " << endl << fixed << E << endl;
	//Rotate vector A by E
	cout << "E * A = " << E * A << endl;
	
	//Create a translation matrix from A
	cout << "Now lets convert vector A to Matrix D." << endl;
	D = gmtl::makeTrans<gmtl::Matrix44f>(A);

	cout << "A in Matrix form (D): " << endl << fixed << D << endl;
	
	
	//Show that E*D != D*E
	cout << "D * E = " << endl << fixed << D * E << endl;
	cout << "E * D = " << endl << fixed << E * D << endl;
	
	//Make F a rotation by 90 degrees around Y and a translation one unit in X
	F = E*D;
	
	cout << "F = E * D " << endl;
	
	//Points != Vectors
	gmtl::Point3f P(1,0,0);
	
	//P is the same as  A, right?
	
	//Will F * A = F * P?
	
	cout << "Vector A = " << A << endl;
	cout << "Point P = " << P << endl;
	
	//Vec and Point share the same base class (VecBase), so you can use them interchangably in binary ops
	//such as +, -, ==, etc.
	cout << "(A == P): " << (A == P) << endl;
	
	//But be careful. They don't necessarily behave the same.
	cout << "F * A = " << F * A << endl;
	cout << "F * P = " << F * P << endl;
	
	//Not the same? How come?
	
	//Vectors are not supposed to change under translation operations
	//Points (vertices) however do. Make sure to keep that in mind when representing
	//things like geometry. Vectors are good for showing direction, but they aren't so 
	//good for representing position. Use Points for that.
	
	cout << endl << "Some More Matrix Ops: " << endl;
	//Now for Matrix inversion
	cout << "Matrix F:" << endl << fixed << F << endl;
	cout << "Matrix F^-1: " << endl << fixed << gmtl::invert(F) << endl;
	//Easy, right? Be careful! Look at the value of F now.
	cout << "Matrix F: " << endl << fixed << F << endl; //That's not F!
	//A better way would have been to:
	//gmtl::Matrix4f Finv = F;
	//gmtl::invert(Finv);
	//	or
	//cout << "Matrix F^-1:" << endl << fixed << gmtl::invert(gmtl::Matrix44f(F)) << endl;
	
	//The same is true of vector normalization
	
	gmtl::Vec3f VP = F * P;
	cout << "Vector VP = " << VP << endl;
	//For some reason gmtl::normalize() returns the magnitude of the vector.
	cout << "Magnitude: " << gmtl::normalize(VP) << endl;
	cout << "Vector VP Normalized = " << VP << endl;
	
	
	//GMTL includes many other data types such as Quaternions (you'll learn about them later), 
	//Spheres, Triangles, Rays, Line Segments, Bounding Boxes, etc. Many of these will end up
	//Being very useful for later assignments, so do a little digging.
	
    int cfkjlf;
    cin >> cfkjlf;
   return 0;

};
