#include "stdafx.h"
#include "../include/g3dtilespatial.h"
#include <cassert>
#include "math.h"

#define WGS84_EllipsoidA   6378137.0			//地球赤道半径   笛卡尔x 轴
#define WGS84_EllipsoidB	 6378137.0			//地球赤道半径  笛卡尔y 轴
#define WGS84_EllipsoidC   6356752.3142451793	//地球纬度方向半径  笛卡尔z轴
#define RadPerDegree       0.017453292519943295	//

MapGIS::Tile::Vector3D::Vector3D() :x(0), y(0), z(0)
{}

MapGIS::Tile::Vector3D::Vector3D(const double fX, const double fY, const double fZ)
	: x(fX), y(fY), z(fZ)
{}

MapGIS::Tile::Vector3D::Vector3D(const double afCoordinate[3]) : x(afCoordinate[0]), y(afCoordinate[1]), z(afCoordinate[2])
{}

MapGIS::Tile::Vector3D::Vector3D(const int afCoordinate[3])
{
	x = (double)afCoordinate[0];
	y = (double)afCoordinate[1];
	z = (double)afCoordinate[2];
}

MapGIS::Tile::Vector3D::Vector3D(double* fX, double*fY, double*fZ)
	: x(*fX), y(*fY), z(*fZ)
{}

MapGIS::Tile::Vector3D::Vector3D(const double scaler) : x(scaler), y(scaler), z(scaler)
{}

/** Exchange the contents of this vector_s with another.
*/
void MapGIS::Tile::Vector3D::Swap(MapGIS::Tile::Vector3D& other)
{
	std::swap(x, other.x);
	std::swap(y, other.y);
	std::swap(z, other.z);
}

double MapGIS::Tile::Vector3D::operator [] (const size_t i) const
{
	assert(i < 3);
	if (i == 0)
		return x;
	else if (i == 1)
		return y;
	else
		return z;
}

double& MapGIS::Tile::Vector3D::operator [] (const size_t i)
{
	assert(i < 3);
	if (i == 0)
		return x;
	else if (i == 1)
		return y;
	else
		return z;
}
MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator = (const MapGIS::Tile::Vector3D& rkVector)
{
	x = rkVector.x;
	y = rkVector.y;
	z = rkVector.z;

	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator = (const double fScaler)
{
	x = fScaler;
	y = fScaler;
	z = fScaler;

	return *this;
}

bool MapGIS::Tile::Vector3D::operator == (const MapGIS::Tile::Vector3D& rkVector) const
{
	return (x == rkVector.x && y == rkVector.y && z == rkVector.z);
}

bool MapGIS::Tile::Vector3D::operator != (const MapGIS::Tile::Vector3D& rkVector) const
{
	return (x != rkVector.x || y != rkVector.y || z != rkVector.z);
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator + (const MapGIS::Tile::Vector3D& rkVector) const
{
	return MapGIS::Tile::Vector3D(
		x + rkVector.x,
		y + rkVector.y,
		z + rkVector.z);
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator - (const MapGIS::Tile::Vector3D& rkVector) const
{
	return MapGIS::Tile::Vector3D(
		x - rkVector.x,
		y - rkVector.y,
		z - rkVector.z);
}
MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator * (const double fScalar) const
{
	return MapGIS::Tile::Vector3D(
		x * fScalar,
		y * fScalar,
		z * fScalar);
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator * (const MapGIS::Tile::Vector3D& rhs) const
{
	return MapGIS::Tile::Vector3D(
		x * rhs.x,
		y * rhs.y,
		z * rhs.z);
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator / (const double fScalar) const
{
	assert(fScalar != 0.0);

	double fInv = 1.0f / fScalar;

	return MapGIS::Tile::Vector3D(
		x * fInv,
		y * fInv,
		z * fInv);
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator / (const MapGIS::Tile::Vector3D& rhs) const
{
	return MapGIS::Tile::Vector3D(
		x / rhs.x,
		y / rhs.y,
		z / rhs.z);
}

const MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator + () const
{
	return *this;
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::operator - () const
{
	return MapGIS::Tile::Vector3D(-x, -y, -z);
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator += (const MapGIS::Tile::Vector3D& rkVector)
{
	x += rkVector.x;
	y += rkVector.y;
	z += rkVector.z;

	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator += (const double fScalar)
{
	x += fScalar;
	y += fScalar;
	z += fScalar;
	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator -= (const MapGIS::Tile::Vector3D& rkVector)
{
	x -= rkVector.x;
	y -= rkVector.y;
	z -= rkVector.z;

	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator -= (const double fScalar)
{
	x -= fScalar;
	y -= fScalar;
	z -= fScalar;
	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator *= (const double fScalar)
{
	x *= fScalar;
	y *= fScalar;
	z *= fScalar;
	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator *= (const MapGIS::Tile::Vector3D& rkVector)
{
	x *= rkVector.x;
	y *= rkVector.y;
	z *= rkVector.z;

	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator /= (const double fScalar)
{
	assert(fScalar != 0.0);

	double fInv = 1.0f / fScalar;

	x *= fInv;
	y *= fInv;
	z *= fInv;

	return *this;
}

MapGIS::Tile::Vector3D& MapGIS::Tile::Vector3D::operator /= (const MapGIS::Tile::Vector3D& rkVector)
{
	x /= rkVector.x;
	y /= rkVector.y;
	z /= rkVector.z;

	return *this;
}

bool MapGIS::Tile::Vector3D::operator < (const MapGIS::Tile::Vector3D& rhs) const
{
	if (x < rhs.x && y < rhs.y && z < rhs.z)
		return true;
	return false;
}

bool MapGIS::Tile::Vector3D::operator > (const MapGIS::Tile::Vector3D& rhs) const
{
	if (x > rhs.x && y > rhs.y && z > rhs.z)
		return true;
	return false;
}

double MapGIS::Tile::Vector3D::DotProduct(const MapGIS::Tile::Vector3D& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::CrossProduct(const MapGIS::Tile::Vector3D& rkVector) const
{
	return MapGIS::Tile::Vector3D(
		y * rkVector.z - z * rkVector.y,
		z * rkVector.x - x * rkVector.z,
		x * rkVector.y - y * rkVector.x);
}

double MapGIS::Tile::Vector3D::Normalise()
{
	double fLength = sqrt(x * x + y * y + z * z);

	// Will also work for zero-sized vectors, but will change nothing
	// We're not using epsilons because we don't need to.
	// Read http://www.G3D3d.org/forums/viewtopic.php?f=4&t=61259
	if (fLength > double(0.0f))
	{
		double fInvLength = 1.0f / fLength;
		x *= fInvLength;
		y *= fInvLength;
		z *= fInvLength;
	}

	return fLength;
}

MapGIS::Tile::Vector3D MapGIS::Tile::Vector3D::NormalisedCopy(void) const
{
	MapGIS::Tile::Vector3D ret = *this;
	ret.Normalise();
	return ret;
}
double MapGIS::Tile::Vector3D::Length() const
{
	return sqrt(x * x + y * y + z * z);
}

double MapGIS::Tile::Vector3D::Distance(const MapGIS::Tile::Vector3D& rhs) const
{
	return (*this - rhs).Length();
}

MapGIS::Tile::Vector4D::Vector4D() :x(0), y(0), z(0), w(0)
{}

MapGIS::Tile::Vector4D::Vector4D(const double fX, const double fY, const double fZ, const double fW)
	: x(fX), y(fY), z(fZ), w(fW)
{
}
MapGIS::Tile::Vector4D::Vector4D(const double afCoordinate[4])
	: x(afCoordinate[0]),
	y(afCoordinate[1]),
	z(afCoordinate[2]),
	w(afCoordinate[3])
{
}

MapGIS::Tile::Vector4D::Vector4D(const int afCoordinate[4])
{
	x = (double)afCoordinate[0];
	y = (double)afCoordinate[1];
	z = (double)afCoordinate[2];
	w = (double)afCoordinate[3];
}

MapGIS::Tile::Vector4D::Vector4D(double* fX, double *fY, double *fZ, double *fW) : x(*fX), y(*fY), z(*fZ), w(*fW)
{}

MapGIS::Tile::Vector4D::Vector4D(const double scaler) : x(scaler), y(scaler), z(scaler), w(scaler)
{}

MapGIS::Tile::Vector4D::Vector4D(const MapGIS::Tile::Vector3D& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(1.0f)
{
}

void MapGIS::Tile::Vector4D::Swap(MapGIS::Tile::Vector4D& other)
{
	std::swap(x, other.x);
	std::swap(y, other.y);
	std::swap(z, other.z);
	std::swap(w, other.w);
}

double MapGIS::Tile::Vector4D::operator [] (const size_t i) const
{
	assert(i < 4);
	switch (i)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		break;
	}

	return x;
}

double& MapGIS::Tile::Vector4D::operator [] (const size_t i)
{
	assert(i < 4);
	switch (i)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		break;
	}
	return x;
}

/** Assigns the value of the other vector_s.
@param
rkVector The other vector_s
*/
MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator = (const MapGIS::Tile::Vector4D& rkVector)
{
	x = rkVector.x;
	y = rkVector.y;
	z = rkVector.z;
	w = rkVector.w;

	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator = (const double fScalar)
{
	x = fScalar;
	y = fScalar;
	z = fScalar;
	w = fScalar;
	return *this;
}

bool MapGIS::Tile::Vector4D::operator == (const MapGIS::Tile::Vector4D& rkVector) const
{
	return (x == rkVector.x &&
		y == rkVector.y &&
		z == rkVector.z &&
		w == rkVector.w);
}

bool MapGIS::Tile::Vector4D::operator != (const MapGIS::Tile::Vector4D& rkVector) const
{
	return (x != rkVector.x ||
		y != rkVector.y ||
		z != rkVector.z ||
		w != rkVector.w);
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator = (const MapGIS::Tile::Vector3D& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = 1.0f;
	return *this;
}

// arithmetic operations
MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator + (const MapGIS::Tile::Vector4D& rkVector) const
{
	return MapGIS::Tile::Vector4D(
		x + rkVector.x,
		y + rkVector.y,
		z + rkVector.z,
		w + rkVector.w);
}

MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator - (const MapGIS::Tile::Vector4D& rkVector) const
{
	return MapGIS::Tile::Vector4D(
		x - rkVector.x,
		y - rkVector.y,
		z - rkVector.z,
		w - rkVector.w);
}

MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator * (const double fScalar) const
{
	return MapGIS::Tile::Vector4D(
		x * fScalar,
		y * fScalar,
		z * fScalar,
		w * fScalar);
}

MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator * (const MapGIS::Tile::Vector4D& rhs) const
{
	return MapGIS::Tile::Vector4D(
		rhs.x * x,
		rhs.y * y,
		rhs.z * z,
		rhs.w * w);
}

MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator / (const double fScalar) const
{
	assert(fScalar != 0.0);

	double fInv = 1.0f / fScalar;

	return MapGIS::Tile::Vector4D(
		x * fInv,
		y * fInv,
		z * fInv,
		w * fInv);
}

MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator / (const MapGIS::Tile::Vector4D& rhs) const
{
	return MapGIS::Tile::Vector4D(
		x / rhs.x,
		y / rhs.y,
		z / rhs.z,
		w / rhs.w);
}

const MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator + () const
{
	return *this;
}

MapGIS::Tile::Vector4D MapGIS::Tile::Vector4D::operator - () const
{
	return MapGIS::Tile::Vector4D(-x, -y, -z, -w);
}
// arithmetic updates
MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator += (const MapGIS::Tile::Vector4D& rkVector)
{
	x += rkVector.x;
	y += rkVector.y;
	z += rkVector.z;
	w += rkVector.w;

	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator -= (const MapGIS::Tile::Vector4D& rkVector)
{
	x -= rkVector.x;
	y -= rkVector.y;
	z -= rkVector.z;
	w -= rkVector.w;

	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator *= (const double fScalar)
{
	x *= fScalar;
	y *= fScalar;
	z *= fScalar;
	w *= fScalar;
	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator += (const double fScalar)
{
	x += fScalar;
	y += fScalar;
	z += fScalar;
	w += fScalar;
	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator -= (const double fScalar)
{
	x -= fScalar;
	y -= fScalar;
	z -= fScalar;
	w -= fScalar;
	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator *= (const MapGIS::Tile::Vector4D& rkVector)
{
	x *= rkVector.x;
	y *= rkVector.y;
	z *= rkVector.z;
	w *= rkVector.w;

	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator /= (const double fScalar)
{
	assert(fScalar != 0.0);

	double fInv = 1.0f / fScalar;

	x *= fInv;
	y *= fInv;
	z *= fInv;
	w *= fInv;

	return *this;
}

MapGIS::Tile::Vector4D& MapGIS::Tile::Vector4D::operator /= (const MapGIS::Tile::Vector4D& rkVector)
{
	x /= rkVector.x;
	y /= rkVector.y;
	z /= rkVector.z;
	w /= rkVector.w;

	return *this;
}

double MapGIS::Tile::Vector4D::DotProduct(const MapGIS::Tile::Vector4D& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
}

MapGIS::Tile::Matrix4D::Matrix4D() :m{ { 1,0,0,0 },{ 0,1,0,0 },{ 0,0,1,0 },{ 0,0,0,1 } }
{
}
MapGIS::Tile::Matrix4D::Matrix4D(
	double m00, double m01, double m02, double m03,
	double m10, double m11, double m12, double m13,
	double m20, double m21, double m22, double m23,
	double m30, double m31, double m32, double m33)
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;
	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;
	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;
	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

bool MapGIS::Tile::Matrix4D::IsUnit() const
{
	double dTo = 0.0000000000001;
	if (fabs(m[0][0] - 1) > dTo || fabs(m[0][1]) > dTo || fabs(m[0][2]) > dTo || fabs(m[0][3]) > dTo)
		return false;
	if (fabs(m[1][0]) > dTo || fabs(m[1][1] - 1) > dTo || fabs(m[1][2]) > dTo || fabs(m[1][3]) > dTo)
		return false;
	if (fabs(m[2][0]) > dTo || fabs(m[2][1]) > dTo || fabs(m[2][2] - 1) > dTo || fabs(m[2][3]) > dTo)
		return false;
	if (fabs(m[3][0]) > dTo || fabs(m[3][1]) > dTo || fabs(m[3][2]) > dTo || fabs(m[3][3] - 1) > dTo)
		return false;
	return true;
}

void MapGIS::Tile::Matrix4D::SetUnit()
{
	m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
	m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
	m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;

}

/** Exchange the contents of this matrix with another.
*/
void MapGIS::Tile::Matrix4D::Swap(MapGIS::Tile::Matrix4D& other)
{
	std::swap(m[0][0], other.m[0][0]);
	std::swap(m[0][1], other.m[0][1]);
	std::swap(m[0][2], other.m[0][2]);
	std::swap(m[0][3], other.m[0][3]);
	std::swap(m[1][0], other.m[1][0]);
	std::swap(m[1][1], other.m[1][1]);
	std::swap(m[1][2], other.m[1][2]);
	std::swap(m[1][3], other.m[1][3]);
	std::swap(m[2][0], other.m[2][0]);
	std::swap(m[2][1], other.m[2][1]);
	std::swap(m[2][2], other.m[2][2]);
	std::swap(m[2][3], other.m[2][3]);
	std::swap(m[3][0], other.m[3][0]);
	std::swap(m[3][1], other.m[3][1]);
	std::swap(m[3][2], other.m[3][2]);
	std::swap(m[3][3], other.m[3][3]);
}

double* MapGIS::Tile::Matrix4D::operator [] (size_t iRow)
{
	assert(iRow < 4);
	return m[iRow];
}

const double *MapGIS::Tile::Matrix4D::operator [] (size_t iRow) const
{
	assert(iRow < 4);
	return m[iRow];
}

/** Matrix concatenation using '*'.
*/
MapGIS::Tile::Matrix4D MapGIS::Tile::Matrix4D::operator * (const MapGIS::Tile::Matrix4D &m2) const
{
	MapGIS::Tile::Matrix4D r;
	r.m[0][0] = m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0] + m[0][3] * m2.m[3][0];
	r.m[0][1] = m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1] + m[0][3] * m2.m[3][1];
	r.m[0][2] = m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2] + m[0][3] * m2.m[3][2];
	r.m[0][3] = m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3] * m2.m[3][3];

	r.m[1][0] = m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0] + m[1][3] * m2.m[3][0];
	r.m[1][1] = m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1] + m[1][3] * m2.m[3][1];
	r.m[1][2] = m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2] + m[1][3] * m2.m[3][2];
	r.m[1][3] = m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3] * m2.m[3][3];

	r.m[2][0] = m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0] + m[2][3] * m2.m[3][0];
	r.m[2][1] = m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1] + m[2][3] * m2.m[3][1];
	r.m[2][2] = m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2] + m[2][3] * m2.m[3][2];
	r.m[2][3] = m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3] * m2.m[3][3];

	r.m[3][0] = m[3][0] * m2.m[0][0] + m[3][1] * m2.m[1][0] + m[3][2] * m2.m[2][0] + m[3][3] * m2.m[3][0];
	r.m[3][1] = m[3][0] * m2.m[0][1] + m[3][1] * m2.m[1][1] + m[3][2] * m2.m[2][1] + m[3][3] * m2.m[3][1];
	r.m[3][2] = m[3][0] * m2.m[0][2] + m[3][1] * m2.m[1][2] + m[3][2] * m2.m[2][2] + m[3][3] * m2.m[3][2];
	r.m[3][3] = m[3][0] * m2.m[0][3] + m[3][1] * m2.m[1][3] + m[3][2] * m2.m[2][3] + m[3][3] * m2.m[3][3];

	return r;
}

/** Vector transformation using '*'.
@remarks
Transforms the given 3-D vector_s by the matrix, projecting the
result back into <i>w</i> = 1.
@note
This means that the initial <i>w</i> is considered to be 1.0,
and then all the tree elements of the resulting 3-D vector_s are
divided by the resulting <i>w</i>.
*/
MapGIS::Tile::Vector3D MapGIS::Tile::Matrix4D::operator * (const MapGIS::Tile::Vector3D &v) const
{
	MapGIS::Tile::Vector3D r;

	double fInvW = 1.0f / (m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3]);

	r.x = (m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3]) * fInvW;
	r.y = (m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3]) * fInvW;
	r.z = (m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]) * fInvW;

	return r;
}

D_3DOT MapGIS::Tile::Matrix4D::operator * (const D_3DOT &v) const
{
	D_3DOT r;

	double fInvW = 1.0f / (m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3]);

	r.x = (m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3]) * fInvW;
	r.y = (m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3]) * fInvW;
	r.z = (m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]) * fInvW;

	return r;
}

/*	MapGIS::Tile::Vector3_d MapGIS::Tile::Matrix4_d::operator * (const MapGIS::Tile::Vector3_d &v) const
{
MapGIS::Tile::Vector3_d r;

double fInvW = 1.0f / (m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3]);

r.x = (m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3]) * fInvW;
r.y = (m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3]) * fInvW;
r.z = (m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]) * fInvW;

return r;
}*/
MapGIS::Tile::Vector4D MapGIS::Tile::Matrix4D::operator * (const MapGIS::Tile::Vector4D& v) const
{
	return MapGIS::Tile::Vector4D(
		m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
		m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
		m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
		m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
	);
}

/** Matrix addition.
*/
MapGIS::Tile::Matrix4D MapGIS::Tile::Matrix4D::operator + (const MapGIS::Tile::Matrix4D &m2) const
{
	MapGIS::Tile::Matrix4D r;

	r.m[0][0] = m[0][0] + m2.m[0][0];
	r.m[0][1] = m[0][1] + m2.m[0][1];
	r.m[0][2] = m[0][2] + m2.m[0][2];
	r.m[0][3] = m[0][3] + m2.m[0][3];

	r.m[1][0] = m[1][0] + m2.m[1][0];
	r.m[1][1] = m[1][1] + m2.m[1][1];
	r.m[1][2] = m[1][2] + m2.m[1][2];
	r.m[1][3] = m[1][3] + m2.m[1][3];

	r.m[2][0] = m[2][0] + m2.m[2][0];
	r.m[2][1] = m[2][1] + m2.m[2][1];
	r.m[2][2] = m[2][2] + m2.m[2][2];
	r.m[2][3] = m[2][3] + m2.m[2][3];

	r.m[3][0] = m[3][0] + m2.m[3][0];
	r.m[3][1] = m[3][1] + m2.m[3][1];
	r.m[3][2] = m[3][2] + m2.m[3][2];
	r.m[3][3] = m[3][3] + m2.m[3][3];

	return r;
}

/** Matrix subtraction.
*/
MapGIS::Tile::Matrix4D MapGIS::Tile::Matrix4D::operator - (const MapGIS::Tile::Matrix4D &m2) const
{
	MapGIS::Tile::Matrix4D r;
	r.m[0][0] = m[0][0] - m2.m[0][0];
	r.m[0][1] = m[0][1] - m2.m[0][1];
	r.m[0][2] = m[0][2] - m2.m[0][2];
	r.m[0][3] = m[0][3] - m2.m[0][3];

	r.m[1][0] = m[1][0] - m2.m[1][0];
	r.m[1][1] = m[1][1] - m2.m[1][1];
	r.m[1][2] = m[1][2] - m2.m[1][2];
	r.m[1][3] = m[1][3] - m2.m[1][3];

	r.m[2][0] = m[2][0] - m2.m[2][0];
	r.m[2][1] = m[2][1] - m2.m[2][1];
	r.m[2][2] = m[2][2] - m2.m[2][2];
	r.m[2][3] = m[2][3] - m2.m[2][3];

	r.m[3][0] = m[3][0] - m2.m[3][0];
	r.m[3][1] = m[3][1] - m2.m[3][1];
	r.m[3][2] = m[3][2] - m2.m[3][2];
	r.m[3][3] = m[3][3] - m2.m[3][3];

	return r;
}
MapGIS::Tile::Matrix4D& MapGIS::Tile::Matrix4D::operator =(const MapGIS::Tile::Matrix4D& matrix)//赋值运算符
{
	if (this != &matrix)
	{
		this->m[0][0] = matrix.m[0][0];
		this->m[0][1] = matrix.m[0][1];
		this->m[0][2] = matrix.m[0][2];
		this->m[0][3] = matrix.m[0][3];

		this->m[1][0] = matrix.m[1][0];
		this->m[1][1] = matrix.m[1][1];
		this->m[1][2] = matrix.m[1][2];
		this->m[1][3] = matrix.m[1][3];

		this->m[2][0] = matrix.m[2][0];
		this->m[2][1] = matrix.m[2][1];
		this->m[2][2] = matrix.m[2][2];
		this->m[2][3] = matrix.m[2][3];

		this->m[3][0] = matrix.m[3][0];
		this->m[3][1] = matrix.m[3][1];
		this->m[3][2] = matrix.m[3][2];
		this->m[3][3] = matrix.m[3][3];
	}
	return *this;
}

/** Tests 2 matrices for equality.
*/
bool MapGIS::Tile::Matrix4D::operator == (const MapGIS::Tile::Matrix4D& m2) const
{
	if (
		m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
		m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
		m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
		m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3])
		return false;
	return true;
}

/** Tests 2 matrices for inequality.
*/
bool MapGIS::Tile::Matrix4D::operator != (const MapGIS::Tile::Matrix4D& m2) const
{
	if (
		m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
		m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
		m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
		m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3])
		return true;
	return false;
}

MapGIS::Tile::Matrix4D MapGIS::Tile::Matrix4D::Transpose(void) const
{
	return MapGIS::Tile::Matrix4D(m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
}

MapGIS::Tile::Matrix4D MapGIS::Tile::Matrix4D::Inverse(void) const
{
	double m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	double m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	double m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	double m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	double v0 = m20 * m31 - m21 * m30;
	double v1 = m20 * m32 - m22 * m30;
	double v2 = m20 * m33 - m23 * m30;
	double v3 = m21 * m32 - m22 * m31;
	double v4 = m21 * m33 - m23 * m31;
	double v5 = m22 * m33 - m23 * m32;

	double t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
	double t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
	double t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
	double t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

	double invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	double d00 = t00 * invDet;
	double d10 = t10 * invDet;
	double d20 = t20 * invDet;
	double d30 = t30 * invDet;

	double d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	double d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	double d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	double d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	double d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	double d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	double d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	double d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	double d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	double d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	double d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	double d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	return MapGIS::Tile::Matrix4D(
		d00, d01, d02, d03,
		d10, d11, d12, d13,
		d20, d21, d22, d23,
		d30, d31, d32, d33);
}
MapGIS::Tile::Matrix4D MapGIS::Tile::Matrix4D::operator*(double scalar) const
{
	return MapGIS::Tile::Matrix4D(
		scalar*m[0][0], scalar*m[0][1], scalar*m[0][2], scalar*m[0][3],
		scalar*m[1][0], scalar*m[1][1], scalar*m[1][2], scalar*m[1][3],
		scalar*m[2][0], scalar*m[2][1], scalar*m[2][2], scalar*m[2][3],
		scalar*m[3][0], scalar*m[3][1], scalar*m[3][2], scalar*m[3][3]);
}

double  MapGIS::Tile::Matrix4D::Determinant() const {
	return m[0][0] * m[1][1] * m[2][2] * m[3][3] - m[0][0] * m[1][1] * m[2][3] * m[3][2] + m[0][0] * m[1][2] * m[2][3] * m[3][1] - m[0][0] * m[1][2] * m[2][1] * m[3][3]
		+ m[0][0] * m[1][3] * m[2][1] * m[3][2] - m[0][0] * m[1][3] * m[2][2] * m[3][1] - m[0][1] * m[1][2] * m[2][3] * m[3][0] + m[0][1] * m[1][2] * m[2][0] * m[3][3]
		- m[0][1] * m[1][3] * m[2][0] * m[3][2] + m[0][1] * m[1][3] * m[2][2] * m[3][0] - m[0][1] * m[1][0] * m[2][2] * m[3][3] + m[0][1] * m[1][0] * m[2][3] * m[3][2]
		+ m[0][2] * m[1][3] * m[2][0] * m[3][1] - m[0][2] * m[1][3] * m[2][1] * m[3][0] + m[0][2] * m[1][0] * m[2][1] * m[3][3] - m[0][2] * m[1][0] * m[2][3] * m[3][1]
		+ m[0][2] * m[1][1] * m[2][3] * m[3][0] - m[0][2] * m[1][1] * m[2][0] * m[3][3] - m[0][3] * m[1][0] * m[2][1] * m[3][2] + m[0][3] * m[1][0] * m[2][2] * m[3][1]
		- m[0][3] * m[1][1] * m[2][2] * m[3][0] + m[0][3] * m[1][1] * m[2][0] * m[3][2] - m[0][3] * m[1][2] * m[2][0] * m[3][1] + m[0][3] * m[1][2] * m[2][1] * m[3][0];
}

double MapGIS::Tile::WGS84Ellipsoid::GetRadFromDegree(double	degree)
{
	return RadPerDegree * degree;
}

//经纬度值 转 椭球上的 法向量
MapGIS::Tile::Vector3D MapGIS::Tile::WGS84Ellipsoid::GetEllipsoidNormalByDegree(double	Lon, double Lat)
{
	double		lonr = Lon * RadPerDegree;
	double		latr = Lat * RadPerDegree;
	double		lenxoy = cos(latr);
	MapGIS::Tile::Vector3D		nor;
	nor.x = lenxoy * cos(lonr);
	nor.y = lenxoy * sin(lonr);
	nor.z = sin(latr);
	nor = nor.NormalisedCopy();
	//修改初始化方式，兼容vs2005版本
	MapGIS::Tile::Vector3D dot = { nor.x , nor.y , nor.z };
	return dot;
}

//地理坐标值 转 椭球笛卡尔坐标
MapGIS::Tile::Vector3D  MapGIS::Tile::WGS84Ellipsoid::CartesianFromGeographicaDegree(double	Lon, double Lat, double height)
{
	MapGIS::Tile::Vector3D mRadiiSquared;
	mRadiiSquared.x = WGS84_EllipsoidA * WGS84_EllipsoidA;
	mRadiiSquared.y = WGS84_EllipsoidB * WGS84_EllipsoidB;
	mRadiiSquared.z = WGS84_EllipsoidC * WGS84_EllipsoidC;

	double		lonr = Lon * RadPerDegree;
	double		latr = Lat * RadPerDegree;
	double		lenxoy = cos(latr);
	MapGIS::Tile::Vector3D		nor;
	nor.x = lenxoy * cos(lonr);
	nor.y = lenxoy * sin(lonr);
	nor.z = sin(latr);
	nor = nor.NormalisedCopy();
	MapGIS::Tile::Vector3D		scratchK;
	scratchK = mRadiiSquared * nor;
	double	dotRes = nor.DotProduct(scratchK);
	double	gamma = sqrt(dotRes);
	scratchK = scratchK / gamma;
	nor = nor * height;
	//MapGIS::Tile::Vector3_d			Cartesian3Dot = scratchK + nor;
	MapGIS::Tile::Vector3D			res;
	res.x = scratchK.x + nor.x;
	res.y = scratchK.y + nor.y;
	res.z = scratchK.z + nor.z;
	return res;
}

//地理坐标值 转 椭球笛卡尔坐标
MapGIS::Tile::Vector3D  MapGIS::Tile::WGS84Ellipsoid::CartesianFromGeographicaDegree2(double Lon, double Lat, double height)
{
	double		lonr = Lon * RadPerDegree;
	double		latr = Lat * RadPerDegree;
	double		lenxoy = cos(latr);
	MapGIS::Tile::Vector3D		nor;
	nor.x = lenxoy * cos(lonr);
	nor.y = lenxoy * sin(lonr);
	nor.z = sin(latr);
	nor = nor.NormalisedCopy();

	double			e = sqrt(WGS84_EllipsoidA  * WGS84_EllipsoidA - WGS84_EllipsoidC * WGS84_EllipsoidC) / WGS84_EllipsoidA;
	double			N = WGS84_EllipsoidA / (sqrt(1 - e * e * sin(latr) * sin(latr)));
	double			xx_yy = N * cos(latr);   //  p 点投影到 xoy 平面的  曲率半径
											 /*
											 x * x / (mA * mA) + y * y / (mB * mB) + z * z / (mC * mC) = 1
											 z = sqrt(  mC^2 - mC^2 * ( x * x / (mA * mA) + y * y / (mB * mB)  )  );
											 x   y  与经度的关系
											 x * x + y * y = xx_yy * xx_yy
											 y / x = tan(lon)
											 */
	double		z = sqrt(WGS84_EllipsoidC * WGS84_EllipsoidC - WGS84_EllipsoidC * WGS84_EllipsoidC * xx_yy * xx_yy / (WGS84_EllipsoidA * WGS84_EllipsoidA));
	double		tanLon = tan(lonr);
	//y = x * tanLon;
	double	x = sqrt(xx_yy * xx_yy / (1 + tanLon * tanLon));
	if (Lon > 90 || Lon < -90)
	{
		x = -1 * x;
	}
	double	y = x * tanLon;
	MapGIS::Tile::Vector3D		res;
	res.x = x + height * nor.x;
	res.y = y + height * nor.y;
	res.z = z + height * nor.z;
	return  res;
}

//椭球笛卡尔坐标 转角度值地理坐标
MapGIS::Tile::Vector3D   MapGIS::Tile::WGS84Ellipsoid::GeographicaDegreeFromCartesian(MapGIS::Tile::Vector3D dot)
{
	double		lonr = atan(dot.y / dot.x);
	double		lon = lonr / RadPerDegree;
	if (dot.y > 0)
	{
		if (lon < 0)
		{
			lon += 180;
		}
	}
	if (dot.y < 0)
	{
		if (lon > 0)
		{
			lon -= 180;
		}
	}
	double			e = sqrt(WGS84_EllipsoidA  * WGS84_EllipsoidA - WGS84_EllipsoidC * WGS84_EllipsoidC) / WGS84_EllipsoidA;
	//  迭代进行求值
	//tan(B) = dot.z + N * e * e * sin(B) / (dot.x * dot.x + dot.y * dot.y);
	double			B = 0;
	double			lat;
	double			lastB = atan(dot.z / sqrt(dot.x * dot.x + dot.y * dot.y));
	lat = lastB / RadPerDegree;
	double			N = WGS84_EllipsoidA / (sqrt(1 - e * e * sin(lastB) * sin(lastB)));
	double			tem = (dot.z + N * e * e * sin(lastB)) / sqrt(dot.x * dot.x + dot.y * dot.y);
	B = atan(tem);
	lat = B / RadPerDegree;
	double		detal = fabs(B - lastB);
	lastB = B;
	int	count = 0;
	while (detal > 1e-10 && count < 100)
	{
		N = WGS84_EllipsoidA / (sqrt(1 - e * e * sin(lastB) * sin(lastB)));
		tem = (dot.z + N * e * e * sin(lastB)) / sqrt(dot.x * dot.x + dot.y * dot.y);
		B = atan(tem);
		detal = fabs(B - lastB);
		lastB = B;
		count++;
	}
	lat = B / RadPerDegree;
	if (dot.z < 0 && lat > 0)
	{
		lat = -1 * lat;
	}
	double			H;
	N = WGS84_EllipsoidA / (sqrt(1 - e * e * sin(B) * sin(B)));
	H = dot.z / sin(B) - N * (1 - e * e);
	MapGIS::Tile::Vector3D		res;
	res.x = lon;
	res.y = lat;
	res.z = H;
	return res;
}

//椭球笛卡尔坐标 转弧度值地理坐标
MapGIS::Tile::Vector3D   MapGIS::Tile::WGS84Ellipsoid::GeographicaRadianFromCartesian(MapGIS::Tile::Vector3D dot)
{
	MapGIS::Tile::Vector3D res3d_d = GeographicaDegreeFromCartesian(dot);

	res3d_d.x = res3d_d.x / 180.0 * PI;
	res3d_d.y = res3d_d.y / 180.0 * PI;
	res3d_d.z = res3d_d.z;

	return res3d_d;
}

D_3DOT  MapGIS::Tile::WGS84Ellipsoid::GeographicaRadianFromCartesian(D_3DOT dot)
{
	MapGIS::Tile::Vector3D res3d_d = GeographicaDegreeFromCartesian(MapGIS::Tile::Vector3D(dot.x, dot.y, dot.z));
	D_3DOT rtn;
	rtn.x = res3d_d.x / 180.0 * PI;
	rtn.y = res3d_d.y / 180.0 * PI;
	rtn.z = res3d_d.z;

	return rtn;
}

MapGIS::Tile::Matrix4D  MapGIS::Tile::WGS84Ellipsoid::GetLocationPointTransform(double longitudeRadians, double latitudeRadians, double heightMeters)
{
	MapGIS::Tile::Vector3D radii = MapGIS::Tile::Vector3D(6378137.0, 6378137.0, 6356752.3142451793);
	MapGIS::Tile::Vector3D radiiSquared = MapGIS::Tile::Vector3D(radii.x * radii.x, radii.y * radii.y, radii.z * radii.z);
	MapGIS::Tile::Vector3D desPoint(0.0, 0.0, 0.0);

	double longitude = longitudeRadians;
	double latitude = latitudeRadians;

	double cosLongitude = cos(longitude);
	double sinLongitude = sin(longitude);
	double cosLatitude = cos(latitude);
	double sinLatitude = sin(latitude);

	MapGIS::Tile::Vector3D tempN;
	tempN.x = cosLatitude * cosLongitude;
	tempN.y = cosLatitude * sinLongitude;
	tempN.z = sinLatitude;
	tempN.Normalise();
	MapGIS::Tile::Vector3D			scratchK = tempN * radiiSquared;
	double gamma = sqrt(tempN.DotProduct(scratchK));
	scratchK = scratchK / gamma;
	tempN = tempN * heightMeters;
	desPoint = scratchK + tempN;
	MapGIS::Tile::Vector3D		oneOverRadiiSquared = MapGIS::Tile::Vector3D(1.0 / (radii.x * radii.x), 1.0 / (radii.y * radii.y), 1.0 / (radii.z * radii.z));
	MapGIS::Tile::Vector3D		nor = (desPoint * oneOverRadiiSquared).NormalisedCopy();
	MapGIS::Tile::Vector3D		up;
	up = nor;
	MapGIS::Tile::Vector3D   eastTemp, east;
	eastTemp.x = -desPoint.y;
	eastTemp.y = desPoint.x;
	eastTemp.z = 0.0;
	east = eastTemp.NormalisedCopy();
	MapGIS::Tile::Vector3D north;
	north = up.CrossProduct(east);
	MapGIS::Tile::Vector3D down, west, south;
	down = up * -1.0;
	west = east * -1.0;
	south = north * -1.0;
	MapGIS::Tile::Matrix4D transformMat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	transformMat4[0][0] = east.x;        transformMat4[0][1] = north.x;        transformMat4[0][2] = up.x; transformMat4[0][3] = desPoint.x;
	transformMat4[1][0] = east.y;        transformMat4[1][1] = north.y;        transformMat4[1][2] = up.y; transformMat4[1][3] = desPoint.y;
	transformMat4[2][0] = east.z;        transformMat4[2][1] = north.z;        transformMat4[2][2] = up.z; transformMat4[2][3] = desPoint.z;
	transformMat4[3][0] = 0.0;			 transformMat4[3][1] = 0.0;            transformMat4[3][2] = 0.0;  transformMat4[3][3] = 1.0;
	return transformMat4;
}