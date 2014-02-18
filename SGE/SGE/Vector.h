#ifndef CVECTOR_H
#define CVECTOR_H

#include <iostream>

#include "Angle.h"
class CAngle;

class CVector
{
public:
	// Members
	double X; // Must be first so you can pass to something like struct Vec{ int x, int y, int z};
	double Y;
	double Z;

	// Funcs
	CVector(double X, double Y, double Z);
	CVector(double Pitch, double Yaw);
	CVector();
	~CVector();
	double Length() const;
	double LengthSqr() const; // These operations are good for comparison, they offer a 'round compare distance' without the slowdowns of sqrt()
	double Length2D() const;
	double Length2DSqr() const;
	CVector ToScreen(const CVector& CamPos, const CAngle& CamAngle, double FOV, unsigned int ScreenWidth, unsigned int ScreenHeight) const;

	CAngle ToAngle() const;
	CVector Right() const;

	CVector Normal() const;
	double Distance(const CVector& A) const;
	CVector Cross(const CVector& A) const;
	double Dot(const CVector& A) const;

	CVector RotateAroundAxis(const CVector& Axis, double Angle) const;

	CVector Perp(const CVector& Axis) const;
	CVector Parallel(const CVector& Axis) const;

	// These modify the vector!
	CVector& Normalize();
	CVector& Approach(const CVector& What, const double& Ammount);

	// Math funcs, changes itself
	CVector& Add(const CVector& A);
	CVector& Sub(const CVector& A);
	CVector& Mul(const CVector& A);
	CVector& Mul(double Scalar);
	CVector& Div(const CVector& A);
	CVector& Div(double Scalar);
};


// Operators

// Stream helpers
std::istream& operator>>(std::istream&, CVector&);
std::ostream& operator<<(std::ostream&, CVector&);

// Math operators, but not with own type (Vec * 5.0)
CVector operator*(const CVector&, double); // Scale
CVector operator/(const CVector&, double);

// Math operators
CVector operator+(const CVector&, const CVector&);
CVector operator-(const CVector&, const CVector&);
CVector operator*(const CVector&, const CVector&);
CVector operator/(const CVector&, const CVector&);

// These return a reference to the first argument
CVector& operator+=(CVector&, const CVector&);
CVector& operator-=(CVector&, const CVector&);
CVector& operator*=(CVector&, const CVector&);
CVector& operator/=(CVector&, const CVector&);

// Unary operators
CVector operator+(const CVector&);
CVector operator-(const CVector&);

// Comparison operators
bool operator==(const CVector&, const CVector&);
bool operator!=(const CVector&, const CVector&);
bool operator>(const CVector&, const CVector&);
bool operator>=(const CVector&, const CVector&);
bool operator<(const CVector&, const CVector&);
bool operator<=(const CVector&, const CVector&);

#endif