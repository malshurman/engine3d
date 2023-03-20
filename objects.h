#pragma once
#include "olcPixelGameEngine.h"

struct Vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1; // Need a 4th term to perform sensible matrix vector multiplication

	float length() { return sqrtf(x * x + y * y + z * z); }
	friend Vec3d operator+(Vec3d& v1, Vec3d& v2);
	friend Vec3d operator-(Vec3d& v1, Vec3d& v2);
	friend Vec3d operator*(Vec3d& v1, float k);
	friend Vec3d operator/(Vec3d& v1, float k);
	friend float operator*(Vec3d& v1, Vec3d& v2); // Dot product
	friend Vec3d operator%(Vec3d& v1, Vec3d& v2); // Cross product
	friend Vec3d operator~(Vec3d& v); // Normaize
};

struct Triangle
{
	Vec3d p[3];
	uint32_t col;
};

struct Mesh
{
	std::vector<Triangle> tris;
};

struct Mat4x4
{
	float m[4][4] = { 0 };

	friend Mat4x4 operator*(Mat4x4 const& m1, Mat4x4 const& m2);
	friend Vec3d operator*(Mat4x4 const& m, Vec3d const& v);
};

Vec3d operator+(Vec3d& v1, Vec3d& v2);
Vec3d operator-(Vec3d& v1, Vec3d& v2);
Vec3d operator*(Vec3d& v1, float k);
Vec3d operator/(Vec3d& v1, float k);
float operator*(Vec3d& v1, Vec3d& v2);
Vec3d operator%(Vec3d& v1, Vec3d& v2);
Vec3d operator~(Vec3d& v);
Vec3d Vector_IntersectPlane(Vec3d& plane_p, Vec3d& plane_n, Vec3d& lineStart, Vec3d& lineEnd);

Mat4x4 operator*(Mat4x4 const& m1, Mat4x4 const& m2);
Vec3d operator*(Mat4x4 const& m, Vec3d const& v);

Mat4x4 createMatrixIdentity();
Mat4x4 Matrix_MakeRotationX(float fAngleRad);
Mat4x4 Matrix_MakeRotationY(float fAngleRad);
Mat4x4 Matrix_MakeRotationZ(float fAngleRad);
Mat4x4 Matrix_MakeTranslation(float x, float y, float z);
Mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
Mat4x4 Matrix_PointAt(Vec3d& pos, Vec3d& target, Vec3d& up);
Mat4x4 Matrix_QuickInverse(Mat4x4& m);

int Triangle_ClipAgainstPlane(Vec3d plane_p, Vec3d plane_n, Triangle& in_tri, Triangle& out_tri1, Triangle& out_tri2);
Mesh loadTrianglesFromObj(std::string sFilename);