#ifndef NGN_GFX_HELPERS_H
#define NGN_GFX_HELPERS_H

#include "GFXIncludes.h"
#include <sstream>
#include <cmath>
#include "glm/glm.hpp"
#include <algorithm>	//find_if
#include <locale>		//isdigit

// A helper macro to get a position
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Function to convert degrees to radians
//const float TO_RADS = 3.141592654f / 180.0f; // The value of 1 degree in radians
static inline float toRads( const float &theAngleInDegrees ) {
    return theAngleInDegrees * 3.141592654f / 180.0f;
}
// Linear interpolate floats
static inline float lerp(float v0, float v1, float t) {
    return v0+(v1-v0)*t;
}
// String helpers
static inline std::string boolToString(bool number) {
    return number == true ? "TRUE" : "FALSE";   //return a string with the contents of the stream
}
static inline std::string intToString( int number ) {
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}
static inline std::string floatToString( float number, int precision = 3 ) {
    std::ostringstream buff;
    buff.precision(precision);
    buff<<number;
    return buff.str();
}
static inline std::string doubleToString( double number, int precision = 3 ) {
    std::ostringstream buff;
    buff.precision(precision);
    buff<<number;
    return buff.str();
}
static inline bool is_number(const std::string& s) {
    return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { std::locale loc; return !std::isdigit(c, loc); }) == s.end();
}
// Bigger of two values
static inline float float_max( const float a, const float b ) {
    return (a > b ? a : b);
}
static inline double double_max( const double a, const double b ) {
    return (a > b ? a : b);
}
static inline int int_max( const int a, const int b ) {
    return (a > b ? a : b);
}

// Relatively quick number clamp
static inline float float_clamp(const float x, const float a, const float b ) {
    return x < a ? a : (x > b ? b : x);
}
static inline double double_clamp(const double x, const double a, const double b ) {
    return x < a ? a : (x > b ? b : x);
}
static inline int int_clamp(const int x, const int a, const int b ) {
    return x < a ? a : (x > b ? b : x);
}
// Relatively solid rounding
static inline float float_round(const float x, const float size) {
    return x > 0.0 ? ceil((x-size)/size)*size : floor(x/size)*size;
}
static inline double double_round(const double x, const double size) {
    return x > 0.0 ? ceil((x-size)/size)*size : floor(x/size)*size;
}
// Integer rounding
inline double int_round(const double x, const double size) {
    double factor = x/size;
    int roundedF = int(factor > 0.0 ? factor + 0.5 : factor - 0.5);
    return double(roundedF*size);
}
// AABB intersection
inline bool aabb_intersect(const glm::vec3 aPos, const glm::vec3 aSize,
                           const glm::vec3 bPos, const glm::vec3 bSize) {
    glm::vec3 aMax = aPos + (aSize*0.5f);
    glm::vec3 aMin = aPos - (aSize*0.5f);
    glm::vec3 bMax = bPos + (bSize*0.5f);
    glm::vec3 bMin = bPos - (bSize*0.5f);
    return ( (aMax.x >= bMin.x && aMin.x <= bMax.x) &&
            (aMax.y >= bMin.y && aMin.y <= bMax.y) &&
            (aMax.z >= bMin.z && aMin.z <= bMax.z) );
}
// OpenGL 4x4 matrix multiplication
static inline void MultMatrices4x4( GLdouble *result, const GLdouble *matrix1, const GLdouble *matrix2)
{
    result[0]=matrix1[0]*matrix2[0]+
      matrix1[4]*matrix2[1]+
      matrix1[8]*matrix2[2]+
      matrix1[12]*matrix2[3];
    result[4]=matrix1[0]*matrix2[4]+
      matrix1[4]*matrix2[5]+
      matrix1[8]*matrix2[6]+
      matrix1[12]*matrix2[7];
    result[8]=matrix1[0]*matrix2[8]+
      matrix1[4]*matrix2[9]+
      matrix1[8]*matrix2[10]+
      matrix1[12]*matrix2[11];
    result[12]=matrix1[0]*matrix2[12]+
      matrix1[4]*matrix2[13]+
      matrix1[8]*matrix2[14]+
      matrix1[12]*matrix2[15];
    result[1]=matrix1[1]*matrix2[0]+
      matrix1[5]*matrix2[1]+
      matrix1[9]*matrix2[2]+
      matrix1[13]*matrix2[3];
    result[5]=matrix1[1]*matrix2[4]+
      matrix1[5]*matrix2[5]+
      matrix1[9]*matrix2[6]+
      matrix1[13]*matrix2[7];
    result[9]=matrix1[1]*matrix2[8]+
      matrix1[5]*matrix2[9]+
      matrix1[9]*matrix2[10]+
      matrix1[13]*matrix2[11];
    result[13]=matrix1[1]*matrix2[12]+
      matrix1[5]*matrix2[13]+
      matrix1[9]*matrix2[14]+
      matrix1[13]*matrix2[15];
    result[2]=matrix1[2]*matrix2[0]+
      matrix1[6]*matrix2[1]+
      matrix1[10]*matrix2[2]+
      matrix1[14]*matrix2[3];
    result[6]=matrix1[2]*matrix2[4]+
      matrix1[6]*matrix2[5]+
      matrix1[10]*matrix2[6]+
      matrix1[14]*matrix2[7];
    result[10]=matrix1[2]*matrix2[8]+
      matrix1[6]*matrix2[9]+
      matrix1[10]*matrix2[10]+
      matrix1[14]*matrix2[11];
    result[14]=matrix1[2]*matrix2[12]+
      matrix1[6]*matrix2[13]+
      matrix1[10]*matrix2[14]+
      matrix1[14]*matrix2[15];
    result[3]=matrix1[3]*matrix2[0]+
      matrix1[7]*matrix2[1]+
      matrix1[11]*matrix2[2]+
      matrix1[15]*matrix2[3];
    result[7]=matrix1[3]*matrix2[4]+
      matrix1[7]*matrix2[5]+
      matrix1[11]*matrix2[6]+
      matrix1[15]*matrix2[7];
    result[11]=matrix1[3]*matrix2[8]+
      matrix1[7]*matrix2[9]+
      matrix1[11]*matrix2[10]+
      matrix1[15]*matrix2[11];
    result[15]=matrix1[3]*matrix2[12]+
      matrix1[7]*matrix2[13]+
      matrix1[11]*matrix2[14]+
      matrix1[15]*matrix2[15];
}
// OpenGL 4x4 matrix by 3D vector multiplication
 static inline void MultMatrixByVector4x4(GLdouble *resultvector, const GLdouble *matrix, const GLdouble *pvector)
{
    resultvector[0]=matrix[0]*pvector[0]+matrix[4]*pvector[1]+matrix[8]*pvector[2]+matrix[12]*pvector[3];
    resultvector[1]=matrix[1]*pvector[0]+matrix[5]*pvector[1]+matrix[9]*pvector[2]+matrix[13]*pvector[3];
    resultvector[2]=matrix[2]*pvector[0]+matrix[6]*pvector[1]+matrix[10]*pvector[2]+matrix[14]*pvector[3];
    resultvector[3]=matrix[3]*pvector[0]+matrix[7]*pvector[1]+matrix[11]*pvector[2]+matrix[15]*pvector[3];
}
// Helper defines for OpenGL matrix inversion
#define SWAP_ROWS_DOUBLE(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define SWAP_ROWS_FLOAT(a, b) { GLfloat *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]
// Invert 4x4 OpenGL matrix 
static inline int InvertMatrix4x4(GLdouble *m, GLdouble *out)
{
   GLdouble wtmp[4][8];
   GLdouble m0, m1, m2, m3, s;
   GLdouble *r0, *r1, *r2, *r3;
   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
   r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
      r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
      r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
      r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
      r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
      r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
      r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
      r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
      r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
      r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
      r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
      r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;
   /* choose pivot - or die */
    if ( std::abs(r3[0]) > std::abs(r2[0]) )
      SWAP_ROWS_DOUBLE(r3, r2);
   if ( std::abs(r2[0]) > std::abs(r1[0]) )
      SWAP_ROWS_DOUBLE(r2, r1);
   if ( std::abs(r1[0]) > std::abs(r0[0]) )
      SWAP_ROWS_DOUBLE(r1, r0);
   if (0.0 == r0[0])
      return 0;
   /* eliminate first variable     */
   m1 = r1[0] / r0[0];
   m2 = r2[0] / r0[0];
   m3 = r3[0] / r0[0];
   s = r0[1];
   r1[1] -= m1 * s;
   r2[1] -= m2 * s;
   r3[1] -= m3 * s;
   s = r0[2];
   r1[2] -= m1 * s;
   r2[2] -= m2 * s;
   r3[2] -= m3 * s;
   s = r0[3];
   r1[3] -= m1 * s;
   r2[3] -= m2 * s;
   r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) {
      r1[4] -= m1 * s;
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r0[5];
   if (s != 0.0) {
      r1[5] -= m1 * s;
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r0[6];
   if (s != 0.0) {
      r1[6] -= m1 * s;
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r0[7];
   if (s != 0.0) {
      r1[7] -= m1 * s;
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }
   /* choose pivot - or die */
   if (std::abs(r3[1]) > std::abs(r2[1]))
      SWAP_ROWS_DOUBLE(r3, r2);
   if (std::abs(r2[1]) > std::abs(r1[1]))
      SWAP_ROWS_DOUBLE(r2, r1);
   if (0.0 == r1[1])
      return 0;
   /* eliminate second variable */
   m2 = r2[1] / r1[1];
   m3 = r3[1] / r1[1];
   r2[2] -= m2 * r1[2];
   r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3];
   r3[3] -= m3 * r1[3];
   s = r1[4];
   if (0.0 != s) {
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r1[5];
   if (0.0 != s) {
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r1[6];
   if (0.0 != s) {
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r1[7];
   if (0.0 != s) {
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }
   /* choose pivot - or die */
   if (std::abs(r3[2]) > std::abs(r2[2]))
      SWAP_ROWS_DOUBLE(r3, r2);
   if (0.0 == r2[2])
      return 0;
   /* eliminate third variable */
   m3 = r3[2] / r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
      r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];
   /* last check */
   if (0.0 == r3[3])
      return 0;
   s = 1.0 / r3[3];		/* now back substitute row 3 */
   r3[4] *= s;
   r3[5] *= s;
   r3[6] *= s;
   r3[7] *= s;
   m2 = r2[3];			/* now back substitute row 2 */
   s = 1.0 / r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
      r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
      r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
      r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;
   m1 = r1[2];			/* now back substitute row 1 */
   s = 1.0 / r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
      r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
      r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;
   m0 = r0[1];			/* now back substitute row 0 */
   s = 1.0 / r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
      r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);
   MAT(out, 0, 0) = r0[4];
   MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
   MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
   MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
   MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
   MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
   MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
   MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
   MAT(out, 3, 3) = r3[7];
   return 1;
}
// Edited from glu for 2D only, so that GLU isn't required
static inline int UnProject2D( GLdouble winX, GLdouble winY,
                              const GLdouble *model, const GLdouble *proj, const GLint *view,
                              GLdouble *objX, GLdouble *objY ) {
      //Transformation matrices
      GLdouble m[16], A[16];
      GLdouble in[4], out[4];
      //Calculation for inverting a matrix, compute projection x modelview
      //and store in A[16]
      MultMatrices4x4(A, proj, model);
      //Now compute the inverse of matrix A
      if( InvertMatrix4x4(A, m)==0 )
         return 0;
      //Transformation of normalized coordinates between -1 and 1
      in[0]=(winX-(float)view[0])/(float)view[2]*2.0-1.0;
      in[1]=(winY-(float)view[1])/(float)view[3]*2.0-1.0;
      in[2]=-2.0;
      in[3]=1.0;
      //Objects coordinates
      MultMatrixByVector4x4(out, m, in);
      if(out[3]==0.0)
         return 0;
      out[3]=1.0/out[3];
      *objX=out[0]*out[3];
      *objY=out[1]*out[3];
      //objectCoordinate[2]=out[2]*out[3];
      return 1;
}

#endif

