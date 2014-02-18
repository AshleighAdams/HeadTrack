#include <cmath>
#include "Vector.h"

using namespace std;

CVector::CVector()
{
	X = 0;
	Y = 0;
	Z = 0;
}

CVector::CVector(double x, double y, double z)
{
	X = x;
	Y = y;
	Z = z;
}

CVector::CVector(double Pitch, double Yaw)
{
	Pitch /= 180.0 / 3.14159263538979323;
	Yaw /= 180.0 / 3.14159263538979323;

	double cos_pitch = cos(Pitch);
	X = cos(Yaw) * cos_pitch;
	Y = sin(Yaw) * cos_pitch;
	Z = sin(Pitch);
}

CVector::~CVector()
{
}

// Functions

double CVector::Length()  const
{
	return sqrt(X * X + Y * Y + Z * Z);
}

double CVector::LengthSqr()  const
{
	return X * X + Y * Y + Z * Z;
}

double CVector::Length2D()  const
{
	return sqrt(X * X + Y * Y);
}

double CVector::Length2DSqr()  const
{
	return X * X + Y * Y;
}

double rads(double ang)
{
	return ang / 180.0 * 3.141592653589793238462;
}

/* Position, visible */
CVector CVector::ToScreen(const CVector& CamPos, const CAngle& CamAngle, double FOV, unsigned int ScreenWidth, unsigned int ScreenHeight) const
{
	CVector up = CamAngle.Up();
	CVector right = CamAngle.Right();
	CVector forward = CamAngle.Forward();

	CVector dir = CamPos - *this;
	double fdp = forward.Dot(dir);

	if(fdp == 0)
		return CVector(0, 0);

	double d = 4.0 * ScreenHeight / (6.0 * tan(rads(0.5 * FOV)));
	CVector proj = dir * (d / fdp);

	double x = 0.5 * ScreenWidth + right.Dot(proj);
	double y = 0.5 * ScreenHeight - up.Dot(proj);

	return CVector(x, y, 0.0);
}

CAngle CVector::ToAngle() const
{
	double yaw, pitch;

	if(this->Y == 0 && this->X == 0)
	{
		yaw = 0;
		pitch = this->Z > 0 ? 270 : 90;
	}
	else
	{
		yaw = (atan2(this->Y, this->X) * 180.0 / 3.14159263538979323);
		if(yaw < 0)
			yaw += 360;

		pitch = (atan2(-this->Z, this->Length2D()) * 180.0 / 3.14159265358979323);
		if(pitch < 0)
			pitch += 360;
	}
	
	return CAngle(pitch, yaw, 0);
}

CVector CVector::Right()  const
{
	CVector ret(*this);
	ret.Z = 0;
	return ret.Cross(CVector(0.0, 0.0, 1.0));
}

CVector CVector::Normal()  const
{
	double Len = Length();
	if(Len == 0)
		Len = 1.0;
	return CVector(X / Len, Y / Len, Z / Len);
}

double CVector::Distance(const CVector& A)  const
{
	CVector ret(X, Y, Z);
	return (ret += A).Length();
}

CVector CVector::Cross(const CVector& A)  const
{
	return CVector
	(
		Y * A.Z - Z * A.Y,
		Z * A.X - X * A.Z,
		X * A.Y - Y * A.X
	);
}

double CVector::Dot(const CVector& A)  const
{
	return X * A.X + Y * A.Y + Z * A.Z;
}


CVector CVector::RotateAroundAxis(const CVector& Axis, double Angle) const
{
	CVector ret;

	Angle = Angle / 180.0 * 3.14159263538979323;

	double cos_theta = cos(Angle);
	double sin_theta = sin(Angle);

	return this->Perp(Axis) * cos_theta + Axis.Cross(*this) * sin_theta + this->Parallel(Axis);

	return ret;
	
}

CVector CVector::Perp(const CVector& Axis) const
{
	return *this - this->Parallel(Axis);
}

CVector CVector::Parallel(const CVector& Axis) const
{
	return Axis * this->Dot(Axis);
}


// These modify the actual vector!

CVector& CVector::Normalize()
{
	double Len = Length();
	if(Len == 0.0)
		Len = 1.0;
	X /= Len;
	Y /= Len;
	Z /= Len;
	return *this;
}

inline double Clamp(const double& Val, const double& Min, const double& Max)
{
	return min(max(Val, Min), Max);
}

inline void ApproachDbl(double& Cur, const double& Targ, const double& Ammount)
{
	double inc = abs(Ammount);

	if(Cur < Targ)
		Cur = Clamp(Cur + inc, Cur, Targ);
	else
		Cur = Clamp(Cur - inc, Targ, Cur);
}

CVector& CVector::Approach(const CVector& What, const double& Ammount)
{
	CVector norm = ((*this) - What).Normalize();
	double MultX = abs(norm.X);
	double MultY = abs(norm.Y);
	double MultZ = abs(norm.Z);
	ApproachDbl(X, What.X, Ammount * MultX);
	ApproachDbl(Y, What.Y, Ammount * MultY);
	ApproachDbl(Z, What.Z, Ammount * MultZ);

	return *this;
}

// Operators


// Math functions, will be used by += but you can still use the method, flexibility.

CVector& CVector::Add(const CVector& A)
{
	X += A.X;
	Y += A.Y;
	Z += A.Z;
	return *this;
}
CVector& CVector::Sub(const CVector& A)
{
	X -= A.X;
	Y -= A.Y;
	Z -= A.Z;
	return *this;
}
CVector& CVector::Mul(const CVector& A)
{
	X *= A.X;
	Y *= A.Y;
	Z *= A.Z;
	return *this;
}

CVector& CVector::Mul(double Scalar)
{
	X *= Scalar;
	Y *= Scalar;
	Z *= Scalar;
	return *this;
}
CVector& CVector::Div(const CVector& A)
{
	X /= A.X;
	Y /= A.Y;
	Z /= A.Z;
	return *this;
}
CVector& CVector::Div(double Scalar)
{
	X /= Scalar;
	Y /= Scalar;
	Z /= Scalar;
	return *this;
}

// Operators

std::istream& operator>>(std::istream& s, CVector& Vec)
{
	s >> Vec.X;
	s >> Vec.Y;
	s >> Vec.Z;
	return s;
}

std::ostream& operator<<(std::ostream& s, CVector& Vec)
{
	s << Vec.X << ", " << Vec.Y << ", " << Vec.Z;
	return s;
}

// Math operators using other than own type

CVector operator*(const CVector& A, double Scalar)
{
	return CVector(A.X * Scalar, A.Y * Scalar, A.Z * Scalar);
}
CVector operator/(const CVector& A, double Scalar)
{
	return CVector(A.X / Scalar, A.Y / Scalar, A.Z / Scalar);
}

CVector& operator*=(CVector& A, double Scalar)
{
	return A.Mul(Scalar);
}
CVector& operator/=(CVector& A, double Scalar)
{
	return A.Div(Scalar);
}

// Math operators

CVector operator+(const CVector& A, const CVector& B)
{
	return CVector(A.X + B.X, A.Y + B.Y, A.Z + B.Z);
}
CVector operator-(const CVector& A, const CVector& B)
{
	return CVector(A.X - B.X, A.Y - B.Y, A.Z - B.Z);
}
CVector operator*(const CVector& A, const CVector& B)
{
	return CVector(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
}
CVector operator/(const CVector& A, const CVector& B)
{
	return CVector(A.X / B.X, A.Y / B.Y, A.Z / B.Z);
}

// X= operators

CVector& operator+=(CVector& A, const CVector& B)
{
	return A.Add(B);
}
CVector& operator-=(CVector& A, const CVector& B)
{
	return A.Sub(B);
}
CVector& operator*=(CVector& A, const CVector& B)
{
	return A.Mul(B);
}
CVector& operator/=(CVector& A, const CVector& B)
{
	return A.Div(B);
}


// Unary operators
CVector operator+(const CVector& A)
{
	return CVector(+A.X, +A.Y, +A.Z);
}
CVector operator-(const CVector& A)
{
	return CVector(-A.X, -A.Y, -A.Z);
}

// Comparison operators

bool operator==(const CVector& A, const CVector& B)
{
	return A.X == B.X && A.Y == B.Y && A.Z == B.Z;
}
bool operator!=(const CVector& A, const CVector& B)
{
	return !operator==(A, B);
}
bool operator>(const CVector& A, const CVector& B)
{
	return A.LengthSqr() > B.LengthSqr(); // To use no sqrt function (only squaring) is much, much quicker!
}
bool operator>=(const CVector& A, const CVector& B)
{
	return operator==(A, B) || operator>(A,B);
}
bool operator<(const CVector& A, const CVector& B)
{
	return A.LengthSqr() < B.LengthSqr();
}
bool operator<=(const CVector& A, const CVector& B)
{
	return operator==(A, B) || operator<(A,B);
}

