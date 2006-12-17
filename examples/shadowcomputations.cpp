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

void MakeShadowMatrix(GLfloat planeCoeff[4], GLfloat lightPos[4], GLfloat destMat[4][4]){
    GLfloat dot;
    
    // Dot product of plane and light position
    dot =  planeCoeff[0] * lightPos[0] +
    planeCoeff[1] * lightPos[1] +
    planeCoeff[2] * lightPos[2] +
    planeCoeff[3] * lightPos[3];
    
    // Now do the projection
    // First column
    destMat[0][0] = dot - lightPos[0] * planeCoeff[0];
    destMat[1][0] = 0.0f - lightPos[0] * planeCoeff[1];
    destMat[2][0] = 0.0f - lightPos[0] * planeCoeff[2];
    destMat[3][0] = 0.0f - lightPos[0] * planeCoeff[3];
    
    // Second column
    destMat[0][1] = 0.0f - lightPos[1] * planeCoeff[0];
    destMat[1][1] = dot - lightPos[1] * planeCoeff[1];
    destMat[2][1] = 0.0f - lightPos[1] * planeCoeff[2];
    destMat[3][1] = 0.0f - lightPos[1] * planeCoeff[3];
    
    // Third Column
    destMat[0][2] = 0.0f - lightPos[2] * planeCoeff[0];
    destMat[1][2] = 0.0f - lightPos[2] * planeCoeff[1];
    destMat[2][2] = dot - lightPos[2] * planeCoeff[2];
    destMat[3][2] = 0.0f - lightPos[2] * planeCoeff[3];
    
    // Fourth Column
    destMat[0][3] = 0.0f - lightPos[3] * planeCoeff[0];
    destMat[1][3] = 0.0f - lightPos[3] * planeCoeff[1];
    destMat[2][3] = 0.0f - lightPos[3] * planeCoeff[2];
    destMat[3][3] = dot - lightPos[3] * planeCoeff[3];
}
