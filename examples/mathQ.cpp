#include <math.h>

void QuaternionToRotationMatrix(double q[4], double R[3][3])
 {
  double s, vx, vy, vz, tvx, tvy, tvz;

  s = q[0]; vx = q[1]; vy = q[2]; vz = q[3];
  tvx = 2.0 * q[1];
  tvy = 2.0 * q[2];
  tvz = 2.0 * q[3];

  R[0][0] = 1.0 - tvy * vy - tvz * vz;
  R[0][1] = tvx * vy - tvz * s;
  R[0][2] = tvx * vz + tvy * s;

  R[1][0] = tvx * vy + tvz * s;
  R[1][1] = 1.0 - tvx * vx - tvz * vz;
  R[1][2] = tvy * vz - tvx * s;

  R[2][0] = tvx * vz - tvy * s;
  R[2][1] = tvy * vz + tvx * s;
  R[2][2] = 1.0 - tvx * vx - tvy * vy;
}

/* ------------------------------------------------------------------
   compose (multiply) two quaternions q1 and q2 and renormalize result r.
   note that the first element is the scalar component,
   and r is assumed not the reference the same quaternion as q1 or q2.
*/
void QuaternionComposite(double q1[4], double q2[4], double r[4])
{
  double cross[3];

  CrossProduct(&q1[1], &q2[1], cross);  // See code for this function below

  r[0] = q1[0] * q2[0] - (q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3]);
  r[1] = cross[0] + q2[0] * q1[1] + q1[0] * q2[1];
  r[2] = cross[1] + q2[0] * q1[2] + q1[0] * q2[2];
  r[3] = cross[2] + q2[0] * q1[3] + q1[0] * q2[3];

  Normalize4(r);  // See code for this function below
}

/* ------------------------------------------------------------------
   convert from quaternion to equivalent angle-axis form,
   as might be required for glRotate().
   NOTE: the LAST element of AA will be the rotation angle,
   the first three elements describe the axis
*/
void QuaternionToAxisAngle(double q[4], double AA[4])
{
  double d = sqrt(1 - q[0]*q[0]);

  if (fabs(d) < .00001)
    d = 1.0;

  AA[0] = q[1] / d;
  AA[1] = q[2] / d;
  AA[2] = q[3] / d;
  AA[3] = 2.0 * acos(q[0]);
}

// compute quaternion conjugate (inverting a rotation)
void QuaternionConjugate(double s[4], double d[4])
{
  d[0] =  s[0];
  d[1] = -s[1];
  d[2] = -s[2];
  d[3] = -s[3];
}

// normalize a 4D vector (or a quaternion, to ensure it is a proper rotation)
void Normalize4(double v[4])
{
  double length = 0.0;
  int i;
 
  for (i = 0; i < 4; i++)
    length += v[i]*v[i];              /* compute vector length:    */
 
  if (length > 0.0)
   {
    length = sqrt(length);
    for (i = 0; i < 4; i++)           /* then divide componentwise */
      v[i] = v[i]/length;
   }
  else
   {
    v[0] = 1.0; v[1] = v[2] = v[3] = 0.0;
   }
 }

/* cross product of two 3-element vectors */
void CrossProduct(double v[3], double w[3], double result[3])
{
  result[0] = v[1]*w[2]-v[2]*w[1];             
  result[1] = v[2]*w[0]-v[0]*w[2];                 
  result[2] = v[0]*w[1]-v[1]*w[0];
}

/* 3x3 matrix applied to 3-element vector, e.g. for rotating a direction vector */
void multiply_tform_vector_3x3(double T[3][3], double v[3], double r[3])
{
  int i;
 
  for (i = 0; i < 3; i++)
    r[i] = T[i][0]*v[0] + T[i][1]*v[1] + T[i][2]*v[2];
}

/* multiplication of two 3x3 matrices */
void multiply_tform_tform_3x3(double A[3][3], double B[3][3], double C[3][3])
 {
  int i, j, k;
  
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
     {
      C[i][j] = 0.0;
      for (k = 0; k < 3; k++)
        C[i][j] += A[i][k] * B[k][j];
     } 
 }
